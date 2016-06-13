/*
 * linux/regulator/nxe1500-regulator.h
 *
 *  Copyright (C) 2016 Nexell
 *  Jongsin Park <pjsin865@nexell.co.kr>
 *
 * Based on code
 *	Copyright (C) 2011 NVIDIA Corporation
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef __LINUX_REGULATOR_NXE1500_H
#define __LINUX_REGULATOR_NXE1500_H

#include <linux/regulator/machine.h>
#include <linux/regulator/driver.h>

#define nxe1500_rails(_name) "NXE1500_"#_name

/* RICHOH Regulator IDs */
enum regulator_id {
	NXE1500_ID_DC1,
	NXE1500_ID_DC2,
	NXE1500_ID_DC3,
	NXE1500_ID_DC4,
	NXE1500_ID_DC5,
	NXE1500_ID_LDO1,
	NXE1500_ID_LDO2,
	NXE1500_ID_LDO3,
	NXE1500_ID_LDO4,
	NXE1500_ID_LDO5,
	NXE1500_ID_LDO6,
	NXE1500_ID_LDO7,
	NXE1500_ID_LDO8,
	NXE1500_ID_LDO9,
	NXE1500_ID_LDO10,
	NXE1500_ID_LDORTC1,
	NXE1500_ID_LDORTC2,
};

struct nxe1500_regulator_platform_data {
		struct regulator_init_data regulator;
		int init_uV;
		unsigned init_enable:1;
		unsigned init_apply:1;
		int sleep_uV;
		int sleep_slots;
		unsigned long ext_pwr_req;
		unsigned long flags;
};

extern	int nxe1500_regulator_enable_eco_mode(struct regulator_dev *rdev);
extern	int nxe1500_regulator_disable_eco_mode(struct regulator_dev *rdev);
extern	int nxe1500_regulator_enable_eco_slp_mode(struct regulator_dev *rdev);
extern	int nxe1500_regulator_disable_eco_slp_mode(struct regulator_dev *rdev);


#endif
