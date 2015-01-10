/*
 * Copyright (C) 2011-2013 Freescale Semiconductor, Inc. All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include <linux/version.h>
#include <linux/types.h>
#include <linux/sched.h>
#include <linux/delay.h>
#include <linux/pm.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/init.h>
#include <linux/input.h>
#include <linux/nodemask.h>
#include <linux/clk.h>
#include <linux/platform_device.h>
#include <linux/fsl_devices.h>
#include <linux/spi/spi.h>
#include <linux/spi/flash.h>
#include <linux/i2c.h>
#include <linux/i2c/pca953x.h>
#include <linux/ata.h>
#include <linux/ion.h>
#include <linux/leds_pwm.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/map.h>
#include <linux/mtd/partitions.h>
#include <linux/regulator/consumer.h>
#include <linux/pmic_external.h>
#include <linux/pmic_status.h>
#include <linux/ipu.h>
#include <linux/mxcfb.h>
#include <linux/pwm.h>
#include <linux/pwm_backlight.h>
#include <linux/fec.h>
#include <linux/memblock.h>
#include <linux/gpio.h>
#include <linux/etherdevice.h>
#include <linux/regulator/anatop-regulator.h>
#include <linux/regulator/consumer.h>
#include <linux/regulator/machine.h>
#include <linux/regulator/fixed.h>
#include <linux/spi/tsc2006.h>
#include <linux/input/sitronix_i2c_touch.h>

#include <mach/common.h>
#include <mach/hardware.h>
#include <mach/mxc_dvfs.h>
#include <mach/memory.h>
#include <mach/iomux-mx6q.h>
#include <mach/imx-uart.h>
#include <mach/viv_gpu.h>
#include <mach/ahci_sata.h>
#include <mach/ipu-v3.h>
#include <mach/mxc_hdmi.h>
#include <mach/mxc_asrc.h>

#include <asm/irq.h>
#include <asm/setup.h>
#include <asm/mach-types.h>
#include <asm/mach/arch.h>
#include <asm/mach/time.h>

#include "usb.h"
#include "devices-imx6q.h"
#include "crm_regs.h"
#include "cpu_op-mx6.h"

#include "board-mx6q_udoo.h"
#include "board-mx6dl_udoo.h"

/******************* SD *******************/
#define MX6_SD1_CD			IMX_GPIO_NR(6, 15)
#define MX6_SD1_WP			IMX_GPIO_NR(5, 20)
#define MX6_SD4_CD			IMX_GPIO_NR(2, 6)
#define MX6_SD4_WP			IMX_GPIO_NR(2, 7)
/******************* SATA *******************/
#define MX6_SATA_LED		IMX_GPIO_NR(6, 11)
/******************* USB *******************/
#define MX6_USB_OTG_PWR		IMX_GPIO_NR(3, 22)
#define MX6_USB_HUB_RESET	IMX_GPIO_NR(7, 12)
/******************* LVDS *******************/
#define MX6_LVDS_BLT_CTRL   IMX_GPIO_NR(1, 6)
/******************* HDMI *******************/
#define MX6_HDMI_CHECK		IMX_GPIO_NR(5, 2)
/******************* ETHERNET *******************/
#define MX6_FEC_RESET		IMX_GPIO_NR(3, 23)
#define MX6_ENET_125MHz_EN			IMX_GPIO_NR(6, 24)
/******************* CAM *******************/
#define MX6_CAMERA_RST		IMX_GPIO_NR(6, 5)
/******************* TOUCH *******************/
#define MX6_UDOO_TOUCH_RST		IMX_GPIO_NR(1, 15)
#define MX6_UDOO_TOUCH_INT		IMX_GPIO_NR(1, 13)
/******************* EIM *******************/
#define MX6_ECSPI1_CS1		IMX_GPIO_NR(3, 19)
#define MX6_ECSPI1_CS2		IMX_GPIO_NR(3, 24)
#define MX6_ECSPI1_CS3		IMX_GPIO_NR(3, 25)
#define MX6_RTC_CS4			IMX_GPIO_NR(2, 17)
/******************* AUDIO *******************/
#ifdef CONFIG_SND_SOC_IMX_AC97_VT1613
#define AC97_GPIO_TXFS				IMX_GPIO_NR(1, 11)
#define SSI_CH_NUMBER				1
#endif
/******************* CAN *******************/
#define MX6_CAN1_STBY		IMX_GPIO_NR(1, 2)
#define MX6_CAN1_EN			IMX_GPIO_NR(1, 4)

#define MX6_CAP_TCH_INT1	IMX_GPIO_NR(1, 9)
#define MX6_CSI0_RST		IMX_GPIO_NR(4, 22)
#define MX6_CSI0_PWN		IMX_GPIO_NR(4, 21)

#define MX6_IO_MICRO        IMX_GPIO_NR(2, 4)

#define MX6_SD3_CD 			IMX_GPIO_NR(7, 0)

#define MX6_SD3_WP_PADCFG	(PAD_CTL_PKE | PAD_CTL_PUE |	\
		PAD_CTL_PUS_22K_UP | PAD_CTL_SPEED_MED |	\
		PAD_CTL_DSE_40ohm | PAD_CTL_HYS)


void __init early_console_setup(unsigned long base, struct clk *clk);
static struct clk *sata_clk;

extern char *gp_reg_id;
extern char *soc_reg_id;
extern char *pu_reg_id;


extern struct regulator *(*get_cpu_regulator)(void);
extern void (*put_cpu_regulator)(void);


/***********************************************************************
 *                                   SD                                *
 ***********************************************************************/

/* for QUAD and DUAL */
static iomux_v3_cfg_t MX6Q_USDHC_PAD_SETTING(3, 50);
static iomux_v3_cfg_t MX6Q_USDHC_PAD_SETTING(3, 100);
static iomux_v3_cfg_t MX6Q_USDHC_PAD_SETTING(3, 200);

static iomux_v3_cfg_t MX6Q_USDHC_PAD_SETTING(4, 50);
static iomux_v3_cfg_t MX6Q_USDHC_PAD_SETTING(4, 100);
static iomux_v3_cfg_t MX6Q_USDHC_PAD_SETTING(4, 200);

/* for SOLO and DUAL LITE */
static iomux_v3_cfg_t MX6DL_USDHC_PAD_SETTING(3, 50);
static iomux_v3_cfg_t MX6DL_USDHC_PAD_SETTING(3, 100);
static iomux_v3_cfg_t MX6DL_USDHC_PAD_SETTING(3, 200);

static iomux_v3_cfg_t MX6DL_USDHC_PAD_SETTING(4, 50);
static iomux_v3_cfg_t MX6DL_USDHC_PAD_SETTING(4, 100);
static iomux_v3_cfg_t MX6DL_USDHC_PAD_SETTING(4, 200);


enum sd_pad_mode {
	SD_PAD_MODE_LOW_SPEED,
	SD_PAD_MODE_MED_SPEED,
	SD_PAD_MODE_HIGH_SPEED,
};


static int plt_sd3_pad_change(int clock) {
	static enum sd_pad_mode pad_mode = SD_PAD_MODE_LOW_SPEED;

	if (clock > 100000000) {
		if (pad_mode == SD_PAD_MODE_HIGH_SPEED)
			return 0;

		pad_mode = SD_PAD_MODE_HIGH_SPEED;
		if (cpu_is_mx6q()) {
			return mxc_iomux_v3_setup_multiple_pads(mx6q_sd3_200mhz,
						ARRAY_SIZE(mx6dl_sd3_200mhz));
		} else if (cpu_is_mx6dl()) {
			return mxc_iomux_v3_setup_multiple_pads(mx6dl_sd3_200mhz,
						ARRAY_SIZE(mx6dl_sd3_200mhz));
		}
	} else if (clock > 52000000) {
		if (pad_mode == SD_PAD_MODE_MED_SPEED)
			return 0;

		pad_mode = SD_PAD_MODE_MED_SPEED;
		if (cpu_is_mx6q()) {
			return mxc_iomux_v3_setup_multiple_pads(mx6q_sd3_100mhz,
						ARRAY_SIZE(mx6dl_sd3_200mhz));
		} else if (cpu_is_mx6dl()) {
			return mxc_iomux_v3_setup_multiple_pads(mx6dl_sd3_100mhz,
						ARRAY_SIZE(mx6dl_sd3_200mhz));
		}
	} else {
		if (pad_mode == SD_PAD_MODE_LOW_SPEED)
			return 0;

		pad_mode = SD_PAD_MODE_LOW_SPEED;
		if (cpu_is_mx6q()) {
			return mxc_iomux_v3_setup_multiple_pads(mx6q_sd3_50mhz,
						ARRAY_SIZE(mx6dl_sd3_200mhz));
		} else if (cpu_is_mx6dl()) {
			return mxc_iomux_v3_setup_multiple_pads(mx6dl_sd3_50mhz,
						ARRAY_SIZE(mx6dl_sd3_200mhz));
		}
	}
}

static int plt_sd4_pad_change(int clock) {
	static enum sd_pad_mode pad_mode = SD_PAD_MODE_LOW_SPEED;

	if (clock > 100000000) {
		if (pad_mode == SD_PAD_MODE_HIGH_SPEED)
			return 0;

		pad_mode = SD_PAD_MODE_HIGH_SPEED;
		if (cpu_is_mx6q()) {
			return mxc_iomux_v3_setup_multiple_pads(mx6q_sd4_200mhz,
						ARRAY_SIZE(mx6dl_sd4_200mhz));
		} else if (cpu_is_mx6dl()) {
			return mxc_iomux_v3_setup_multiple_pads(mx6dl_sd4_200mhz,
						ARRAY_SIZE(mx6dl_sd4_200mhz));
		}
	} else if (clock > 52000000) {
		if (pad_mode == SD_PAD_MODE_MED_SPEED)
			return 0;

		pad_mode = SD_PAD_MODE_MED_SPEED;
		if (cpu_is_mx6q()) {
			return mxc_iomux_v3_setup_multiple_pads(mx6q_sd4_100mhz,
						ARRAY_SIZE(mx6dl_sd4_200mhz));
		} else if (cpu_is_mx6dl()) {
			return mxc_iomux_v3_setup_multiple_pads(mx6dl_sd4_100mhz,
						ARRAY_SIZE(mx6dl_sd4_200mhz));
		}
	} else {
		if (pad_mode == SD_PAD_MODE_LOW_SPEED)
			return 0;

		pad_mode = SD_PAD_MODE_LOW_SPEED;
		if (cpu_is_mx6q()) {
			return mxc_iomux_v3_setup_multiple_pads(mx6q_sd4_50mhz,
						ARRAY_SIZE(mx6dl_sd4_200mhz));
		} else if (cpu_is_mx6dl()) {
			return mxc_iomux_v3_setup_multiple_pads(mx6dl_sd4_50mhz,
						ARRAY_SIZE(mx6dl_sd4_200mhz));
		}
	}
}



	/* MMC 2 - SD_primary */
static const struct esdhc_platform_data mx6q_sd3_data __initconst = {
	.cd_gpio = MX6_SD3_CD,	
	.wp_gpio = -1,
	.always_present = 1,
	.keep_power_at_suspend = 1,
};


/***********************************************************************
 *                                  SATA                               *
 ***********************************************************************/

/* HW Initialization, if return 0, initialization is successful. */
static int mx6q_sata_init(struct device *dev, void __iomem *addr) {
	u32 tmpdata;
	int ret = 0;
	struct clk *clk;

	sata_clk = clk_get(dev, "imx_sata_clk");
	if (IS_ERR(sata_clk)) {
		dev_err(dev, "no sata clock.\n");
		return PTR_ERR(sata_clk);
	}
	ret = clk_enable(sata_clk);
	if (ret) {
		dev_err(dev, "can't enable sata clock.\n");
		goto put_sata_clk;
	}

	/* Set PHY Paremeters, two steps to configure the GPR13,
	 * one write for rest of parameters, mask of first write is 0x07FFFFFD,
	 * and the other one write for setting the mpll_clk_off_b
	 *.rx_eq_val_0(iomuxc_gpr13[26:24]),
	 *.los_lvl(iomuxc_gpr13[23:19]),
	 *.rx_dpll_mode_0(iomuxc_gpr13[18:16]),
	 *.sata_speed(iomuxc_gpr13[15]),
	 *.mpll_ss_en(iomuxc_gpr13[14]),
	 *.tx_atten_0(iomuxc_gpr13[13:11]),
	 *.tx_boost_0(iomuxc_gpr13[10:7]),
	 *.tx_lvl(iomuxc_gpr13[6:2]),
	 *.mpll_ck_off(iomuxc_gpr13[1]),
	 *.tx_edgerate_0(iomuxc_gpr13[0]),
	 */
	tmpdata = readl(IOMUXC_GPR13);
	writel(((tmpdata & ~0x07FFFFFD) | 0x0593A044), IOMUXC_GPR13);

	/* enable SATA_PHY PLL */
	tmpdata = readl(IOMUXC_GPR13);
	writel(((tmpdata & ~0x2) | 0x2), IOMUXC_GPR13);

	/* Get the AHB clock rate, and configure the TIMER1MS reg later */
	clk = clk_get(NULL, "ahb");
	if (IS_ERR(clk)) {
		dev_err(dev, "no ahb clock.\n");
		ret = PTR_ERR(clk);
		goto release_sata_clk;
	}
	tmpdata = clk_get_rate(clk) / 1000;
	clk_put(clk);

#ifdef CONFIG_SATA_AHCI
	ret = sata_init(addr, tmpdata);
	if (ret == 0)
		return ret;
#else
	usleep_range(1000, 2000);
	/* AHCI PHY enter into PDDQ mode if the AHCI module is not enabled */
	tmpdata = readl(addr + PORT_PHY_CTL);
	writel(tmpdata | PORT_PHY_CTL_PDDQ_LOC, addr + PORT_PHY_CTL);
	pr_info("No AHCI save PWR: PDDQ %s\n", ((readl(addr + PORT_PHY_CTL)
					>> 20) & 1) ? "enabled" : "disabled");
#endif

release_sata_clk:
	clk_disable(sata_clk);
put_sata_clk:
	clk_put(sata_clk);

	return ret;
}

#ifdef CONFIG_SATA_AHCI
static void mx6q_sata_exit(struct device *dev) {
	clk_disable(sata_clk);
	clk_put(sata_clk);
}

static struct ahci_platform_data mx6q_sata_data = {
	.init = mx6q_sata_init,
	.exit = mx6q_sata_exit,
};

#endif



/***********************************************************************
 *                                  UART                               *
 ***********************************************************************/

static inline void mx6q_init_uart(void) {
	imx6q_add_imx_uart(0, NULL);
	imx6q_add_imx_uart(1, NULL);
    imx6q_add_imx_uart(3, NULL);
}

/***********************************************************************
 *                    MIPI - CSI 5Mpx OV5640 CAMERA on CN11                        *
 ***********************************************************************/

#if defined(CONFIG_MXC_CAMERA_OV5640_MIPI) || defined(CONFIG_MXC_CAMERA_OV5640_MIPI_MODULE)

static void ov5640_mipi_camera_io_init(void)
{
	struct clk *clko1;
	
	if (cpu_is_mx6q())
	{
		mxc_iomux_v3_setup_pad(MX6Q_PAD_CSI0_MCLK__CCM_CLKO);
		mxc_iomux_v3_setup_pad(MX6Q_PAD_CSI0_DAT18__GPIO_6_4);
	}
	else
	{
		mxc_iomux_v3_setup_pad(MX6DL_PAD_CSI0_MCLK__CCM_CLKO);
		mxc_iomux_v3_setup_pad(MX6DL_PAD_CSI0_DAT18__GPIO_6_4);
	}
	
	clko1 = clk_get(NULL, "clko_clk");
	if (IS_ERR(clko1)) {
		pr_err("can't get CLKO1 clock.\n");
	} else {
		long round = clk_round_rate(clko1, 27000000);
		clk_set_rate(clko1, round);
		clk_enable(clko1);
	}
	
	/* Camera reset */
	gpio_request(MX6_CAMERA_RST, "cam-reset");
	gpio_direction_output(MX6_CAMERA_RST, 0);
	msleep(1);
	gpio_set_value(MX6_CAMERA_RST, 1);
	msleep(100);
	
/* for mx6dl, mipi virtual channel 1 connect to csi 0*/
	if (cpu_is_mx6dl())
		mxc_iomux_set_gpr_register(13, 0, 3, 0);	
}

static void ov5640_mipi_camera_powerdown(int powerdown)
{
}

static struct fsl_mxc_camera_platform_data ov5640_mipi_data = {
	.mclk = 22000000,
	.csi = 0,
	.io_init = ov5640_mipi_camera_io_init,
	.pwdn = ov5640_mipi_camera_powerdown,
};
#endif //  if defined(CONFIG_MXC_CAMERA_OV5640_MIPI) || defined(CONFIG_MXC_CAMERA_OV5640_MIPI_MODULE)


/***********************************************************************
 *                                   USB                               *
 ***********************************************************************/

static void imx6q_usbotg_vbus(bool on) {
	if (on)
		gpio_set_value(MX6_USB_OTG_PWR, 1);
	else
		gpio_set_value(MX6_USB_OTG_PWR, 0);
}

static void __init imx6q_init_usb(void) {
	int ret = 0;

	imx_otg_base = MX6_IO_ADDRESS(MX6Q_USB_OTG_BASE_ADDR);
	/* disable external charger detect,
	 * or it will affect signal quality at dp .
	 */
	ret = gpio_request(MX6_USB_OTG_PWR, "usb-pwr");
	if (ret) {
		pr_err("failed to get GPIO MX6_USB_OTG_PWR: %d\n",
			ret);
		return;
	}
	gpio_direction_output(MX6_USB_OTG_PWR, 0);
	mxc_iomux_set_gpr_register(1, 13, 1, 1);

	mx6_set_otghost_vbus_func(imx6q_usbotg_vbus);
//	mx6_usb_dr_init();
//	mx6_usb_h1_init();
}



/***********************************************************************
 *                               GPU - IPU                             *
 ***********************************************************************/

static struct viv_gpu_platform_data imx6q_gpu_pdata __initdata = {
	.reserved_mem_size = SZ_128M,
};

static struct imx_asrc_platform_data imx_asrc_data = {
	.channel_bits = 4,
	.clk_map_ver = 2,
};

static struct imx_ipuv3_platform_data ipu_data[] = {
	{
	.rev = 4,
	.csi_clk[0] = "clko2_clk",
	}, {
	.rev = 4,
	.csi_clk[0] = "clko2_clk",
	},
};



/***********************************************************************
 *                               ETHERNET                              *
 ***********************************************************************/

static int mx6q_fec_phy_reset(struct phy_device *phydev) {
	int ret;
	mxc_iomux_v3_setup_pad(MX6Q_PAD_RGMII_RX_CTL__GPIO_6_24);
	ret = gpio_request(MX6_ENET_125MHz_EN, "125mHz_en");
	if (!ret) {
		gpio_direction_output(MX6_ENET_125MHz_EN, 1);
		gpio_set_value(MX6_ENET_125MHz_EN, 1);
		printk("Resetting ethernet physical layer.\n");
		gpio_set_value(MX6_FEC_RESET, 0);
		msleep(2);
		gpio_set_value(MX6_FEC_RESET, 1);
		msleep(1);
		gpio_free(MX6_ENET_125MHz_EN);
		mxc_iomux_v3_setup_pad(MX6Q_PAD_RGMII_RX_CTL__ENET_RGMII_RX_CTL);
	} else {
		printk(KERN_ERR "Reset of ethernet physical layer failed.\n");
	}
	return 0;
}

static int mx6q_fec_phy_init(struct phy_device *phydev) {
	int prod_ID;
	/* prefer master mode, disable 1000 Base-T capable */
	phy_write(phydev, 0x9, 0x1c00);
	prod_ID = phy_read(phydev, 0x3);

#define KSZ9021_id 0x1611
#define KSZ9031_id 0x1621

        switch (prod_ID) {
                case KSZ9021_id:
			printk("Activating ethernet physical layer Micrel KSZ9021 Gigabit PHY.\n");
			/* min rx data delay */
			phy_write(phydev, 0x0b, 0x8105);
			phy_write(phydev, 0x0c, 0x0000);

			/* max rx/tx clock delay, min rx/tx control delay */
			phy_write(phydev, 0x0b, 0x8104);
			phy_write(phydev, 0x0c, 0xf0f0);
//			phy_write(phydev, 0x0b, 0x104);
			break;

                case KSZ9031_id:
            printk("Activating ethernet physical layer Micrel KSZ9031 Gigabit PHY.\n");
			phy_write(phydev, 0x0d, 0x0002); // reg. select operation
			phy_write(phydev, 0x0e, 0x0008); // reg. idx address
			phy_write(phydev, 0x0d, 0x4002); // data operation - no autoinc.
			phy_write(phydev, 0x0e, 0x03FF);
			break;
                default:
                        printk("Warning: Product ID of physical layer not recognized [id = 0x%04x unknown]\n", prod_ID);
                        break;
        }

	return 0;
}

static struct fec_platform_data fec_data __initdata = {
	.init = mx6q_fec_phy_init,
	//.reset = mx6q_fec_phy_reset,
	.phy = PHY_INTERFACE_MODE_RGMII,
};



/***********************************************************************
 *                                  HDMI                               *
 ***********************************************************************/

static void hdmi_init(int ipu_id, int disp_id) {
	int hdmi_mux_setting;

	if ((ipu_id > 1) || (ipu_id < 0)) {
		pr_err("Invalid IPU select for HDMI: %d. Set to 0\n", ipu_id);
		ipu_id = 0;
	}

	if ((disp_id > 1) || (disp_id < 0)) {
		pr_err("Invalid DI select for HDMI: %d. Set to 0\n", disp_id);
		disp_id = 0;
	}

	/* Configure the connection between IPU1/2 and HDMI */
	hdmi_mux_setting = 2*ipu_id + disp_id;

	/* GPR3, bits 2-3 = HDMI_MUX_CTL */
	mxc_iomux_set_gpr_register(3, 2, 2, hdmi_mux_setting);

	mxc_iomux_set_gpr_register(0, 0, 1, 1);
}

static struct fsl_mxc_hdmi_platform_data hdmi_data = {
	.init = hdmi_init,
};

static struct fsl_mxc_hdmi_core_platform_data hdmi_core_data = {
	.ipu_id = 0,
	.disp_id = 0,
};



/***********************************************************************
 *                                 VIDEO                               *
 ***********************************************************************/

static struct ipuv3_fb_platform_data fb_data[] = {
	{ /*fb0*/
	.disp_dev = "ldb",
	.interface_pix_fmt = IPU_PIX_FMT_RGB24,
	.mode_str = "LDB-WSXGA",
	.default_bpp = 16,
	.int_clk = false,
	}, {
	.disp_dev = "ldb",
	.interface_pix_fmt = IPU_PIX_FMT_RGB666,
	.mode_str = "CLAA-WVGA",
	.default_bpp = 16,
	.int_clk = false,
	}, {
	.disp_dev = "ldb",
	.interface_pix_fmt = IPU_PIX_FMT_RGB666,
	.mode_str = "CLAA-WVGA",
	.default_bpp = 16,
	.int_clk = false,
	},
};

static struct fsl_mxc_lcd_platform_data lcdif_data = {
	.ipu_id = 0,
	.disp_id = 0,
	.default_ifmt = IPU_PIX_FMT_RGB565,
};

static struct fsl_mxc_ldb_platform_data ldb_data = {
	.ipu_id = 1,
	.disp_id = 0,
	.ext_ref = 1,
	.mode = LDB_SEP0,
	.sec_ipu_id = 1,
	.sec_disp_id = 1,
};

static struct platform_pwm_backlight_data mx6_pwm_backlight_data = {
	.pwm_id = 1,
	.max_brightness = 255,
	.dft_brightness = 128,
	.pwm_period_ns = 50000,
};

static struct fsl_mxc_capture_platform_data capture_data[] = {
	{
		.csi = 0,
		.ipu = 0,
		.mclk_source = 0,
		.is_mipi = 0,
	}, {
		.csi = 1,
		.ipu = 0,
		.mclk_source = 0,
		.is_mipi = 1,
	},
};



/***********************************************************************
 *                                  AUDIO                              *
 ***********************************************************************/

#ifdef CONFIG_SND_SOC_IMX_AC97_VT1613

static struct mxc_audio_platform_data mx6_audio_data;

static void audio_codec_ac97_cold_reset(struct snd_ac97 *ac97) {
    int ret = gpio_request(AC97_GPIO_TXFS, "ac97_reset");
	if (ret) {
		printk (KERN_ERR "request AC97_GPIO_TXFS failed!"); 
	} else {
		gpio_direction_output(AC97_GPIO_TXFS, 0);
	    udelay(2);
	    gpio_set_value(AC97_GPIO_TXFS, 0);
	    mdelay(1);
	    gpio_set_value(AC97_GPIO_TXFS, 1);
	    mdelay(100);
	}
	printk("audio_codec_ac97_cold_reset \n");
}

static struct imx_ssi_platform_data mx6_ssi_pdata = {
    .ac97_reset             = audio_codec_ac97_cold_reset,
	.flags = (IMX_SSI_USE_AC97 | IMX_SSI_DMA),
};

static struct mxc_audio_platform_data mx6_audio_data = {
	.ssi_num = SSI_CH_NUMBER,
	.src_port = SSI_CH_NUMBER,
	.ext_port = 6, // OUTPUT ON AUD6 FOR AUDIO MUXING
	.hp_gpio = -1,
};

#ifdef CONFIG_SND_SOC_IMX_SPDIF
static int spdif_clk_set_rate(struct clk *clk, unsigned long rate)
{
        unsigned long rate_actual;
        rate_actual = clk_round_rate(clk, rate);
        clk_set_rate(clk, rate_actual);
        return 0;
}

static struct mxc_spdif_platform_data mxc_spdif_data = {
        .spdif_tx               = 1,            /* enable tx */
        .spdif_rx               = 0,            /* disable rx */
        /*
         * spdif0_clk will be 454.7MHz divided by ccm dividers.
         *
         * 44.1KHz: 454.7MHz / 7 (ccm) / 23 (spdif) = 44,128 Hz ~ 0.06% error
         * 48KHz:   454.7MHz / 4 (ccm) / 37 (spdif) = 48,004 Hz ~ 0.01% error
         * 32KHz:   454.7MHz / 6 (ccm) / 37 (spdif) = 32,003 Hz ~ 0.01% error
         */
        .spdif_clk_44100        = 1,    /* tx clk from spdif0_clk_root */
        .spdif_clk_48000        = 1,    /* tx clk from spdif0_clk_root */
        .spdif_div_44100        = 23,
        .spdif_div_48000        = 37,
        .spdif_div_32000        = 37,
        .spdif_rx_clk           = 0,    /* rx clk from spdif stream */
        .spdif_clk_set_rate     = spdif_clk_set_rate,
        .spdif_clk              = NULL, /* spdif bus clk */
};
#endif

struct vt1613_codec_vibra_data {
        unsigned int    vibra_mclk;
};

struct vt1613_codec_audio_data {
        unsigned int    audio_mclk;
};

struct vt1613_codec_data {
        unsigned int    audio_mclk;
        struct vt1613_codec_audio_data          *audio;
        struct vt1613_codec_vibra_data          *vibra;
};

static struct platform_device mx6_imx_audio_device = {
        .name           = "imx-vt1613",
};

static struct platform_device mx6_audio_device = {
        .name           = "vt1613-ac97",
};
#endif

static int imx6q_init_audio(void) {
#ifdef CONFIG_SND_SOC_IMX_AC97_VT1613
	mxc_register_device(&mx6_imx_audio_device, &mx6_audio_data);
	mxc_register_device(&mx6_audio_device, NULL);
	imx6q_add_imx_ssi(SSI_CH_NUMBER - 1, &mx6_ssi_pdata);
#endif
	return 0;
}



/***********************************************************************
 *                            POWER MANAGEMENT                         *
 ***********************************************************************/

static const struct anatop_thermal_platform_data
	mx6q_anatop_thermal_data __initconst = {
		.name = "anatop_thermal",
};

static struct mxc_dvfs_platform_data dvfscore_data = {
	.reg_id = "cpu_vddgp",
	.soc_id = "cpu_vddsoc",
	.pu_id = "cpu_vddvpu",
	.clk1_id = "cpu_clk",
	.clk2_id = "gpc_dvfs_clk",
	.gpc_cntr_offset = MXC_GPC_CNTR_OFFSET,
	.ccm_cdcr_offset = MXC_CCM_CDCR_OFFSET,
	.ccm_cacrr_offset = MXC_CCM_CACRR_OFFSET,
	.ccm_cdhipr_offset = MXC_CCM_CDHIPR_OFFSET,
	.prediv_mask = 0x1F800,
	.prediv_offset = 11,
	.prediv_val = 3,
	.div3ck_mask = 0xE0000000,
	.div3ck_offset = 29,
	.div3ck_val = 2,
	.emac_val = 0x08,
	.upthr_val = 25,
	.dnthr_val = 9,
	.pncthr_val = 33,
	.upcnt_val = 10,
	.dncnt_val = 10,
	.delay_time = 80,
};

static void suspend_enter(void) {
	/* suspend preparation */
}

static void suspend_exit(void) {
	/* resume restore */
}

static const struct pm_platform_data mx6q_pm_data __initconst = {
	.name = "imx_pm",
	.suspend_enter = suspend_enter,
	.suspend_exit = suspend_exit,
};

static struct regulator_consumer_supply vmmc_consumers[] = {
	REGULATOR_SUPPLY("vmmc", "sdhci-esdhc-imx.1"),
	REGULATOR_SUPPLY("vmmc", "sdhci-esdhc-imx.2"),
	REGULATOR_SUPPLY("vmmc", "sdhci-esdhc-imx.3"),
};

static struct regulator_init_data vmmc_init = {
	.num_consumer_supplies = ARRAY_SIZE(vmmc_consumers),
	.consumer_supplies = vmmc_consumers,
};

static struct fixed_voltage_config vmmc_reg_config = {
	.supply_name	= "vmmc",
	.microvolts		= 3300000,
	.gpio			= -1,
	.init_data		= &vmmc_init,
};

static struct platform_device vmmc_reg_devices = {
	.name	= "reg-fixed-voltage",
	.id	= 3,
	.dev	= {
		.platform_data = &vmmc_reg_config,
	},
};



/***********************************************************************
 *                                  PCIE                               *
 ***********************************************************************/

#if defined(CONFIG_IMX_PCIE) 
static const struct imx_pcie_platform_data mx6_pcie_data  __initconst = {
	.pcie_pwr_en	= -EINVAL,
	.pcie_rst	= -EINVAL,
	.pcie_wake_up	= -EINVAL,
	.pcie_dis	= -EINVAL,
};
#endif



/***********************************************************************
 *                                 SPI-NOR                             *
 ***********************************************************************/

#if defined(CONFIG_MTD_M25P80) 
static struct mtd_partition imx6_spi_nor_partitions[] = {
	{
	 .name = "bootloader",
	 .offset = 0,
	 .size = 0x00040000,
	},
	{
	 .name = "kernel",
	 .offset = MTDPART_OFS_APPEND,
	 .size = MTDPART_SIZ_FULL,
	},
};

static struct flash_platform_data imx6_spi_flash_data = {
	.name = "m25p80",
	.parts = imx6_spi_nor_partitions,
	.nr_parts = ARRAY_SIZE(imx6_spi_nor_partitions),
	.type = "sst25vf016b",
};
#endif



/***********************************************************************
 *                                   SPI                               *
 ***********************************************************************/

static int mx6q_spi_cs[] = {
	MX6_ECSPI1_CS1,
	MX6_ECSPI1_CS2,
	MX6_ECSPI1_CS3,
	MX6_RTC_CS4,
};

static const struct spi_imx_master mx6q_spi_data __initconst = {
	.chipselect     = mx6q_spi_cs,
	.num_chipselect = ARRAY_SIZE(mx6q_spi_cs),
};

#ifdef CONFIG_TOUCHSCREEN_TSC2006
static struct tsc2006_platform_data tsc2006_config __initdata = {
};

static void tsc2006_dev_init(void) {
	return;

	if (gpio_request(MX6_HDMI_CHECK, "TSC2006 pendown") < 0) {
		printk("can't get tsc2006 pen down GPIO\n");
	}

        gpio_direction_input(MX6_HDMI_CHECK);
	msleep(5);
}

#endif

static struct spi_board_info imx6_spi_device[] __initdata = {
#if defined(CONFIG_MTD_M25P80)
	{
		.modalias = "m25p80",
		.max_speed_hz = 20000000, /* max spi clock (SCK) speed in HZ */
		.bus_num = 0,
		.chip_select = 0,
		.platform_data = &imx6_spi_flash_data,
	},
#endif
#ifdef CONFIG_TOUCHSCREEN_TSC2006
    {
        .modalias               = "tsc2006",
        .bus_num                = 0,
        .chip_select            = 1,
        .max_speed_hz           = 10*1000*1000,
        .mode                   = SPI_MODE_0,
        .platform_data          = &tsc2006_config,
		.irq 					= gpio_to_irq(MX6_HDMI_CHECK),
    },
#endif
	{
		.modalias				= "rtc-pcf2123",
		.bus_num				= 0,
		.chip_select			= 4,	
//		.controller_data		= 
		.mode					= SPI_MODE_0,
	},
};

static void spi_device_init(void) {
	spi_register_board_info(imx6_spi_device,
				ARRAY_SIZE(imx6_spi_device));
}



/***********************************************************************
 *                                   I2C                               *
 ***********************************************************************/

static struct imxi2c_platform_data mx6q_i2c0_data = {
	.bitrate = 100000,
};

static struct imxi2c_platform_data mx6q_i2c1_data = {
	.bitrate = 400000,
};

static struct imxi2c_platform_data mx6q_i2c2_data = {
	.bitrate = 400000,
};

static struct i2c_board_info mxc_i2c1_board_info[] __initdata = {
	{
		I2C_BOARD_INFO("mxc_hdmi_i2c", 0x50),
	},
};

#if defined(CONFIG_TOUCHSCREEN_SITRONIX_I2C_TOUCH) || defined(CONFIG_TOUCHSCREEN_SITRONIX_I2C_TOUCH_MODULE)
static void sitronix_reset_ic(void) {
	gpio_direction_output(MX6_UDOO_TOUCH_RST, 1);
	gpio_set_value(MX6_UDOO_TOUCH_RST, 0);
	mdelay(1);
	gpio_set_value(MX6_UDOO_TOUCH_RST, 1);
}

static int sitronix_get_int_status(void)
{
	return gpio_get_value(MX6_UDOO_TOUCH_INT);
}

struct sitronix_i2c_touch_platform_data 	touch_i2c_conf ={
	//.get_int_status = sitronix_get_int_status,
	.reset_ic = sitronix_reset_ic,
};
#endif // CONFIG_TOUCHSCREEN_SITRONIX_I2C_TOUCH

#ifdef CONFIG_TOUCHSCREEN_TSC2004
static int tsc2004_get_pendown_state (void) {
        return gpio_get_value(MX6_HDMI_CHECK);
}

static int tsc2004_init_platform_hw (void) {
	return 0;
}

static struct tsc2004_platform_data mx6_tsc2004 = {
	.model		= 2004,
	.x_plate_ohms 	= 180,
	.get_pendown_state	= tsc2004_get_pendown_state,
	.init_platform_hw = tsc2004_init_platform_hw,
};
#endif

static struct i2c_board_info mxc_i2c2_board_info[] __initdata = {
#ifdef CONFIG_TOUCHSCREEN_TSC2004
	{
		I2C_BOARD_INFO("tsc2004", 0x48),
		.platform_data 	= &mx6_tsc2004,
		.irq		= gpio_to_irq(MX6_HDMI_CHECK),
    },
#endif
#if defined(CONFIG_MXC_CAMERA_OV5640_MIPI) || defined(CONFIG_MXC_CAMERA_OV5640_MIPI_MODULE)
	{
		I2C_BOARD_INFO("ov5640_mipi", 0x3c),
       		.platform_data = (void *)&ov5640_mipi_data,
        },
#endif
#if defined(CONFIG_TOUCHSCREEN_SITRONIX_I2C_TOUCH) || defined(CONFIG_TOUCHSCREEN_SITRONIX_I2C_TOUCH_MODULE)
	{
		I2C_BOARD_INFO(SITRONIX_I2C_TOUCH_DRV_NAME, 0x55),
		.irq		= gpio_to_irq(MX6_UDOO_TOUCH_INT),
		.platform_data 	= &touch_i2c_conf,
	},
#endif
};

/***********************************************************************
 *                                   CAN                               *
 ***********************************************************************/

static const struct flexcan_platform_data
        mx6q_sabrelite_flexcan0_pdata __initconst = {
        .transceiver_switch = NULL,
};

/***********************************************************************
 *                                   ION                               *
 ***********************************************************************/

static struct ion_platform_data imx_ion_data = {
	.nr = 1,
	.heaps = {
		{
		.id = 0,
		.type = ION_HEAP_TYPE_CARVEOUT,
		.name = "vpu_ion",
		.size = SZ_16M,
		.cacheable = 1,
		},
	},
};

/***********************************************************************
 *                                   MIPI                              *
 ***********************************************************************/

static struct mipi_csi2_platform_data mipi_csi2_pdata = {
	.ipu_id	 = 0,
	.csi_id = 0,
	.v_channel = 0,
	.lanes = 2,
	.dphy_clk = "mipi_pllref_clk",
	.pixel_clk = "emi_clk",
};




/***********************************************************************
 *                               GPIO INPUT\OUTPUT                     *
 ***********************************************************************/

void set_gpios_directions(void)
{
	int ret;
	int i;

	printk("####Called set_gpios_direction\n");

	if(cpu_is_mx6q()){
	
		for (i = 0; i<ARRAY_SIZE(mx6q_set_in_inputmode) ; i++){
			// printk("INPUT GPIO %d:", i);
			ret = gpio_request (mx6q_set_in_inputmode[i], "gpio :D");
			if (ret) {
				printk("#### FAILED Eported gpio %d. error : %d \n", mx6q_set_in_inputmode[i], ret);
			
			} else {	
				gpio_direction_input (mx6q_set_in_inputmode[i]);
				gpio_export(mx6q_set_in_inputmode[i], true);
				// printk(KERN_INFO "> Eported gpio %d (in the test input array )\n", test_gpios_set_in_inputmode[i]);
			}
		}
	
	
		for (i = 0; i<ARRAY_SIZE(mx6q_set_in_outputmode_low); i++){
			// printk("OUTPUT GPIO BASSO %d:", i);
			ret = gpio_request (mx6q_set_in_outputmode_low[i], "gpio :S");
			if (ret) {
				printk("#### FAILED Eported gpio %d. error : %d \n", mx6q_set_in_outputmode_low[i], ret);
			} else {	
				gpio_direction_output (mx6q_set_in_outputmode_low[i], 0);
				gpio_export(mx6q_set_in_outputmode_low[i], true);
				// printk(KERN_INFO "> Eported gpio %d (in the test output LOW array )\n", test_gpios_set_in_outputmode_low[i]);
			}
		}

		for (i = 0; i<ARRAY_SIZE(mx6q_set_in_outputmode_high) ; i++){
			// printk("OUTPUT GPIO ALTO %d:", i);
			ret = gpio_request (mx6q_set_in_outputmode_high[i], "gpio :S"); 
			if (ret) {
				printk("#### FAILED Eported gpio %d. error : %d \n", mx6q_set_in_outputmode_high[i], ret);
			} else {	
				gpio_direction_output (mx6q_set_in_outputmode_high[i], 1);
				gpio_export(mx6q_set_in_outputmode_high[i], true);
				// printk(KERN_INFO ">Eported gpio %d (in the test output HIGH array\n", test_gpios_set_in_outputmode_high[i]);
			}
		}
	}
	else if(cpu_is_mx6dl()){
		for (i = 0; i<ARRAY_SIZE(mx6dl_set_in_inputmode) ; i++){
			// printk("INPUT GPIO %d:", i);
			ret = gpio_request (mx6dl_set_in_inputmode[i], "gpio :D");
			if (ret) {
				printk("#### FAILED Eported gpio %d. error : %d \n", mx6dl_set_in_inputmode[i], ret);
			
			} else {	
				gpio_direction_input (mx6dl_set_in_inputmode[i]);
				gpio_export(mx6dl_set_in_inputmode[i], true);
				// printk(KERN_INFO "> Eported gpio %d (in the test input array )\n", test_gpios_set_in_inputmode[i]);
			}
		}
	
	
		for (i = 0; i<ARRAY_SIZE(mx6dl_set_in_outputmode_low); i++){
			// printk("OUTPUT GPIO BASSO %d:", i);
			ret = gpio_request (mx6dl_set_in_outputmode_low[i], "gpio :S");
			if (ret) {
				printk("#### FAILED Eported gpio %d. error : %d \n", mx6dl_set_in_outputmode_low[i], ret);
			} else {	
				gpio_direction_output (mx6dl_set_in_outputmode_low[i], 0);
				gpio_export(mx6dl_set_in_outputmode_low[i], true);
				// printk(KERN_INFO "> Eported gpio %d (in the test output LOW array )\n", test_gpios_set_in_outputmode_low[i]);
			}
		}

		for (i = 0; i<ARRAY_SIZE(mx6dl_set_in_outputmode_high) ; i++){
			// printk("OUTPUT GPIO ALTO %d:", i);
			ret = gpio_request (mx6dl_set_in_outputmode_high[i], "gpio :S"); 
			if (ret) {
				printk("#### FAILED Eported gpio %d. error : %d \n", mx6dl_set_in_outputmode_high[i], ret);
			} else {	
				gpio_direction_output (mx6dl_set_in_outputmode_high[i], 1);
				gpio_export(mx6dl_set_in_outputmode_high[i], true);
				// printk(KERN_INFO ">Eported gpio %d (in the test output HIGH array\n", test_gpios_set_in_outputmode_high[i]);
			}
		}
	}
	else
		printk("#### No gpios to export");
}


/***********************************************************************
 *                              LED PWM                                *
 ***********************************************************************/

static struct led_pwm imx_pwm_leds[] = {
	{
		.name		= "pwm1",
		.pwm_id		= 0,
		.max_brightness	= 32768,
		.pwm_period_ns	= 1000000,
	},
	{
		.name		= "pwm3",
		.pwm_id		= 2,
		.max_brightness	= 32768,
		.pwm_period_ns	= 1000000,
	},
	{
		.name		= "pwm4",
		.pwm_id		= 3,
		.max_brightness	= 32768,
		.pwm_period_ns	= 1000000,
	},
};

static struct led_pwm_platform_data imx_pwm_data = {
	.num_leds	= ARRAY_SIZE(imx_pwm_leds),
	.leds		= imx_pwm_leds,
};

static struct platform_device imx_pwm_leds_dev = {
	.name	= "leds_pwm",
	.id	= -1,
	.dev	= {
		.platform_data = &imx_pwm_data,
	},
};


/***********************************************************************
 *                               BOARD INIT                            *
 ***********************************************************************/

static void __init fixup_mxc_board(struct machine_desc *desc, struct tag *tags,
				   char **cmdline, struct meminfo *mi)
{
}

/*!
 * Board specific initialization.
 */
static void __init mx6_board_init(void)
{
	int i, ret;
	struct clk *clko2;
	struct clk *new_parent;
	int rate;
	
	printk("UDOO board init\n");
	
	if (cpu_is_mx6q())
		mxc_iomux_v3_setup_multiple_pads(mx6qd_UDOO_pads,
			ARRAY_SIZE(mx6qd_UDOO_pads));
	else if (cpu_is_mx6dl()) {
		mxc_iomux_v3_setup_multiple_pads(mx6sdl_UDOO_pads,
			ARRAY_SIZE(mx6sdl_UDOO_pads));
	}
	

	set_gpios_directions();

	ret = gpio_request (MX6_IO_MICRO, "IO MICRO");
	if (ret) {
		printk("failed to get MX6_IO_MICRO: %d\n", ret);
	} else {	
		gpio_direction_input (MX6_IO_MICRO);
	}

	ret = gpio_request (MX6_LVDS_BLT_CTRL, "LVDS backlight");
	if (ret) {
		printk("failed to get MX6_LVDS_BLT_CTRL: %d\n", ret);
	} else {
		gpio_direction_output(MX6_LVDS_BLT_CTRL, 1);
	}
	
#ifdef CONFIG_FEC_1588
	/* Set GPIO_16 input for IEEE-1588 ts_clk and RMII reference clock
	 * For MX6 GPR1 bit21 meaning:
	 * Bit21:       0 - GPIO_16 pad output
	 *              1 - GPIO_16 pad input
	 */
	mxc_iomux_set_gpr_register(1, 21, 1, 1);
#endif

/*
#ifdef CONFIG_TOUCHSCREEN_TSC2006
	tsc2006_dev_init ();
#endif
*/

//   GPIO for Ethernet reset
	gpio_request(MX6_FEC_RESET, "fec-reset");
	gpio_direction_output(MX6_FEC_RESET, 1);
//   GPIO for Ethernet reset
 
	gp_reg_id = dvfscore_data.reg_id;
	soc_reg_id = dvfscore_data.soc_id;
	pu_reg_id = dvfscore_data.pu_id;
	mx6q_init_uart();

#if defined(CONFIG_TOUCHSCREEN_SITRONIX_I2C_TOUCH) || defined(CONFIG_TOUCHSCREEN_SITRONIX_I2C_TOUCH_MODULE)
// Sitronix touch Initialization
	gpio_request(MX6_UDOO_TOUCH_RST, "Multitouch Reset");
	sitronix_reset_ic();
// Sitronix touch Initialization
#endif // CONFIG_TOUCHSCREEN_SITRONIX_I2C_TOUCH

	/*
	 * MX6DL/Solo only supports single IPU
	 * The following codes are used to change ipu id
	 * and display id information for MX6DL/Solo. Then
	 * register 1 IPU device and up to 2 displays for
	 * MX6DL/Solo
	 */
	if (cpu_is_mx6dl()) {
		ldb_data.ipu_id = 0;
		ldb_data.disp_id = 0;
		ldb_data.sec_ipu_id = 0;
		ldb_data.sec_disp_id = 1;
		hdmi_core_data.disp_id = 1;
	}

	imx6q_add_mxc_hdmi_core(&hdmi_core_data);

	imx6q_add_ipuv3(0, &ipu_data[0]);
	if (cpu_is_mx6q()) {
		imx6q_add_ipuv3(1, &ipu_data[1]);
		for (i = 0; i < 4 && i < ARRAY_SIZE(fb_data); i++)
			imx6q_add_ipuv3fb(i, &fb_data[i]);
	} else
		for (i = 0; i < 2 && i < ARRAY_SIZE(fb_data); i++)
			imx6q_add_ipuv3fb(i, &fb_data[i]);

	imx6q_add_vdoa();
	imx6q_add_lcdif(&lcdif_data);
	imx6q_add_ldb(&ldb_data);
	imx6q_add_v4l2_output(0);
	imx6q_add_v4l2_capture(0, &capture_data[0]);
	imx6q_add_v4l2_capture(1, &capture_data[1]);
	imx6q_add_mipi_csi2(&mipi_csi2_pdata);
	imx6q_add_imx_snvs_rtc();

	imx6q_add_imx_i2c(0, &mx6q_i2c0_data);
	imx6q_add_imx_i2c(1, &mx6q_i2c1_data);
	imx6q_add_imx_i2c(2, &mx6q_i2c2_data);
	i2c_register_board_info(1, mxc_i2c1_board_info,
			ARRAY_SIZE(mxc_i2c1_board_info));
	i2c_register_board_info(2, mxc_i2c2_board_info,
			ARRAY_SIZE(mxc_i2c2_board_info));

	/* SPI */
//	imx6q_add_ecspi(0, &mx6q_spi_data);
//	spi_device_init();;

	imx6q_add_mxc_hdmi(&hdmi_data);
	
	if (imx_ion_data.heaps[0].size)
		imx6q_add_ion(0, &imx_ion_data,
			sizeof(imx_ion_data) + sizeof(struct ion_platform_heap));

	imx6q_add_anatop_thermal_imx(1, &mx6q_anatop_thermal_data);
	mx6q_fec_phy_reset(NULL);
	imx6_init_fec(fec_data);
	imx6q_add_pm_imx(0, &mx6q_pm_data);

	imx6q_add_sdhci_usdhc_imx(2, &mx6q_sd3_data);

	imx_add_viv_gpu(&imx6_gpu_data, &imx6q_gpu_pdata);
	imx6q_init_usb();

#ifdef CONFIG_SATA_AHCI
	if (cpu_is_mx6q())
		imx6q_add_ahci(0, &mx6q_sata_data);
#endif

	imx6q_add_vpu();
	imx6q_init_audio();
	platform_device_register(&vmmc_reg_devices);
	imx_asrc_data.asrc_core_clk = clk_get(NULL, "asrc_clk");
	imx_asrc_data.asrc_audio_clk = clk_get(NULL, "asrc_serial_clk");
	imx6q_add_asrc(&imx_asrc_data);
      //  imx6q_add_flexcan0(&mx6q_sabrelite_flexcan0_pdata);

	/* release USB Hub reset */
	gpio_set_value(MX6_USB_HUB_RESET, 1);

	imx6q_add_mxc_pwm(0);
	imx6q_add_mxc_pwm(1);
	imx6q_add_mxc_pwm(2);
	imx6q_add_mxc_pwm(3);

	imx6q_add_mxc_pwm_backlight(0, &mx6_pwm_backlight_data);

	imx6q_add_otp();
	imx6q_add_viim();
	imx6q_add_imx2_wdt(0, NULL);
	imx6q_add_dma();

	imx6q_add_dvfs_core(&dvfscore_data);

	imx6q_add_hdmi_soc();
	imx6q_add_hdmi_soc_dai();

#if defined(CONFIG_MX6_PCIE) 
	imx6q_add_pcie(&mx6_pcie_data);
#endif
	clko2 = clk_get(NULL, "clko2_clk");
	if (IS_ERR(clko2))
		pr_err("can't get CLKO2 clock.\n");

	new_parent = clk_get(NULL, "osc_clk");
	if (!IS_ERR(new_parent)) {
		clk_set_parent(clko2, new_parent);
		clk_put(new_parent);
	}
	rate = clk_round_rate(clko2, 24000000);
	clk_set_rate(clko2, rate);
	clk_enable(clko2);
	
	imx6q_add_busfreq();

	imx6_add_armpmu();

#ifdef CONFIG_SND_SOC_IMX_SPDIF
        mxc_spdif_data.spdif_core_clk = clk_get_sys("mxc_spdif.0", NULL);
        clk_put(mxc_spdif_data.spdif_core_clk);
        imx6q_add_spdif(&mxc_spdif_data);
        imx6q_add_spdif_dai();
        imx6q_add_spdif_audio_device();
#endif

	imx6q_add_perfmon(0);
	imx6q_add_perfmon(1);
	imx6q_add_perfmon(2);
	platform_device_register(&imx_pwm_leds_dev);
}

extern void __iomem *twd_base;
static void __init mx6_timer_init(void) {
	struct clk *uart_clk;
#ifdef CONFIG_LOCAL_TIMERS
	twd_base = ioremap(LOCAL_TWD_ADDR, SZ_256);
	BUG_ON(!twd_base);
#endif
	mx6_clocks_init(32768, 24000000, 0, 0);

	uart_clk = clk_get_sys("imx-uart.0", NULL);
	early_console_setup(UART2_BASE_ADDR, uart_clk);
}

static struct sys_timer mx6_timer = {
	.init   = mx6_timer_init,
};

static void __init mx6q_reserve(void)
{
	phys_addr_t phys;
#if defined(CONFIG_MXC_GPU_VIV) || defined(CONFIG_MXC_GPU_VIV_MODULE)
	if (imx6q_gpu_pdata.reserved_mem_size) {
		phys = memblock_alloc_base(imx6q_gpu_pdata.reserved_mem_size,
					   SZ_4K, SZ_1G);
		memblock_free(phys, imx6q_gpu_pdata.reserved_mem_size);
		memblock_remove(phys, imx6q_gpu_pdata.reserved_mem_size);
		imx6q_gpu_pdata.reserved_mem_base = phys;
	}
#endif
#if defined(CONFIG_ION)
	if (imx_ion_data.heaps[0].size) {
		phys = memblock_alloc(imx_ion_data.heaps[0].size, SZ_4K);
		memblock_remove(phys, imx_ion_data.heaps[0].size);
		imx_ion_data.heaps[0].base = phys;
	}
#endif
}

/*
 * initialize __mach_desc_MX6 data structure.
 */
MACHINE_START(MX6_UDOO, "i.Mx6 UDOO Board")
	/* Maintainer: Freescale Semiconductor, Inc. */
	.boot_params = MX6_PHYS_OFFSET + 0x100,
	.fixup = fixup_mxc_board,
	.map_io = mx6_map_io,
	.init_irq = mx6_init_irq,
	.init_machine = mx6_board_init,
	.timer = &mx6_timer,
	.reserve = mx6q_reserve,
MACHINE_END

