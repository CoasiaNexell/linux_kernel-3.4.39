/*
 * (C) Copyright 2009
 * jung hyun kim, Nexell Co, <jhkim@nexell.co.kr>
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/platform_device.h>
#include <linux/power_supply.h>
#include <linux/irq.h>
#include <linux/amba/pl022.h>
#include <linux/gpio.h>

/* nexell soc headers */
#include <mach/platform.h>
#include <mach/devices.h>
#include <mach/soc.h>

#if defined(CONFIG_NXP_HDMI_CEC)
#include <mach/nxp-hdmi-cec.h>
#endif

/*------------------------------------------------------------------------------
 * BUS Configure
 */
#if (CFG_BUS_RECONFIG_ENB == 1)
#include <mach/s5p4418_bus.h>

const u8 g_DrexBRB_RD[2] = {
	0x4,    /* Port0 */
	0xF	/* Port1 */
};

const u8 g_DrexBRB_WR[2] = {
	0x4,    /* Port0 */
	0xF     /* Port1 */
};

const u16 g_DrexQoS[2] = {
	0x100,  /* S0 */
	0xFFF	/* S1, Default value */
};


#if (CFG_BUS_RECONFIG_TOPBUSSI == 1)
const u8 g_TopBusSI[8] = {
	TOPBUS_SI_SLOT_DMAC0,
	TOPBUS_SI_SLOT_USBOTG,
	TOPBUS_SI_SLOT_USBHOST0,
	TOPBUS_SI_SLOT_DMAC1,
	TOPBUS_SI_SLOT_SDMMC,
	TOPBUS_SI_SLOT_USBOTG,
	TOPBUS_SI_SLOT_USBHOST1,
	TOPBUS_SI_SLOT_USBOTG
};
#endif

#if (CFG_BUS_RECONFIG_TOPBUSQOS == 1)
const u8 g_TopQoSSI[2] = {
	1,	/* Tidemark */
	(1<<TOPBUS_SI_SLOT_DMAC0) |	/* Control */
	(1<<TOPBUS_SI_SLOT_MP2TS) |
	(1<<TOPBUS_SI_SLOT_DMAC1) |
	(1<<TOPBUS_SI_SLOT_SDMMC) |
	(1<<TOPBUS_SI_SLOT_USBOTG) |
	(1<<TOPBUS_SI_SLOT_USBHOST0) |
	(1<<TOPBUS_SI_SLOT_USBHOST1)
};
#endif

#if (CFG_BUS_RECONFIG_BOTTOMBUSSI == 1)
const u8 g_BottomBusSI[8] = {
        BOTBUS_SI_SLOT_1ST_ARM,
        BOTBUS_SI_SLOT_MALI,
        BOTBUS_SI_SLOT_TOP,
        BOTBUS_SI_SLOT_1ST_CODA,
        BOTBUS_SI_SLOT_2ND_ARM,
        BOTBUS_SI_SLOT_TOP,
        BOTBUS_SI_SLOT_TOP,
        BOTBUS_SI_SLOT_2ND_CODA
};
#endif

#if (CFG_BUS_RECONFIG_BOTTOMBUSQOS == 1)
const u8 g_BottomQoSSI[2] = {
	4,	/* Tidemark */
	(1<<BOTBUS_SI_SLOT_1ST_ARM) |	/* Control */
		(1<<BOTBUS_SI_SLOT_2ND_ARM) |
		(0<<BOTBUS_SI_SLOT_MALI) |
		(1<<BOTBUS_SI_SLOT_TOP) |
		(0<<BOTBUS_SI_SLOT_DEINTERLACE) |
		(1<<BOTBUS_SI_SLOT_1ST_CODA)
};
#endif

#if (CFG_BUS_RECONFIG_DISPBUSSI == 1)
const u8 g_DispBusSI[3] = {
	DISBUS_SI_SLOT_1ST_DISPLAY,
	DISBUS_SI_SLOT_2ND_DISPLAY,
	DISBUS_SI_SLOT_2ND_DISPLAY	/* DISBUS_SI_SLOT_GMAC */
};
#endif
#endif	/* CFG_BUS_RECONFIG_ENB */

/*------------------------------------------------------------------------------
 * CPU Frequence
 */
#if defined(CONFIG_ARM_NXP_CPUFREQ)

static struct nxp_cpufreq_plat_data dfs_plat_data = {
	.pll_dev	= CONFIG_NXP_CPUFREQ_PLLDEV,
	.supply_name	= "vdd_arm_1.3V",
	#if 0
	.max_cpufreq	= 1400*1000,
	.max_retention  =   20*1000,
	.rest_cpufreq   =  400*1000,
	.rest_retention =    1*1000,
	#endif
};

static struct platform_device dfs_plat_device = {
	.name = DEV_NAME_CPUFREQ,
	.dev = {
		.platform_data = &dfs_plat_data,
	}
};
#endif

#if defined (CONFIG_SENSORS_NXP_ADC_TEMP)
static struct nxp_adc_tmp_trigger adc_tmp_event[] = {
	{
		.temp  = 52,
		.freq  = 1200000,
		.period = 1000, /* Ms */
	} , {
		.temp  = 56,
		.freq  = 800000,
		.period = 1000, /* Ms */
	}
};

struct nxp_adc_tmp_platdata adc_tmp_plat_data ={
	.channel	= 2,
	.tmp_offset	= 0,
	.duration	= 1000,
	.step_up	= 1,
	.event		= adc_tmp_event,
	.eventsize = ARRAY_SIZE( adc_tmp_event),
};

static struct platform_device adc_temp_plat_device = {
	.name			= "nxp-adc-tmp",
	.dev			= {
		.platform_data	= &adc_tmp_plat_data,
	}
};
#endif /* SENSORS_NXP_ADC_TEMP */

/*------------------------------------------------------------------------------
 * Network DM9000
 */
#if defined(CONFIG_DM9000) || defined(CONFIG_DM9000_MODULE)
#include <linux/dm9000.h>

static struct resource dm9000_resource[] = {
	[0] = {
		.start	= CFG_ETHER_EXT_PHY_BASEADDR,
		.end	= CFG_ETHER_EXT_PHY_BASEADDR + 1,		// 1 (8/16 BIT)
		.flags	= IORESOURCE_MEM
	},
	[1] = {
		.start	= CFG_ETHER_EXT_PHY_BASEADDR + 4,		// + 4 (8/16 BIT)
		.end	= CFG_ETHER_EXT_PHY_BASEADDR + 5,		// + 5 (8/16 BIT)
		.flags	= IORESOURCE_MEM
	},
	[2] = {
		.start	= CFG_ETHER_EXT_IRQ_NUM,
		.end	= CFG_ETHER_EXT_IRQ_NUM,
		.flags	= IORESOURCE_IRQ | IORESOURCE_IRQ_HIGHLEVEL,
	}
};

static struct dm9000_plat_data eth_plat_data = {
	.flags		= DM9000_PLATF_8BITONLY,	// DM9000_PLATF_16BITONLY
};

static struct platform_device dm9000_plat_device = {
	.name			= "dm9000",
	.id				= 0,
	.num_resources	= ARRAY_SIZE(dm9000_resource),
	.resource		= dm9000_resource,
	.dev			= {
		.platform_data	= &eth_plat_data,
	}
};
#endif	/* CONFIG_DM9000 || CONFIG_DM9000_MODULE */


/*------------------------------------------------------------------------------
 * MPEGTS platform device
 */
#if defined(CONFIG_NXP_MP2TS_IF)
#include <mach/nxp_mp2ts.h>

#define NXP_TS_PAGE_NUM_0       (36)	// Variable
#define NXP_TS_BUF_SIZE_0       (TS_PAGE_SIZE * NXP_TS_PAGE_NUM_0)

#define NXP_TS_PAGE_NUM_1       (36)	// Variable
#define NXP_TS_BUF_SIZE_1       (TS_PAGE_SIZE * NXP_TS_PAGE_NUM_1)

#define NXP_TS_PAGE_NUM_CORE    (36)	// Variable
#define NXP_TS_BUF_SIZE_CORE    (TS_PAGE_SIZE * NXP_TS_PAGE_NUM_CORE)


static struct nxp_mp2ts_dev_info mp2ts_dev_info[2] = {
    {
        .demod_irq_num = CFG_GPIO_DEMOD_0_IRQ_NUM,
        .demod_rst_num = CFG_GPIO_DEMOD_0_RST_NUM,
        .tuner_rst_num = CFG_GPIO_TUNER_0_RST_NUM,
    },
    {
        .demod_irq_num = CFG_GPIO_DEMOD_1_IRQ_NUM,
        .demod_rst_num = CFG_GPIO_DEMOD_1_RST_NUM,
        .tuner_rst_num = CFG_GPIO_TUNER_1_RST_NUM,
    },
};

static struct nxp_mp2ts_plat_data mpegts_plat_data = {
	.dev_info = mp2ts_dev_info,
	.ts_dma_size[0] = -1, /* TS ch 0 - Static alloc size */
	.ts_dma_size[1] = NXP_TS_BUF_SIZE_1, /* TS ch 1 - Static alloc size */
	.ts_dma_size[2] = -1, /* TS core - Static alloc size */
};

static struct platform_device mpegts_plat_device = {
	.name = DEV_NAME_MPEGTSI,
	.id = 0,
	.dev = {
		.platform_data = &mpegts_plat_data,
	},
};
#endif  /* CONFIG_NXP_MP2TS_IF */


/*------------------------------------------------------------------------------
 * DISPLAY (LVDS) / FB
 */
#if defined (CONFIG_FB_NXP)
#if defined (CONFIG_FB0_NXP)
static struct nxp_fb_plat_data fb0_plat_data = {
	.module			= CONFIG_FB0_NXP_DISPOUT,
	.layer			= CFG_DISP_PRI_SCREEN_LAYER,
	.format			= CFG_DISP_PRI_SCREEN_RGB_FORMAT,
	.bgcolor		= CFG_DISP_PRI_BACK_GROUND_COLOR,
	.bitperpixel	= CFG_DISP_PRI_SCREEN_PIXEL_BYTE * 8,
	.x_resol		= CFG_DISP_PRI_RESOL_WIDTH,
	.y_resol		= CFG_DISP_PRI_RESOL_HEIGHT,
	#ifdef CONFIG_ANDROID
	.buffers		= 3,
	.skip_pan_vsync	= 1,
	#else
	.buffers		= 2,
	#endif
	.lcd_with_mm	= CFG_DISP_PRI_LCD_WIDTH_MM,	/* 152.4 */
	.lcd_height_mm	= CFG_DISP_PRI_LCD_HEIGHT_MM,	/* 91.44 */
};

static struct platform_device fb0_device = {
	.name	= DEV_NAME_FB,
	.id		= 0,	/* FB device node num */
	.dev    = {
		.coherent_dma_mask 	= 0xffffffffUL,	/* for DMA allocate */
		.platform_data		= &fb0_plat_data
	},
};
#endif

static struct platform_device *fb_devices[] = {
	#if defined (CONFIG_FB0_NXP)
	&fb0_device,
	#endif
};
#endif /* CONFIG_FB_NXP */

/*------------------------------------------------------------------------------
 * backlight : generic pwm device
 */
#if defined(CONFIG_BACKLIGHT_PWM)
#include <linux/pwm_backlight.h>

static struct platform_pwm_backlight_data bl_plat_data = {
	.pwm_id			= CFG_LCD_PRI_PWM_CH,
	.max_brightness = 255,	/* 255 is 100%, set over 100% */
	.dft_brightness = 128,	/* 50% */
	.lth_brightness = 75,	/* about to 5% */
	.pwm_period_ns	= 1000000000/CFG_LCD_PRI_PWM_FREQ,
};

static struct platform_device bl_plat_device = {
	.name	= "pwm-backlight",
	.id		= -1,
	.dev	= {
		.platform_data	= &bl_plat_data,
	},
};

#endif

#if defined(CONFIG_PPM_NXP)
#include <mach/ppm.h>
static struct nxp_ppm_platform_data ppm_plat_data = {
    .input_polarity = NX_PPM_INPUTPOL_INVERT,//NX_PPM_INPUTPOL_INVERT  or  NX_PPM_INPUTPOL_BYPASS
};

static struct platform_device ppm_device = {
    .name           = DEV_NAME_PPM,
    .dev            = {
    	.platform_data  = &ppm_plat_data,
	}
};
#endif

/*------------------------------------------------------------------------------
 * NAND device
 */
#if defined(CONFIG_MTD_NAND_NXP)
#include <linux/mtd/partitions.h>
#include <asm-generic/sizes.h>

static struct mtd_partition nxp_nand_parts[] = {
#if 0
	{
		.name           = "root",
		.offset         =   0 * SZ_1M,
	},
#else
	{
		.name		= "system",
		.offset		=  64 * SZ_1M,
		.size		= 512 * SZ_1M,
	}, {
		.name		= "cache",
		.offset		= MTDPART_OFS_APPEND,
		.size		= 256 * SZ_1M,
	}, {
		.name		= "userdata",
		.offset		= MTDPART_OFS_APPEND,
		.size		= MTDPART_SIZ_FULL,
	}
#endif
};

static struct nxp_nand_plat_data nand_plat_data = {
	.parts		= nxp_nand_parts,
	.nr_parts	= ARRAY_SIZE(nxp_nand_parts),
	.chip_delay = 10,
};

static struct platform_device nand_plat_device = {
	.name	= DEV_NAME_NAND,
	.id		= -1,
	.dev	= {
		.platform_data	= &nand_plat_data,
	},
};
#elif defined(CONFIG_NXP_FTL)
static struct resource nand_resource =
{
};

static struct platform_device nand_plat_device = {
	.name	= DEV_NAME_NAND,
	.id		= -1,
	.dev	= {
	},
};
#endif	/* CONFIG_NXP_FTL */

#if defined(CONFIG_I2C_NXP)
#define I2CUDELAY(x)	1000000/x

#define PMIC_I2CBUS		3
#define I2C3_SCL		((PAD_GPIO_E + 9) | PAD_FUNC_ALT0)	// AP_GPE9_PMIC_SCL
#define I2C3_SDA		((PAD_GPIO_E + 8) | PAD_FUNC_ALT0)	// AP_GPE8_PMIC_SDA

static struct i2c_gpio_platform_data nxp_i2c_gpio_port3 = {
    .sda_pin    = I2C3_SDA,
    .scl_pin    = I2C3_SCL,
    .udelay     = I2CUDELAY(CFG_I2C3_CLK),
    .timeout    = 10,
};

static struct platform_device i2c_device_ch3 = {
    .name   = "i2c-gpio",
    .id     = PMIC_I2CBUS,
    .dev    = {
        .platform_data  = &nxp_i2c_gpio_port3,
    },
};

static struct platform_device *i2c_devices[] = {
    &i2c_device_ch3,
};
#endif /* CONFIG_I2C_NXP */


#if defined(CONFIG_TOUCHSCREEN_TSC2007)
#include <linux/i2c.h>
#include <linux/i2c/tsc2007.h>

#define TSC2007_I2C_BUS     (2)

static int tsc2007_get_pendown_state(struct device *dev){
	return !gpio_get_value(CFG_IO_TOUCH_PENDOWN_DETECT);
}
#define MAX_12BIT           ((1 << 12) - 1)
#define MIN_12BIT           (0)


static struct tsc2007_platform_data tsc2007_plat_data = {
		#if 1
        .x_plate_ohms   = 100,
		.min_x = MIN_12BIT,
		.min_y = MIN_12BIT,
		.max_x = MAX_12BIT,
		.max_y = MAX_12BIT,
		.fuzzx = 0,
		.fuzzy = 0,
		.fuzzz = 0,
		.get_pendown_state = &tsc2007_get_pendown_state,
		#else
        .x_plate_ohms   = 10,
        .poll_period    = 10,
		#endif

    };

static struct i2c_board_info __initdata tsc2007_i2c_bdi = {
    .type   = "tsc2007",
    .addr   = (0x90>>1),
    .irq    = PB_PIO_IRQ(CFG_IO_TOUCH_PENDOWN_DETECT),
    .platform_data = &tsc2007_plat_data,
};
#endif

/*------------------------------------------------------------------------------
 * Keypad platform device
 */
#if defined(CONFIG_KEYBOARD_NXP_KEY) || defined(CONFIG_KEYBOARD_NXP_KEY_MODULE)

#include <linux/input.h>

static unsigned int  button_gpio[] = CFG_KEYPAD_KEY_BUTTON;
static unsigned int  button_code[] = CFG_KEYPAD_KEY_CODE;

static struct nxp_key_plat_data key_plat_data = {
	.bt_count	= ARRAY_SIZE(button_gpio),
	.bt_io		= button_gpio,
	.bt_code	= button_code,
	.bt_repeat	= CFG_KEYPAD_REPEAT,
};

static struct platform_device key_plat_device = {
	.name	= DEV_NAME_KEYPAD,
	.id		= -1,
	.dev    = {
		.platform_data	= &key_plat_data
	},
};
#endif	/* CONFIG_KEYBOARD_NXP_KEY || CONFIG_KEYBOARD_NXP_KEY_MODULE */





#if defined(CONFIG_FCI_FC8300)
#include <linux/i2c.h>

#define	FC8300_I2C_BUS		(1)

/* CODEC */
static struct i2c_board_info __initdata fc8300_i2c_bdi = {
	.type	= "fc8300_i2c",
	.addr	= (0x58>>1),
};

static struct platform_device fc8300_dai = {
	.name			= "fc8300_i2c",
	.id				= 0,
};
#endif


/*------------------------------------------------------------------------------
 * ASoC Codec platform device
 */

#if defined(CONFIG_SND_CODEC_TLV320AIC32X4) || defined(CONFIG_SND_CODEC_TLV320AIC32X4_MODULE)
#include <linux/i2c.h>

#define	TLV320AIC32X4_I2C_BUS		(1)

/* CODEC */
static struct i2c_board_info __initdata tlv320aic32x4_i2c_bdi = {
	.type	= "tlv320aic32x4",
	.addr	= (0x18),
};

/* DAI */
static struct nxp_snd_dai_plat_data tlv320aic32x4_i2s_dai_data = {
	.i2s_ch	= 0,
	.sample_rate	= 48000,
	.pcm_format	 = SNDRV_PCM_FMTBIT_S16_LE,
#if 0
   	.hp_jack 		= {
		.support    	= 1,
		.detect_io		= PAD_GPIO_E + 8,
		.detect_level	= 1,
	},
#endif
};

static struct platform_device tlv320aic32x4_dai = {
	.name			= "tlv320aic32x4-audio",
	.id				= 0,
	.dev			= {
		.platform_data	= &tlv320aic32x4_i2s_dai_data,
	}
};
#endif

#if defined(CONFIG_SND_SPDIF_TRANSCIEVER) || defined(CONFIG_SND_SPDIF_TRANSCIEVER_MODULE)
static struct platform_device spdif_transciever = {
	.name	= "spdif-dit",
	.id		= -1,
};

static struct nxp_snd_dai_plat_data spdif_trans_dai_data = {
	.sample_rate = 48000,
	.pcm_format	 = SNDRV_PCM_FMTBIT_S16_LE,
};

static struct platform_device spdif_trans_dai = {
	.name	= "spdif-transciever",
	.id		= -1,
	.dev	= {
		.platform_data	= &spdif_trans_dai_data,
	}
};
#endif

#if defined(CONFIG_SND_SPDIF_RECEIVER) || defined(CONFIG_SND_SPDIF_RECEIVER_MODULE)
static struct platform_device spdif_receiver = {
	.name	= "spdif-dit-receiver",
	.id		= -1,
};

static struct nxp_snd_dai_plat_data spdif_recev_dai_data = {
	.sample_rate = 48000,
	.pcm_format	 = SNDRV_PCM_FMTBIT_S16_LE,
};

static struct platform_device spdif_recev_dai = {
	.name	= "spdif-receiver",
	.id		= -1,
	.dev	= {
		.platform_data	= &spdif_recev_dai_data,
	}
};
#endif

/*
 * SMART-VOICE Alsa sound
 */
#if defined(CONFIG_SND_CODEC_SMARTVOICE)

/* snd smart-voice codec device */
static struct nxp_svoice_plat_data svoice_data = {
	.spi_cs_gpio = (PAD_GPIO_C + 16),
	.pdm_isrun_gpio = (PAD_GPIO_E + 14),
	.pdm_lrck_gpio = (PAD_GPIO_B + 26),
	.i2s_lrck_gpio = (PAD_GPIO_B + 4),
	.cpu_dais_num = 2,
};

static struct platform_device snd_svoice_dac = {
	.name = "nxp-smartvoice-codec",
	.id = -1,
	.dev    = {
		.platform_data = &svoice_data
	},
};

/* snd pdm-spi cpu device */
static struct nxp_pdm_spi_plat_data snd_spi_data = {
	.data_bits = 16,
	.clk_freq = (100*1000*1000),
	.clk_invert = 1,
	.clk_phase = 1,
	.dma_filter = pl08x_filter_id,
	.dma_ch	= DMA_PERIPHERAL_NAME_SSP2_RX,
};

static struct platform_device snd_spi_dev = {
	.name	= "nxp-pdm-spi",
	.id	= 2,	/* channel */
	.dev    = {
		.platform_data = &snd_spi_data
	},
};

static struct nxp_snd_svoice_dai_plat_data snd_spi_svoice_card_data = {
	.cpu_dai = SND_SVOICE_SPI,
	.ch = 2, /* SPI channel */
	.sample_rate = 16000,
	.pcm_format = SNDRV_PCM_FMTBIT_S16_LE,
};

/* card : snd pdm-spi <-> snd smart-voice codec */
static struct platform_device snd_spi_svoice_card = {
	.name = "nxp-smartvoice-card",
	.id = 0,
	.dev = {
		.platform_data = &snd_spi_svoice_card_data,
	}
};

/* card : snd i2s <-> snd smart-voice codec */
static struct nxp_snd_svoice_dai_plat_data snd_i2s_svoice_card_data = {
	.cpu_dai = SND_SVOICE_I2S,
	.ch = 2, /* I2S channel */
	.sample_rate = 16000,
	.pcm_format = SNDRV_PCM_FMTBIT_S16_LE,
};

static struct platform_device snd_i2s_svoice_card = {
	.name = "nxp-smartvoice-card",
	.id = 1,
	.dev = {
		.platform_data = &snd_i2s_svoice_card_data,
	}
};
#endif

/*------------------------------------------------------------------------------
 *  * reserve mem
 *   */
#ifdef CONFIG_CMA
#include <linux/cma.h>
extern void nxp_cma_region_reserve(struct cma_region *, const char *);

void __init nxp_reserve_mem(void)
{
    static struct cma_region regions[] = {
        {
            .name = "ion",
#ifdef CONFIG_ION_NXP_CONTIGHEAP_SIZE
            .size = CONFIG_ION_NXP_CONTIGHEAP_SIZE * SZ_1K,
#else
			.size = 0,
#endif
            {
                .alignment = PAGE_SIZE,
            }
        },
        {
            .size = 0
        }
    };

    static const char map[] __initconst =
        "ion-nxp=ion;"
        "nx_vpu=ion;";

#ifdef CONFIG_ION_NXP_CONTIGHEAP_SIZE
	pr_info("%s: reserve CMA: size %d\n",
		__func__, CONFIG_ION_NXP_CONTIGHEAP_SIZE * SZ_1K);
#endif
	nxp_cma_region_reserve(regions, map);
}
#endif


/*------------------------------------------------------------------------------
 * PMIC platform device
 */
#if defined(CONFIG_REGULATOR_NXE2000)

#include <linux/i2c.h>
#include <linux/regulator/machine.h>
#include <linux/mfd/nxe2000.h>
#include <linux/gpio.h>
#include <linux/io.h>
#include <linux/regulator/fixed.h>
#include <linux/regulator/nxe2000-regulator.h>
#include <linux/power/nxe2000_battery.h>
//#include <linux/rtc/rtc-nxe2000.h>
//#include <linux/rtc.h>

#define NXE2000_I2C_BUS		(PMIC_I2CBUS)
#define NXE2000_I2C_ADDR	(0x64 >> 1)

/* NXE2000 IRQs */
#define NXE2000_IRQ_BASE	(IRQ_SYSTEM_END)
#define NXE2000_GPIO_BASE	(ARCH_NR_GPIOS) //PLATFORM_NXE2000_GPIO_BASE
#define NXE2000_GPIO_IRQ	(NXE2000_GPIO_BASE + 8)

//#define CONFIG_NXE2000_RTC


static struct regulator_consumer_supply nxe2000_dc1_supply_0[] = {
	REGULATOR_SUPPLY("vdd_arm_1.3V", NULL),
};
static struct regulator_consumer_supply nxe2000_dc2_supply_0[] = {
	REGULATOR_SUPPLY("vdd_core_1.2V", NULL),
};
static struct regulator_consumer_supply nxe2000_dc3_supply_0[] = {
	REGULATOR_SUPPLY("vdd_sys_3.3V", NULL),
};
static struct regulator_consumer_supply nxe2000_dc4_supply_0[] = {
	REGULATOR_SUPPLY("vdd_ddr_1.6V", NULL),
};
static struct regulator_consumer_supply nxe2000_dc5_supply_0[] = {
	REGULATOR_SUPPLY("vdd_sys_1.6V", NULL),
};

static struct regulator_consumer_supply nxe2000_ldo1_supply_0[] = {
	REGULATOR_SUPPLY("vgps_3.3V", NULL),
};
static struct regulator_consumer_supply nxe2000_ldo2_supply_0[] = {
	REGULATOR_SUPPLY("vcam1_1.8V", NULL),
};
static struct regulator_consumer_supply nxe2000_ldo3_supply_0[] = {
	REGULATOR_SUPPLY("vsys1_1.8V", NULL),
};
static struct regulator_consumer_supply nxe2000_ldo4_supply_0[] = {
	REGULATOR_SUPPLY("vsys_1.9V", NULL),
};
static struct regulator_consumer_supply nxe2000_ldo5_supply_0[] = {
	REGULATOR_SUPPLY("vcam_2.8V", NULL),
};
static struct regulator_consumer_supply nxe2000_ldo6_supply_0[] = {
	REGULATOR_SUPPLY("valive_3.3V", NULL),
};
static struct regulator_consumer_supply nxe2000_ldo7_supply_0[] = {
	REGULATOR_SUPPLY("vvid_2.8V", NULL),
};
static struct regulator_consumer_supply nxe2000_ldo8_supply_0[] = {
	REGULATOR_SUPPLY("vdumy0_3.3V", NULL),
};
static struct regulator_consumer_supply nxe2000_ldo9_supply_0[] = {
	REGULATOR_SUPPLY("vcam_3.3V", NULL),
};
static struct regulator_consumer_supply nxe2000_ldo10_supply_0[] = {
	REGULATOR_SUPPLY("vcore_1.0V", NULL),
};
static struct regulator_consumer_supply nxe2000_ldortc1_supply_0[] = {
	REGULATOR_SUPPLY("valive_1.8V", NULL),
};
static struct regulator_consumer_supply nxe2000_ldortc2_supply_0[] = {
	REGULATOR_SUPPLY("valive_1.0V", NULL),
};


#define NXE2000_PDATA_INIT(_name, _sname, _minuv, _maxuv, _always_on, _boot_on, \
	_init_uv, _init_enable, _slp_slots) \
	static struct nxe2000_regulator_platform_data pdata_##_name##_##_sname = \
	{	\
		.regulator = {	\
			.constraints = {	\
				.min_uV		= _minuv,	\
				.max_uV		= _maxuv,	\
				.valid_modes_mask	= (REGULATOR_MODE_NORMAL |	\
									REGULATOR_MODE_STANDBY),	\
				.valid_ops_mask		= (REGULATOR_CHANGE_MODE |	\
									REGULATOR_CHANGE_STATUS |	\
									REGULATOR_CHANGE_VOLTAGE),	\
				.always_on	= _always_on,	\
				.boot_on	= _boot_on,		\
				.apply_uV	= 1,			\
			},	\
			.num_consumer_supplies =		\
				ARRAY_SIZE(nxe2000_##_name##_supply_##_sname),	\
			.consumer_supplies	= nxe2000_##_name##_supply_##_sname, \
			.supply_regulator	= 0,	\
		},	\
		.init_uV		= _init_uv,		\
		.init_enable	= _init_enable,	\
		.sleep_slots	= _slp_slots,	\
	}
/* min_uV/max_uV : Please set the appropriate value for the devices that the power supplied within a*/
/*                 range from min to max voltage according to NXE2000 specification. */
NXE2000_PDATA_INIT(dc1,      0,	 950000, 2000000, 1, 1, 1250000, 1,  4);	/* 1.2V ARM */
NXE2000_PDATA_INIT(dc2,      0,	1000000, 2000000, 1, 1, 1100000, 1,  4);	/* 1.1V CORE */
NXE2000_PDATA_INIT(dc3,      0,	1000000, 3500000, 1, 1, 3300000, 1,  0);	/* 3.3V SYS */
NXE2000_PDATA_INIT(dc4,      0,	1000000, 2000000, 1, 1, 1500000, 1, -1);	/* 1.5V DDR */
NXE2000_PDATA_INIT(dc5,      0,	1000000, 2000000, 1, 1, 1500000, 1,  4);	/* 1.5V SYS */
#if defined(CONFIG_RFKILL_NXP)
NXE2000_PDATA_INIT(ldo1,     0,	1000000, 3500000, 0, 0, 3300000, 0,  0);	/* 3.3V GPS */
#else
NXE2000_PDATA_INIT(ldo1,     0,	1000000, 3500000, 0, 0, 3300000, 1,  0);	/* 3.3V GPS */
#endif
NXE2000_PDATA_INIT(ldo2,     0,	1000000, 3500000, 0, 0,      -1, 0,  0);	/* Not Use */
NXE2000_PDATA_INIT(ldo3,     0,	1000000, 3500000, 1, 0, 1800000, 1,  2);	/* 1.8V CORE */
NXE2000_PDATA_INIT(ldo4,     0,	1000000, 3500000, 1, 0, 1800000, 1,  2);	/* 1.8V AUDIO */
NXE2000_PDATA_INIT(ldo5,     0,	1000000, 3500000, 1, 0, 3300000, 0,  0);	/* 3.3V WIFI */
NXE2000_PDATA_INIT(ldo6,     0,	1000000, 3500000, 1, 0, 3300000, 1, -1);	/* 3.3V ALIVE */
NXE2000_PDATA_INIT(ldo7,     0,	1000000, 3500000, 0, 0,      -1, 0,  0);	/* Not Use */
NXE2000_PDATA_INIT(ldo8,     0,	1000000, 3500000, 0, 0,      -1, 0,  0);	/* Not Use */
NXE2000_PDATA_INIT(ldo9,     0,	1000000, 3500000, 0, 0,      -1, 0,  0);	/* Not Use */
NXE2000_PDATA_INIT(ldo10,    0,	1000000, 3500000, 1, 1, 1000000, 1,  4);	/* 1.0V VCC1P0 CORE */
NXE2000_PDATA_INIT(ldortc1,  0,	1700000, 3500000, 1, 0, 1800000, 1, -1);	/* 1.8V ALIVE */
NXE2000_PDATA_INIT(ldortc2,  0,	1000000, 3500000, 1, 0, 1000000, 1, -1);	/* 1.0V ALIVE */


/*-------- if nxe2000 RTC exists -----------*/
#ifdef CONFIG_NXE2000_RTC
static struct nxe2000_rtc_platform_data rtc_data = {
	.irq	= NXE2000_IRQ_BASE,
	.time	= {
		.tm_year	= 1970,
		.tm_mon		= 0,
		.tm_mday	= 1,
		.tm_hour	= 0,
		.tm_min		= 0,
		.tm_sec		= 0,
	},
};

#define NXE2000_RTC_REG	\
{	\
	.id		= 0,	\
	.name	= "rtc_nxe2000",	\
	.platform_data	= &rtc_data,	\
}
#endif
/*-------- if Nexell RTC exists -----------*/

#define NXE2000_REG(_id, _name, _sname)	\
{	\
	.id		= NXE2000_ID_##_id,	\
	.name	= "nxe2000-regulator",	\
	.platform_data	= &pdata_##_name##_##_sname,	\
}

//==========================================
//NXE2000 Power_Key device data
//==========================================
static struct nxe2000_pwrkey_platform_data nxe2000_pwrkey_data = {
	.irq 		= NXE2000_IRQ_BASE,
	.delay_ms 	= 20,
};
#define NXE2000_PWRKEY_REG		\
{	\
	.id 	= -1,	\
	.name 	= "nxe2000-pwrkey",	\
	.platform_data 	= &nxe2000_pwrkey_data,	\
}

#define NXE2000_DEV_REG 		\
	NXE2000_REG(DC1, dc1, 0),	\
	NXE2000_REG(DC2, dc2, 0),	\
	NXE2000_REG(DC3, dc3, 0),	\
	NXE2000_REG(DC4, dc4, 0),	\
	NXE2000_REG(DC5, dc5, 0),	\
	NXE2000_REG(LDO1, ldo1, 0),	\
	NXE2000_REG(LDO2, ldo2, 0),	\
	NXE2000_REG(LDO3, ldo3, 0),	\
	NXE2000_REG(LDO4, ldo4, 0),	\
	NXE2000_REG(LDO5, ldo5, 0),	\
	NXE2000_REG(LDO6, ldo6, 0),	\
	NXE2000_REG(LDO7, ldo7, 0),	\
	NXE2000_REG(LDO8, ldo8, 0),	\
	NXE2000_REG(LDO9, ldo9, 0),	\
	NXE2000_REG(LDO10, ldo10, 0),	\
	NXE2000_REG(LDORTC1, ldortc1, 0),	\
	NXE2000_REG(LDORTC2, ldortc2, 0)

static struct nxe2000_subdev_info nxe2000_devs_dcdc[] = {
	NXE2000_DEV_REG,
	NXE2000_PWRKEY_REG,
};


#define NXE2000_GPIO_INIT(_init_apply, _output_mode, _output_val, _led_mode, _led_func) \
	{									\
		.output_mode_en = _output_mode,	\
		.output_val		= _output_val,	\
		.init_apply		= _init_apply,	\
		.led_mode		= _led_mode,	\
		.led_func		= _led_func,	\
	}
struct nxe2000_gpio_init_data nxe2000_gpio_data[] = {
	NXE2000_GPIO_INIT(false, false, 0, 0, 0),
	NXE2000_GPIO_INIT(false, false, 0, 0, 0),
	NXE2000_GPIO_INIT(false, false, 0, 0, 0),
	NXE2000_GPIO_INIT(false, false, 0, 0, 0),
	NXE2000_GPIO_INIT(false, false, 0, 0, 0),
};

static struct nxe2000_platform_data nxe2000_platform = {
	.num_subdevs		= ARRAY_SIZE(nxe2000_devs_dcdc),
	.subdevs			= nxe2000_devs_dcdc,
	.irq_base			= NXE2000_IRQ_BASE,
	.irq_type			= IRQ_TYPE_EDGE_FALLING,
	.gpio_base			= NXE2000_GPIO_BASE,
	.gpio_init_data		= nxe2000_gpio_data,
	.num_gpioinit_data	= ARRAY_SIZE(nxe2000_gpio_data),
	.enable_shutdown_pin	= true,
};

static struct i2c_board_info __initdata nxe2000_i2c_boardinfo[] = {
	{
		I2C_BOARD_INFO("nxe2000", NXE2000_I2C_ADDR),
		.irq			= CFG_GPIO_PMIC_INTR,
		.platform_data	= &nxe2000_platform,
	},
};
#endif  /* CONFIG_REGULATOR_NXE2000 */


/*------------------------------------------------------------------------------
 * v4l2 platform device
 */
#if defined(CONFIG_V4L2_NXP) || defined(CONFIG_V4L2_NXP_MODULE)
#include <linux/i2c.h>
#include <linux/delay.h>
#include <linux/regulator/consumer.h>
#include <mach/nxp-v4l2-platformdata.h>
#include <mach/soc.h>

static int camera_common_set_clock(ulong clk_rate)
{
    PM_DBGOUT("%s: %d\n", __func__, (int)clk_rate);
    if (clk_rate > 0)
        nxp_soc_pwm_set_frequency(1, clk_rate, 50);
    else
        nxp_soc_pwm_set_frequency(1, 0, 0);
    msleep(1);
    return 0;
}

static bool is_camera_port_configured = false;
static void camera_common_vin_setup_io(int module, bool force)
{
	if (!force && is_camera_port_configured)
		return;
	else {
		u_int *pad;
		int i, len;
		u_int io, fn;


		/* VIP0:0 = VCLK, VID0 ~ 7 */
		const u_int port[][2] = {
		/* VCLK, HSYNC, VSYNC */
		{ PAD_GPIO_E +  4, NX_GPIO_PADFUNC_1 },
		{ PAD_GPIO_E +  5, NX_GPIO_PADFUNC_1 },
		{ PAD_GPIO_E +  6, NX_GPIO_PADFUNC_1 },
		/* DATA */
		{ PAD_GPIO_D + 28, NX_GPIO_PADFUNC_1 },
		{ PAD_GPIO_D + 29, NX_GPIO_PADFUNC_1 },
		{ PAD_GPIO_D + 30, NX_GPIO_PADFUNC_1 },
		{ PAD_GPIO_D + 31, NX_GPIO_PADFUNC_1 },
		{ PAD_GPIO_E +  0, NX_GPIO_PADFUNC_1 },
		{ PAD_GPIO_E +  1, NX_GPIO_PADFUNC_1 },
		{ PAD_GPIO_E +  2, NX_GPIO_PADFUNC_1 },
		{ PAD_GPIO_E +  3, NX_GPIO_PADFUNC_1 },
		};

		pr_info("%s\n", __func__);

		pad = (u_int *)port;
		len = sizeof(port)/sizeof(port[0]);

		for (i = 0; i < len; i++) {
			io = *pad++;
			fn = *pad++;
			nxp_soc_gpio_set_io_dir(io, 0);
			nxp_soc_gpio_set_io_func(io, fn);
		}

		is_camera_port_configured = true;
	}
}

static bool camera_power_enabled = false;
static void camera_power_control(int enable)
{
	struct regulator *cam_io_28V = NULL;
	struct regulator *cam_core_18V = NULL;
	struct regulator *cam_io_33V = NULL;

	if (enable && camera_power_enabled)
		return;
	if (!enable && !camera_power_enabled)
		return;

	cam_core_18V = regulator_get(NULL, "vcam1_1.8V");
	if (IS_ERR(cam_core_18V)) {
		pr_err("%s: failed to regulator_get() for vcam1_1.8V",
			__func__);
		return;
	}

	cam_io_28V = regulator_get(NULL, "vcam_2.8V");
	if (IS_ERR(cam_io_28V)) {
		pr_err("%s: failed to regulator_get() for vcam_2.8V",
			__func__);
		return;
	}

	cam_io_33V = regulator_get(NULL, "vcam_3.3V");
	if (IS_ERR(cam_io_33V)) {
		pr_err("%s: failed to regulator_get() for vcam_3.3V",
			__func__);
		return;
	}

	pr_info("%s: %d\n", __func__, enable);
	if (enable) {
		regulator_enable(cam_core_18V);
		regulator_enable(cam_io_28V);
		regulator_enable(cam_io_33V);
	} else {
		regulator_disable(cam_io_33V);
		regulator_disable(cam_io_28V);
		regulator_disable(cam_core_18V);
	}

	regulator_put(cam_io_28V);
	regulator_put(cam_core_18V);
	regulator_put(cam_io_33V);

	camera_power_enabled = enable ? true : false;
}

static bool is_back_camera_enabled = false;
static bool is_back_camera_power_state_changed = false;
static bool is_front_camera_enabled = false;
static bool is_front_camera_power_state_changed = false;

static int front_camera_power_enable(bool on);
static int back_camera_power_enable(bool on)
{
    unsigned int io = CFG_IO_CAMERA_BACK_POWER_DOWN;
    unsigned int reset_io = CFG_IO_CAMERA_RESET;
    PM_DBGOUT("%s: is_back_camera_enabled %d, on %d\n", __func__, is_back_camera_enabled, on);
    if (on) {
        front_camera_power_enable(0);
        if (!is_back_camera_enabled) {
            camera_power_control(1);
            /* PD signal */
            nxp_soc_gpio_set_out_value(io, 0);
            nxp_soc_gpio_set_io_dir(io, 1);
            nxp_soc_gpio_set_io_func(io, nxp_soc_gpio_get_altnum(io));
            nxp_soc_gpio_set_out_value(io, 1);
            camera_common_set_clock(24000000);
            /* mdelay(10); */
            mdelay(1);
            nxp_soc_gpio_set_out_value(io, 0);
            /* RST signal */
            nxp_soc_gpio_set_out_value(reset_io, 1);
            nxp_soc_gpio_set_io_dir(reset_io, 1);
            nxp_soc_gpio_set_io_func(reset_io, nxp_soc_gpio_get_altnum(io));
            nxp_soc_gpio_set_out_value(reset_io, 0);
            /* mdelay(100); */
            mdelay(1);
            nxp_soc_gpio_set_out_value(reset_io, 1);
            /* mdelay(100); */
            mdelay(1);
            is_back_camera_enabled = true;
            is_back_camera_power_state_changed = true;
        } else {
            is_back_camera_power_state_changed = false;
        }
    } else {
        if (is_back_camera_enabled) {
            nxp_soc_gpio_set_out_value(io, 1);
            nxp_soc_gpio_set_out_value(reset_io, 0);
            is_back_camera_enabled = false;
            is_back_camera_power_state_changed = true;
        } else {
            nxp_soc_gpio_set_out_value(io, 1);
            nxp_soc_gpio_set_io_dir(io, 1);
            nxp_soc_gpio_set_io_func(io, nxp_soc_gpio_get_altnum(io));
            nxp_soc_gpio_set_out_value(io, 1);
            is_back_camera_power_state_changed = false;
        }

        if (!(is_back_camera_enabled || is_front_camera_enabled)) {
            camera_power_control(0);
        }
    }

    return 0;
}

static bool back_camera_power_state_changed(void)
{
    return is_back_camera_power_state_changed;
}

static struct i2c_board_info back_camera_i2c_boardinfo[] = {
    {
        I2C_BOARD_INFO("SP2518", 0x60>>1),
    },
};

static int front_camera_power_enable(bool on)
{
    unsigned int io = CFG_IO_CAMERA_FRONT_POWER_DOWN;
    unsigned int reset_io = CFG_IO_CAMERA_RESET;
    PM_DBGOUT("%s: is_front_camera_enabled %d, on %d\n", __func__, is_front_camera_enabled, on);
    if (on) {
        back_camera_power_enable(0);
        if (!is_front_camera_enabled) {
            camera_power_control(1);
            /* First RST signal to low */
            nxp_soc_gpio_set_out_value(reset_io, 1);
            nxp_soc_gpio_set_io_dir(reset_io, 1);
            nxp_soc_gpio_set_io_func(reset_io, nxp_soc_gpio_get_altnum(io));
            nxp_soc_gpio_set_out_value(reset_io, 0);
            mdelay(1);

            /* PWDN signal High to Low */
            nxp_soc_gpio_set_out_value(io, 0);
            nxp_soc_gpio_set_io_dir(io, 1);
            nxp_soc_gpio_set_io_func(io, nxp_soc_gpio_get_altnum(io));
            nxp_soc_gpio_set_out_value(io, 1);
            camera_common_set_clock(24000000);
            mdelay(10);
            /* mdelay(1); */
            nxp_soc_gpio_set_out_value(io, 0);
            /* mdelay(10); */
            mdelay(10);

            /* RST signal  to High */
            nxp_soc_gpio_set_out_value(reset_io, 1);
            /* mdelay(100); */
            mdelay(5);

            is_front_camera_enabled = true;
            is_front_camera_power_state_changed = true;
        } else {
            is_front_camera_power_state_changed = false;
        }
    } else {
        if (is_front_camera_enabled) {
            nxp_soc_gpio_set_out_value(io, 1);
            is_front_camera_enabled = false;
            is_front_camera_power_state_changed = true;
        } else {
            nxp_soc_gpio_set_out_value(io, 1);
            is_front_camera_power_state_changed = false;
        }
        if (!(is_back_camera_enabled || is_front_camera_enabled)) {
            camera_power_control(0);
        }
    }

    return 0;
}

static bool front_camera_power_state_changed(void)
{
    return is_front_camera_power_state_changed;
}

static struct i2c_board_info front_camera_i2c_boardinfo[] = {
    {
        I2C_BOARD_INFO("SP0838", 0x18),
    },
};

static struct nxp_v4l2_i2c_board_info sensor[] = {
    {
        .board_info = &back_camera_i2c_boardinfo[0],
        .i2c_adapter_id = 0,
    },
    {
        .board_info = &front_camera_i2c_boardinfo[0],
        .i2c_adapter_id = 0,
    },
};


static struct nxp_capture_platformdata capture_plat_data[] = {
    {
        /* back_camera 656 interface */
        .module = 1,
        .sensor = &sensor[0],
        .type = NXP_CAPTURE_INF_PARALLEL,
        .parallel = {
            /* for 656 */
            .is_mipi        = false,
            .external_sync  = false, /* 656 interface */
            .h_active       = 800,
            .h_frontporch   = 7,
            .h_syncwidth    = 1,
            .h_backporch    = 10,
            .v_active       = 600,
            .v_frontporch   = 0,
            .v_syncwidth    = 2,
            .v_backporch    = 3,
            .clock_invert   = true,
            .port           = 0,
            .data_order     = NXP_VIN_Y0CBY1CR,
            .interlace      = false,
            .clk_rate       = 24000000,
            .late_power_down = true,
            .power_enable   = back_camera_power_enable,
            .power_state_changed = back_camera_power_state_changed,
            .set_clock      = camera_common_set_clock,
            .setup_io       = camera_common_vin_setup_io,
        },
        .deci = {
            .start_delay_ms = 0,
            .stop_delay_ms  = 0,
        },
    },
    {
        /* front_camera 601 interface */
        .module = 1,
        .sensor = &sensor[1],
        .type = NXP_CAPTURE_INF_PARALLEL,
        .parallel = {
            .is_mipi        = false,
            .external_sync  = true,
            .h_active       = 640,
            .h_frontporch   = 0,
            .h_syncwidth    = 0,
            .h_backporch    = 2,
            .v_active       = 480,
            .v_frontporch   = 0,
            .v_syncwidth    = 0,
            .v_backporch    = 2,
            .clock_invert   = false,
            .port           = 0,
            .data_order     = NXP_VIN_CBY0CRY1,
            .interlace      = false,
            .clk_rate       = 24000000,
            .late_power_down = true,
            .power_enable   = front_camera_power_enable,
            .power_state_changed = front_camera_power_state_changed,
            .set_clock      = camera_common_set_clock,
            .setup_io       = camera_common_vin_setup_io,
        },
	.deci = {
			.start_delay_ms = 0,
			.stop_delay_ms  = 0,
		},
	},
	{ 0, NULL, 0, },
};

/* out platformdata */
static struct i2c_board_info hdmi_edid_i2c_boardinfo = {
	I2C_BOARD_INFO("nxp_edid", 0xA0>>1),
};

static struct nxp_v4l2_i2c_board_info edid = {
	.board_info = &hdmi_edid_i2c_boardinfo,
	.i2c_adapter_id = 0,
};

static struct i2c_board_info hdmi_hdcp_i2c_boardinfo = {
	I2C_BOARD_INFO("nxp_hdcp", 0x74>>1),
};

static struct nxp_v4l2_i2c_board_info hdcp = {
	.board_info = &hdmi_hdcp_i2c_boardinfo,
	.i2c_adapter_id = 0,
};


static void hdmi_set_int_external(int gpio)
{
	nxp_soc_gpio_set_int_enable(gpio, 0);
	nxp_soc_gpio_set_int_mode(gpio, 1); /* high level */
	nxp_soc_gpio_set_int_enable(gpio, 1);
	nxp_soc_gpio_clr_int_pend(gpio);
}

static void hdmi_set_int_internal(int gpio)
{
	nxp_soc_gpio_set_int_enable(gpio, 0);
	nxp_soc_gpio_set_int_mode(gpio, 0); /* low level */
	nxp_soc_gpio_set_int_enable(gpio, 1);
	nxp_soc_gpio_clr_int_pend(gpio);
}

static int hdmi_read_hpd_gpio(int gpio)
{
	return nxp_soc_gpio_get_in_value(gpio);
}

static struct nxp_out_platformdata out_plat_data = {
	.hdmi = {
	.internal_irq = 0,
	.external_irq = 0, /* PAD_GPIO_A + 19, */
	.set_int_external = hdmi_set_int_external,
	.set_int_internal = hdmi_set_int_internal,
	.read_hpd_gpio = hdmi_read_hpd_gpio,
	.edid = &edid,
	.hdcp = &hdcp,
	},
};

static struct nxp_v4l2_platformdata v4l2_plat_data = {
	.captures = &capture_plat_data[0],
	.out = &out_plat_data,
};

static struct platform_device nxp_v4l2_dev = {
	.name = NXP_V4L2_DEV_NAME,
	.id = 0,
	.dev = {
		.platform_data = &v4l2_plat_data,
	},
};
#endif /* CONFIG_V4L2_NXP || CONFIG_V4L2_NXP_MODULE */

/*------------------------------------------------------------------------------
 * SSP/SPI
 */
#if defined(CONFIG_SPI_SPIDEV) || defined(CONFIG_SPI_SPIDEV_MODULE)
#include <linux/spi/spi.h>
static void spi0_cs(u32 chipselect)
{
#if (CFG_SPI0_CS_GPIO_MODE)
	if (nxp_soc_gpio_get_io_func(CFG_SPI0_CS) !=
	nxp_soc_gpio_get_altnum(CFG_SPI0_CS))
		nxp_soc_gpio_set_io_func(CFG_SPI0_CS,
			nxp_soc_gpio_get_altnum(CFG_SPI0_CS));

	nxp_soc_gpio_set_io_dir(CFG_SPI0_CS, 1);
	nxp_soc_gpio_set_out_value(CFG_SPI0_CS , chipselect);
#endif
}
static struct pl022_config_chip spi0_info = {
	/* available POLLING_TRANSFER, INTERRUPT_TRANSFER, DMA_TRANSFER */
	.com_mode = CFG_SPI0_COM_MODE,
	.iface = SSP_INTERFACE_MOTOROLA_SPI,
	/* We can only act as master but SSP_SLAVE is possible in theory */
	.hierarchy = SSP_MASTER,
	/* 0 = drive TX even as slave, 1 = do not drive TX as slave */
	.slave_tx_disable = 1,
	.rx_lev_trig = SSP_RX_4_OR_MORE_ELEM,
	.tx_lev_trig = SSP_TX_4_OR_MORE_EMPTY_LOC,
	.ctrl_len = SSP_BITS_8,
	.wait_state = SSP_MWIRE_WAIT_ZERO,
	.duplex = SSP_MICROWIRE_CHANNEL_FULL_DUPLEX,
	/*
	 * This is where you insert a call to a function to enable CS
	 * (usually GPIO) for a certain chip.
	 */
#if (CFG_SPI0_CS_GPIO_MODE)
	.cs_control = spi0_cs,
#endif
	.clkdelay = SSP_FEEDBACK_CLK_DELAY_1T,

};

static struct spi_board_info spi_plat_board[] __initdata = {
	[0] = {
	.modalias = "spidev", /* fixup */
	.max_speed_hz = 3125000, /* max spi clock (SCK) speed in HZ */
	.bus_num = 0,
	.chip_select = 0,
	.controller_data = &spi0_info,
	.mode = SPI_MODE_3 | SPI_CPOL | SPI_CPHA,
	},
};

#endif

/*------------------------------------------------------------------------------
 * DW MMC board config
 */
#if defined(CONFIG_MMC_DW)
static int _dwmci_ext_cd_init(void (*notify_func)(struct platform_device *,
			int state))
{
	return 0;
}

static int _dwmci_ext_cd_cleanup(void (*notify_func)(struct platform_device *,
			int state))
{
	return 0;
}

static int _dwmci_get_ro(u32 slot_id)
{
	return 0;
}

#ifdef CONFIG_MMC_NXP_CH2
static int _dwmci2_init(u32 slot_id, irq_handler_t handler, void *data)
{
	struct dw_mci *host = (struct dw_mci *)data;
	int io  = CFG_SDMMC2_DETECT_IO;
	int irq = IRQ_GPIO_START + io;
	int id  = 0, ret = 0;

	pr_info("dw_mmc.%d: Using external card detect irq %3d (io %2d)\n",
			id, irq, io);

	ret  = request_irq(irq, handler,
			IRQF_TRIGGER_FALLING | IRQF_TRIGGER_RISING,
			DEV_NAME_SDHC "2", (void *)host->slot[slot_id]);
	if (0 > ret)
		pr_err("dw_mmc dw_mmc.%d: fail request interrupt %d ...\n",
			id, irq);
	return 0;
}

static int _dwmci2_get_cd(u32 slot_id)
{
	int io = CFG_SDMMC2_DETECT_IO;

	return nxp_soc_gpio_get_in_value(io);
}

static struct dw_mci_board _dwmci2_data = {
	.quirks = DW_MCI_QUIRK_HIGHSPEED,
	.bus_hz = 100 * 1000 * 1000,
	.caps = MMC_CAP_CMD23,
	.detect_delay_ms = 200,
	.cd_type = DW_MCI_CD_EXTERNAL,
	.clk_dly = DW_MMC_DRIVE_DELAY(0) | DW_MMC_SAMPLE_DELAY(0) |
		DW_MMC_DRIVE_PHASE(2) | DW_MMC_SAMPLE_PHASE(1),
	.init = _dwmci2_init,
	.get_ro = _dwmci_get_ro,
	.get_cd = _dwmci2_get_cd,
	.ext_cd_init = _dwmci_ext_cd_init,
	.ext_cd_cleanup = _dwmci_ext_cd_cleanup,
#if defined(CONFIG_MMC_DW_IDMAC) && defined(CONFIG_MMC_NXP_CH2_USE_DMA)
	.mode = DMA_MODE,
#else
	.mode = PIO_MODE,
#endif

};
#endif

#ifdef CONFIG_MMC_NXP_CH0
static struct dw_mci_board _dwmci0_data = {
	.quirks	= DW_MCI_QUIRK_BROKEN_CARD_DETECTION |
			DW_MCI_QUIRK_HIGHSPEED |
			DW_MMC_QUIRK_HW_RESET_PW |
			DW_MCI_QUIRK_NO_DETECT_EBIT,
	.bus_hz	= 100 * 1000 * 1000,
	.hs_over_clk = 50 * 1000 * 1000,
	.caps = MMC_CAP_UHS_DDR50 |
		MMC_CAP_NONREMOVABLE |
		MMC_CAP_4_BIT_DATA | MMC_CAP_CMD23 |
		MMC_CAP_ERASE | MMC_CAP_HW_RESET,
	.desc_sz = 4,
	.detect_delay_ms = 200,
	.clk_dly = DW_MMC_DRIVE_DELAY(0) | DW_MMC_SAMPLE_DELAY(0) |
			DW_MMC_DRIVE_PHASE(2) | DW_MMC_SAMPLE_PHASE(1),
#if defined(CONFIG_MMC_DW_IDMAC) && defined(CONFIG_MMC_NXP_CH0_USE_DMA)
	.mode = DMA_MODE,
#else
	.mode = PIO_MODE,
#endif
};
#endif

#endif /* CONFIG_MMC_DW */



/*------------------------------------------------------------------------------
 * USB HSIC power control.
 */
int nxp_hsic_phy_pwr_on(struct platform_device *pdev, bool on)
{
	return 0;
}
EXPORT_SYMBOL(nxp_hsic_phy_pwr_on);

#ifdef CONFIG_MFD_NXE2000
/*------------------------------------------------------------------------------
 * USB OTGVBUS power control.
 */
void nxp_otgvbus_pwr_set(int enable)
{
	int vbus_det = CFG_GPIO_OTG_VBUS_DET;
	if (vbus_det > -1)
		nxp_soc_gpio_set_out_value(CFG_GPIO_OTG_VBUS_DET, enable);
}
EXPORT_SYMBOL(nxp_otgvbus_pwr_set);
#endif

#if defined(CONFIG_NXP_HDMI_CEC)
static struct platform_device hdmi_cec_device = {
	.name			= NXP_HDMI_CEC_DRV_NAME,
};
#endif /* CONFIG_NXP_HDMI_CEC */

/*------------------------------------------------------------------------------
 * register board platform devices
 */
void __init nxp_board_devices_register(void)
{
	pr_info("[Register board platform devices]\n");

#if defined(CONFIG_ARM_NXP_CPUFREQ)
	pr_info("plat: add dynamic frequency (pll.%d)\n",
			dfs_plat_data.pll_dev);
	platform_device_register(&dfs_plat_device);
#endif

#ifdef CONFIG_SENSORS_NXP_ADC_TEMP
	pr_info("plat: add device adc temp\n");
	platform_device_register(&adc_temp_plat_device);
#endif

#if defined(CONFIG_FB_NXP)
	pr_info("plat: add framebuffer\n");
	platform_add_devices(fb_devices, ARRAY_SIZE(fb_devices));
#endif

#if defined(CONFIG_MMC_DW)
	#ifdef CONFIG_MMC_NXP_CH0
	nxp_mmc_add_device(0, &_dwmci0_data);
	#endif

	#if 0 /* def CONFIG_MMC_NXP_CH1 */
	nxp_mmc_add_device(1, &_dwmci1_data);
	#endif

	#ifdef CONFIG_MMC_NXP_CH2
	nxp_mmc_add_device(2, &_dwmci2_data);
	#endif
#endif

#if defined(CONFIG_DM9000) || defined(CONFIG_DM9000_MODULE)
	pr_info("plat: add device dm9000 net\n");
	platform_device_register(&dm9000_plat_device);
#endif

#if defined(CONFIG_BACKLIGHT_PWM)
	pr_info("plat: add backlight pwm device\n");
	platform_device_register(&bl_plat_device);
#endif


#if defined(CONFIG_KEYBOARD_NXP_KEY) || defined(CONFIG_KEYBOARD_NXP_KEY_MODULE)
	pr_info("plat: add device keypad\n");
	platform_device_register(&key_plat_device);
#endif

#if defined(CONFIG_I2C_NXP)
	pr_info("plat: add device i2c bus(array:%d)\n",
			ARRAY_SIZE(i2c_devices));
	platform_add_devices(i2c_devices, ARRAY_SIZE(i2c_devices));
#endif

#if defined(CONFIG_REGULATOR_NXE2000)
	pr_info("plat: add device nxe2000 pmic\n");
	i2c_register_board_info(NXE2000_I2C_BUS,
		nxe2000_i2c_boardinfo, ARRAY_SIZE(nxe2000_i2c_boardinfo));
#endif

#if defined(CONFIG_SND_CODEC_TLV320AIC32X4) ||	\
defined(CONFIG_SND_CODEC_TLV320AIC32X4_MODULE)
	pr_info("plat: add device asoc-tlv320aic32x4\n");
	i2c_register_board_info(TLV320AIC32X4_I2C_BUS,
			&tlv320aic32x4_i2c_bdi, 1);
	platform_device_register(&tlv320aic32x4_dai);
#endif

#if defined(CONFIG_SND_CODEC_SMARTVOICE)
	pr_info("plat: add device SPI smart-voice capture\n");
	platform_device_register(&snd_svoice_dac);
	platform_device_register(&snd_spi_dev);
	platform_device_register(&snd_spi_svoice_card);

	pr_info("plat: add device I2S smart-voice card\n");
	platform_device_register(&snd_i2s_svoice_card);
#endif

#if defined(CONFIG_SND_SPDIF_TRANSCIEVER) ||	\
defined(CONFIG_SND_SPDIF_TRANSCIEVER_MODULE)
	pr_info("plat: add device spdif playback\n");
	platform_device_register(&spdif_transciever);
	platform_device_register(&spdif_trans_dai);
#endif

#if defined(CONFIG_V4L2_NXP) || defined(CONFIG_V4L2_NXP_MODULE)
	pr_info("plat: add device nxp-v4l2\n");
	platform_device_register(&nxp_v4l2_dev);
#endif

#if defined(CONFIG_NXP_MP2TS_IF)
	pr_info("plat: add device misc mpegts\n");
	platform_device_register(&mpegts_plat_device);
#endif

#if defined(CONFIG_SPI_SPIDEV) || defined(CONFIG_SPI_SPIDEV_MODULE)
	spi_register_board_info(spi_plat_board, ARRAY_SIZE(spi_plat_board));
	pr_info("plat: register spidev\n");
#endif

#if defined(CONFIG_NXP_HDMI_CEC)
	pr_info("plat: add device hdmi-cec\n");
	platform_device_register(&hdmi_cec_device);
#endif

#if defined(CONFIG_TOUCHSCREEN_TSC2007)
	pr_info("plat: add touch(tsc2007) device\n");
	i2c_register_board_info(TSC2007_I2C_BUS, &tsc2007_i2c_bdi, 1);
#endif

#if defined(CONFIG_PPM_NXP)
	pr_info("plat: add device ppm\n");
	platform_device_register(&ppm_device);
#endif

	/* END */
	pr_info("\n");
}
