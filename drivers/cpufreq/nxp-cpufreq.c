/*
 * (C) Copyright 2009
 * jung hyun kim, Nexell Co, <jhkim@nexell.co.kr>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/cpufreq.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/clk.h>
#include <linux/kthread.h>
#include <linux/sysrq.h>
#include <linux/suspend.h>
#include <linux/notifier.h>
#include <linux/platform_device.h>
#include <linux/regulator/consumer.h>
#include <mach/platform.h>
#include <mach/devices.h>

/*
#define pr_debug	printk
*/

#include "nxp-cpufreq.h"

/*
 * DVFS info
 */
struct freq_dvfs_time {
	unsigned long start;
	unsigned long duration;
};

struct cpufreq_dvfs_info {
	struct cpufreq_frequency_table *freq_table;
	unsigned long (*freq_volts)[2];	/* khz freq (khz): voltage(uV): voltage (us) */
	struct clk *clk;
	cpumask_var_t cpus;
	int cpu;
	struct mutex lock;
	/* limited max frequency */
	int  run_monitor;
	long target_freq;
    long max_cpufreq;		/* khz */
    long max_retention;		/* msec */
    long rest_cpufreq;		/* khz */
    long rest_retention;	/* msec */
    long rest_period;
    long new_cpufreq;		/* khz */
   	ktime_t rest_ktime;
   	struct hrtimer rest_hrtimer;
   	struct hrtimer restore_hrtimer;
   	struct task_struct *rest_p;
    /* voltage control */
    struct regulator *volt;
    int table_size;
    long supply_delay_us;
    /* for suspend/resume */
    struct notifier_block pm_notifier;
    unsigned long resume_state;
    long reset_freq;
    int reset_voltage;
    /* check frequency duration */
	int  freq_cur_index;
	int  freq_pre_index;
    unsigned long freq_check_state;
    struct freq_dvfs_time *freq_times;
};

#define	FREQ_TABLE_MAX			(30)
#define	FREQ_STATE_RESUME 		(0)	/* bit num */
#define	FREQ_STATE_TIME_RUN   	(0)	/* bit num */

static unsigned long st_asv_freq_tables[FREQ_TABLE_MAX][2];
static struct freq_dvfs_time st_freq_times[FREQ_TABLE_MAX] = { {0,}, };

static struct cpufreq_dvfs_info		*cpufreq_dvfs = NULL;
#define	set_cpufreq_dvfs_info(d)	(cpufreq_dvfs = d)
#define	get_cpufreq_dvfs_info()		(cpufreq_dvfs)
#define	ms_to_ktime(m)				ns_to_ktime((u64)m * 1000 * 1000)


static int nxp_cpufreq_frequency_index(unsigned long frequency)
{
	struct cpufreq_dvfs_info *dvfs = get_cpufreq_dvfs_info();
	unsigned long (*freq_tables)[2] = (unsigned long(*)[2])dvfs->freq_volts;
	int len = dvfs->table_size;
	int index = 0;

	for (index = 0; len > index; index++) {
		if (frequency == freq_tables[index][0])
			break;
	}

	if (index == len) {
		index = CPUFREQ_ENTRY_INVALID;
		printk("Fail : invalid frequency (%ld) index !!!\n", frequency);
	}
	return index;
}

static enum hrtimer_restart nxp_cpufreq_restore_timer(struct hrtimer *hrtimer)
{
	struct cpufreq_dvfs_info *dvfs = get_cpufreq_dvfs_info();

	dvfs->rest_ktime = ktime_set(0, 0);	/* clear */
	dvfs->new_cpufreq = dvfs->target_freq;	/* restore */

	pr_debug("cpufreq : restore %ldkhz after rest %4ldms\n",
		dvfs->target_freq, dvfs->rest_retention);

	if (dvfs->target_freq > dvfs->rest_cpufreq) {
		wake_up_process(dvfs->rest_p);

		/* to rest frequency after end of rest time */
		hrtimer_start(&dvfs->rest_hrtimer, ms_to_ktime(dvfs->max_retention),
				HRTIMER_MODE_REL_PINNED);
	}
	return HRTIMER_NORESTART;
}

static enum hrtimer_restart nxp_cpufreq_rest_timer(struct hrtimer *hrtimer)
{
	struct cpufreq_dvfs_info *dvfs = get_cpufreq_dvfs_info();

	dvfs->rest_ktime = ktime_get();
	dvfs->new_cpufreq = dvfs->rest_cpufreq;

	pr_debug("cpufreq : %ldkhz (%4ldms) -> %ldkhz rest (%4ldms) \n",
		dvfs->max_cpufreq, dvfs->max_retention,
		dvfs->new_cpufreq, dvfs->rest_retention);

	wake_up_process(dvfs->rest_p);

	/* to restore frequency after end of rest time */
	hrtimer_start(&dvfs->restore_hrtimer, ms_to_ktime(dvfs->rest_retention),
			HRTIMER_MODE_REL_PINNED);

	return HRTIMER_NORESTART;
}

unsigned int nxp_cpufreq_voltage(unsigned long freqhz)
{
	struct cpufreq_dvfs_info *dvfs = get_cpufreq_dvfs_info();
 	unsigned long (*freq_volts)[2] = (unsigned long(*)[2])dvfs->freq_volts;
	long mS = 0, uS = 0, uV = 0, wT = 0;
	int index = dvfs->freq_cur_index;

	if (!dvfs->volt)
		return 0;

	uV = freq_volts[index][1];
	wT = dvfs->supply_delay_us;

	regulator_set_voltage(dvfs->volt, uV, uV);

	if (wT) {
		mS = wT/1000;
		uS = wT%1000;
		if (mS) mdelay(mS);
		if (uS) udelay(uS);
	}

#ifdef CONFIG_ARM_NXP_CPUFREQ_VOLTAGE_DEBUG
	printk(" volt (%lukhz %ld.%06ld V, %ld.%03ld us)\n",
			freq_volts[index][0], uV/1000000, uV%1000000, mS, uS);
#endif
	return uV;
}

static unsigned long nxp_cpufreq_update(struct cpufreq_dvfs_info *dvfs,
				struct cpufreq_freqs *freqs)
{
	struct clk *clk = dvfs->clk;
	unsigned long rate = 0;
	int index = dvfs->freq_cur_index;

	if (!test_bit(FREQ_STATE_RESUME, &dvfs->resume_state))
		return freqs->old;

	/* pre voltage */
	if (freqs->new > freqs->old)
		nxp_cpufreq_voltage(freqs->new*1000);

	for_each_cpu(freqs->cpu, dvfs->cpus)
		cpufreq_notify_transition(freqs, CPUFREQ_PRECHANGE);

	rate = clk_set_rate(clk, freqs->new*1000);

	if (test_bit(FREQ_STATE_TIME_RUN, &dvfs->freq_check_state)) {
		int prev = dvfs->freq_pre_index;
		long ms = ktime_to_ms(ktime_get());

		dvfs->freq_times[prev].duration += (ms - dvfs->freq_times[prev].start);
		dvfs->freq_times[index].start = ms;
		dvfs->freq_pre_index = index;
	}

	for_each_cpu(freqs->cpu, dvfs->cpus)
		cpufreq_notify_transition(freqs, CPUFREQ_POSTCHANGE);

	/* post voltage */
	if (freqs->old > freqs->new)
		nxp_cpufreq_voltage(freqs->new*1000);

	return rate;
}

static int nxp_cpufreq_pm_notify(struct notifier_block *this,
        unsigned long mode, void *unused)
{
	struct cpufreq_dvfs_info *dvfs = container_of(this,
					struct cpufreq_dvfs_info, pm_notifier);
	struct clk *clk = dvfs->clk;
	struct cpufreq_freqs freqs;

    switch(mode) {
    case PM_SUSPEND_PREPARE:	/* set initial frequecny */
		mutex_lock(&dvfs->lock);

		freqs.new = dvfs->reset_freq;
		freqs.old = clk_get_rate(clk) / 1000;

		nxp_cpufreq_update(dvfs, &freqs);

    	clear_bit(FREQ_STATE_RESUME, &dvfs->resume_state);
		mutex_unlock(&dvfs->lock);
    	break;

    case PM_POST_SUSPEND:	/* set restore frequecny */
		mutex_lock(&dvfs->lock);
    	set_bit(FREQ_STATE_RESUME, &dvfs->resume_state);

		freqs.new = dvfs->target_freq;
		freqs.old = clk_get_rate(clk) / 1000;

		nxp_cpufreq_update(dvfs, &freqs);

		mutex_unlock(&dvfs->lock);
    	break;
    }
    return 0;
}

static int nxp_cpufreq_proc_update(void *unused)
{
	struct cpufreq_dvfs_info *dvfs = get_cpufreq_dvfs_info();
	struct cpufreq_freqs freqs;
	struct clk *clk = dvfs->clk;

	set_current_state(TASK_INTERRUPTIBLE);

	while (!kthread_should_stop()) {

		if (dvfs->new_cpufreq) {
			mutex_lock(&dvfs->lock);
			set_current_state(TASK_UNINTERRUPTIBLE);

			freqs.new = dvfs->new_cpufreq;
			freqs.old = clk_get_rate(clk)/1000;;
			freqs.cpu = dvfs->cpu;
			dvfs->freq_cur_index = nxp_cpufreq_frequency_index(freqs.new);

			nxp_cpufreq_update(dvfs, &freqs);

			set_current_state(TASK_INTERRUPTIBLE);
			mutex_unlock(&dvfs->lock);
		}

		/* wait */
		schedule();

		if (kthread_should_stop())
			break;

		set_current_state(TASK_INTERRUPTIBLE);
	}

	set_current_state(TASK_RUNNING);
	return 0;
}

static inline int nxp_cpufreq_setup(struct cpufreq_dvfs_info *dvfs)
{
	struct hrtimer *hrtimer = &dvfs->rest_hrtimer;
	int cpu = 0;
	struct task_struct *p;

	dvfs->run_monitor = 0;
	mutex_init(&dvfs->lock);

	hrtimer_init(hrtimer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	hrtimer->function = nxp_cpufreq_rest_timer;

	hrtimer = &dvfs->restore_hrtimer;
	hrtimer_init(hrtimer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	hrtimer->function = nxp_cpufreq_restore_timer;

	p = kthread_create_on_node(nxp_cpufreq_proc_update,
				NULL, cpu_to_node(cpu), "cpufreq-update");
	if (IS_ERR(p)) {
		pr_err("%s: cpu%d: failed rest thread for cpufreq\n", __func__, cpu);
		return PTR_ERR(p);
	}
	kthread_bind(p, cpu);
	wake_up_process(p);

	dvfs->rest_p = p;

	return 0;
}

static ssize_t show_speed_duration(struct cpufreq_policy *policy, char *buf)
{
	struct cpufreq_dvfs_info *dvfs = get_cpufreq_dvfs_info();
	int index = dvfs->freq_cur_index;
	ssize_t count = 0;
	int i = 0;

	if (test_bit(FREQ_STATE_TIME_RUN, &dvfs->freq_check_state)) {
		long ms = ktime_to_ms(ktime_get());
		if (dvfs->freq_times[index].start)
			dvfs->freq_times[index].duration += (ms - dvfs->freq_times[index].start);
		dvfs->freq_times[index].start = ms;
		dvfs->freq_pre_index = index;
	}

	for (; dvfs->table_size > i; i++)
		count += sprintf(&buf[count], "%8ld ", dvfs->freq_times[i].duration);

	count += sprintf(&buf[count], "\n");
	return count;
}

static ssize_t store_speed_duration(struct cpufreq_policy *policy,
			const char *buf, size_t count)
{
	struct cpufreq_dvfs_info *dvfs = get_cpufreq_dvfs_info();
	int index = dvfs->freq_cur_index;
	long ms = ktime_to_ms(ktime_get());
	const char *s = buf;

	mutex_lock(&dvfs->lock);

	if (0 == strncmp(s, "run", strlen("run"))) {
		dvfs->freq_pre_index = index;
		dvfs->freq_times[index].start = ms;
		set_bit(FREQ_STATE_TIME_RUN, &dvfs->freq_check_state);
	}
	else if (0 == strncmp(s, "stop", strlen("stop"))) {
		clear_bit(FREQ_STATE_TIME_RUN, &dvfs->freq_check_state);
	}
	else if (0 == strncmp(s, "clear", strlen("clear"))) {
		memset(dvfs->freq_times, 0, sizeof(st_freq_times));
		if (test_bit(FREQ_STATE_TIME_RUN, &dvfs->freq_check_state)) {
			dvfs->freq_times[index].start = ms;
			dvfs->freq_pre_index = index;
		}
	} else {
		count = -1;
	}

	mutex_unlock(&dvfs->lock);

	return count;
}

static ssize_t show_voltage_level(struct cpufreq_policy *policy, char *buf)
{
	struct cpufreq_dvfs_info *dvfs = get_cpufreq_dvfs_info();
 	unsigned long (*freq_volts)[2] = (unsigned long(*)[2])dvfs->freq_volts;
	char *s = buf;
	int i = 0;

	for (; dvfs->table_size > i; i++)
		s += sprintf(s, "%8ld", freq_volts[i][1]);

	if (s != buf)
		*(s-1) = '\n';

	return (s - buf);
}

static ssize_t store_voltage_level(struct cpufreq_policy *policy,
			const char *buf, size_t count)
{
	bool percent = false, down = false;
	const char *s = strchr(buf, '-');
	long value;

	if (s)
		down = true;
	else
		s = strchr(buf, '+');

	if (!s)
		s = buf;
	else
		s++;

	if (strchr(buf, '%'))
		percent = 1;

	value = simple_strtol(s, NULL, 10);
	nxp_cpufreq_asv_change_vol(st_asv_freq_tables, value, down, percent);

	return count;
}

static struct freq_attr cpufreq_freq_attr_scaling_speed_duration = {
    .attr = {
    	.name = "scaling_speed_duration",
		.mode = 0666,
	},
    .show  = show_speed_duration,
    .store = store_speed_duration,
};

static struct freq_attr cpufreq_freq_attr_scaling_voltage_level = {
    .attr = {
    	.name = "scaling_voltage_level",
		.mode = 0666,
	},
    .show  = show_voltage_level,
    .store = store_voltage_level,
};

static struct freq_attr *nxp_cpufreq_attr[] = {
	&cpufreq_freq_attr_scaling_available_freqs,
	&cpufreq_freq_attr_scaling_voltage_level,
	&cpufreq_freq_attr_scaling_speed_duration,
	NULL,
};

static int nxp_cpufreq_verify_speed(struct cpufreq_policy *policy)
{
	struct cpufreq_dvfs_info *dvfs = get_cpufreq_dvfs_info();
	struct cpufreq_frequency_table *freq_table = dvfs->freq_table;

	if (!freq_table)
		return -EINVAL;

	return cpufreq_frequency_table_verify(policy, freq_table);
}

static unsigned int nxp_cpufreq_getspeed(unsigned int cpu)
{
	struct cpufreq_dvfs_info *dvfs = get_cpufreq_dvfs_info();
	struct clk *clk = dvfs->clk;
	long rate_khz = clk_get_rate(clk)/1000;

	return rate_khz;
}

static int nxp_cpufreq_target(struct cpufreq_policy *policy,
				unsigned int target_freq,
				unsigned int relation)
{
	struct cpufreq_dvfs_info *dvfs = get_cpufreq_dvfs_info();
	struct cpufreq_frequency_table *freq_table = dvfs->freq_table;
	struct cpufreq_frequency_table *table;
	struct cpufreq_freqs freqs;
	unsigned long rate = 0;
	long ts;
	int ret = 0, i = 0;

	ret = cpufreq_frequency_table_target(policy, freq_table,
						target_freq, relation, &i);
	if (ret) {
		pr_err("%s: cpu%d: no freq match for %d khz(ret=%d)\n",
			__func__, policy->cpu, target_freq, ret);
		return ret;
	}

	mutex_lock(&dvfs->lock);

	table = &freq_table[i];
	freqs.new = table->frequency;

	if (!freqs.new) {
		pr_err("%s: cpu%d: no match for freq %d khz\n",
			__func__, policy->cpu, target_freq);
		mutex_unlock(&dvfs->lock);
		return -EINVAL;
	}

	freqs.old = nxp_cpufreq_getspeed(policy->cpu);
	freqs.cpu = policy->cpu;
	pr_debug("cpufreq : target %u -> %u khz mon(%s) ",
		freqs.old, freqs.new, dvfs->run_monitor?"run":"no");

	if (freqs.old == freqs.new && policy->cur == freqs.new) {
		pr_debug("PASS\n");
		mutex_unlock(&dvfs->lock);
		return ret;
	}

	dvfs->cpu = policy->cpu;
	dvfs->target_freq = freqs.new;
	dvfs->freq_cur_index = table->index;

	/* rest period */
	if (ktime_to_ms(dvfs->rest_ktime) && freqs.new > dvfs->rest_cpufreq) {
		ts = (ktime_to_ms(ktime_get()) - ktime_to_ms(dvfs->rest_ktime));
		if (dvfs->rest_retention > ts) {
			freqs.new = dvfs->rest_cpufreq;
			pr_debug("rest %4ld:%4ldms (%u khz)\n", dvfs->rest_retention, ts, freqs.new);
			goto _cpu_freq;	/* retry */
		}
		dvfs->rest_ktime = ktime_set(0, 0);	/* clear rest time */
	}

	if (dvfs->max_cpufreq && dvfs->run_monitor && freqs.new < dvfs->max_cpufreq ) {
		dvfs->run_monitor = 0;
		hrtimer_cancel(&dvfs->rest_hrtimer);
		pr_debug("stop monitor");
	}

	if (dvfs->max_cpufreq && !dvfs->run_monitor && freqs.new >= dvfs->max_cpufreq) {
		dvfs->run_monitor = 1;
		hrtimer_start(&dvfs->rest_hrtimer, ms_to_ktime(dvfs->max_retention),
			      HRTIMER_MODE_REL_PINNED);
		pr_debug("run  monitor");
	}

_cpu_freq:

	pr_debug(" set rate %ukhz\n", freqs.new);
	rate = nxp_cpufreq_update(dvfs, &freqs);

	mutex_unlock(&dvfs->lock);

	return rate;
}

static int __cpuinit nxp_cpufreq_init(struct cpufreq_policy *policy)
{
	struct cpufreq_dvfs_info *dvfs = get_cpufreq_dvfs_info();
	struct cpufreq_frequency_table *freq_table = dvfs->freq_table;
	int res;

	pr_debug("nxp-cpufreq: available frequencies cpus (%d) \n",
		num_online_cpus());

	/* get policy fields based on the table */
	res = cpufreq_frequency_table_cpuinfo(policy, freq_table);
	if (!res) {
		cpufreq_frequency_table_get_attr(freq_table, policy->cpu);
	} else {
		pr_err("nxp-cpufreq: Failed to read policy table\n");
		return res;
	}

	policy->cur = nxp_cpufreq_getspeed(policy->cpu);
	policy->governor = CPUFREQ_DEFAULT_GOVERNOR;

	/*
	 * FIXME : Need to take time measurement across the target()
	 *	   function with no/some/all drivers in the notification
	 *	   list.
	 */
	policy->cpuinfo.transition_latency = 100000; /* in ns */

	/*
	 * multi-core processors has 2 cores
	 * that the frequency cannot be set independently.
	 * Each cpu is bound to the same speed.
	 * So the affected cpu is all of the cpus.
	 */
	if (num_online_cpus() == 1) {
		cpumask_copy(policy->related_cpus, cpu_possible_mask);
		cpumask_copy(policy->cpus, cpu_online_mask);
		cpumask_copy(dvfs->cpus, cpu_online_mask);
	} else {
		cpumask_setall(policy->cpus);
		cpumask_setall(dvfs->cpus);
	}

	return 0;
}

static struct cpufreq_driver nxp_cpufreq_driver = {
	.flags   = CPUFREQ_STICKY,
	.verify  = nxp_cpufreq_verify_speed,
	.target  = nxp_cpufreq_target,
	.get     = nxp_cpufreq_getspeed,
	.init    = nxp_cpufreq_init,
	.name    = "nxp-cpufreq",
	.attr    = nxp_cpufreq_attr,
};

static int nxp_cpufreq_probe(struct platform_device *pdev)
{
	struct nxp_cpufreq_plat_data *plat = pdev->dev.platform_data;
	static struct notifier_block *pm_notifier;
	struct cpufreq_dvfs_info *dvfs;
	struct cpufreq_frequency_table *table;
	char name[16];
	int asv_size = 0;
	int size = 0, i = 0;

	/*
	 * check asv support
	 */
	asv_size = nxp_cpufreq_asv_table(st_asv_freq_tables);
	if (0 >= asv_size &&
		(!plat || !plat->freq_table || !plat->table_size)) {
		dev_err(&pdev->dev, "%s: failed no freq table !!!\n", __func__);
		return -EINVAL;
	}

	dvfs = kzalloc(sizeof(*dvfs), GFP_KERNEL);
	if (!dvfs) {
		dev_err(&pdev->dev, "%s: failed allocate DVFS data !!!\n", __func__);
		return -ENOMEM;
	}

	sprintf(name, "pll%d", plat->pll_dev);
	dvfs->clk = clk_get(NULL, name);
	if (IS_ERR(dvfs->clk))
		return PTR_ERR(dvfs->clk);

	size  = (asv_size > 0 ? asv_size : plat->table_size);
	table = kzalloc((sizeof(*table)*size) + 1, GFP_KERNEL);
	if (!table) {
		dev_err(&pdev->dev, "%s: failed allocate freq table !!!\n", __func__);
		return -ENOMEM;
	}

	set_cpufreq_dvfs_info(dvfs);

	dvfs->freq_table = table;
	dvfs->freq_volts = (unsigned long(*)[2])(asv_size > 0 ?
						st_asv_freq_tables : plat->freq_table);
	dvfs->max_cpufreq = plat->max_cpufreq;
	dvfs->max_retention = plat->max_retention;
	dvfs->rest_cpufreq = plat->rest_cpufreq;
	dvfs->rest_retention = plat->rest_retention;
	dvfs->rest_ktime = ktime_set(0, 0);
	dvfs->run_monitor = 0;
	dvfs->table_size = size;
	dvfs->supply_delay_us = plat->supply_delay_us;
	dvfs->reset_freq = nxp_cpufreq_getspeed(0);
	dvfs->freq_pre_index = -1;
	dvfs->freq_check_state = 0;
	dvfs->freq_times = st_freq_times;

	/*
     * make frequency table with platform data
	 */
	for (; size > i; i++) {
		table->index = i;
		table->frequency = dvfs->freq_volts[i][0];
		table++;
		pr_debug("[%s] %2d = %8ldkhz, %8ld uV (%lu us)\n",
			name, i, dvfs->freq_volts[i][0], dvfs->freq_volts[i][1],
			dvfs->supply_delay_us);
	}

	table->index = i;
	table->frequency = CPUFREQ_TABLE_END;

	/*
     * get voltage regulator table with platform data
	 */
	if (plat->supply_name) {
		dvfs->volt = regulator_get(NULL, plat->supply_name);
		if (IS_ERR(dvfs->volt)) {
			dev_err(&pdev->dev, "%s: Cannot get regulator for DVS supply %s\n",
				__func__, plat->supply_name);
			kfree(table);
			kfree(dvfs);
			return -1;
		}
		dvfs->reset_voltage = regulator_get_voltage(dvfs->volt);

		pm_notifier = &dvfs->pm_notifier;
		pm_notifier->notifier_call = nxp_cpufreq_pm_notify;
		if (register_pm_notifier(pm_notifier)) {
			dev_err(&pdev->dev, "%s: Cannot pm notifier %s\n",
				__func__, plat->supply_name);
			return -1;
		}
		set_bit(FREQ_STATE_RESUME, &dvfs->resume_state);
	}

	if (0 > nxp_cpufreq_setup(dvfs))
		return -EINVAL;

	printk("DVFS: cpu %s with PLL.%d [tables=%d]\n",
		dvfs->volt?"DVFS":"DFS", plat->pll_dev, dvfs->table_size);

	return cpufreq_register_driver(&nxp_cpufreq_driver);
}

static struct platform_driver cpufreq_driver = {
	.driver	= {
	.name	= DEV_NAME_CPUFREQ,
	.owner	= THIS_MODULE,
	},
	.probe	= nxp_cpufreq_probe,
};
module_platform_driver(cpufreq_driver);
