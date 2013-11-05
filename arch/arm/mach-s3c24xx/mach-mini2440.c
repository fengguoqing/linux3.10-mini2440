/* linux/arch/arm/mach-s3c2440/mach-mini2440.c
 *
 * Copyright (c) 2004-2005 Simtec Electronics
 *	Ben Dooks <ben@simtec.co.uk>
 *  Feng Guoqing <fengguoqing611@gmail.com>
 *  modify to support mini2440
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
*/

#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/interrupt.h>
#include <linux/list.h>
#include <linux/timer.h>
#include <linux/init.h>
#include <linux/serial_core.h>
#include <linux/platform_device.h>
#include <linux/io.h>

#include <linux/mtd/mtd.h>
#include <linux/mtd/nand.h>
#include <linux/mtd/nand_ecc.h>
#include <linux/mtd/partitions.h>
#include <linux/platform_data/mtd-nand-s3c2410.h>

#include <asm/mach/arch.h>
#include <asm/mach/map.h>
#include <asm/mach/irq.h>

#include <mach/hardware.h>
#include <asm/irq.h>
#include <asm/mach-types.h>

#include <plat/regs-serial.h>
#include <mach/regs-gpio.h>
#include <mach/regs-lcd.h>

#include <mach/fb.h>
#include <linux/platform_data/i2c-s3c2410.h>

#include <plat/clock.h>
#include <plat/devs.h>
#include <plat/cpu.h>
#include <plat/samsung-time.h>

#include "common.h"
#include <linux/dm9000.h>

#include <linux/gpio.h>
#include <linux/mmc/host.h>
#include <linux/platform_data/mmc-s3cmci.h>


static struct map_desc mini2440_iodesc[] __initdata = {
	/* ISA IO Space map (memory space selected by A24) */

	{
		.virtual	= (u32)S3C24XX_VA_ISA_WORD,
		.pfn		= __phys_to_pfn(S3C2410_CS2),
		.length		= 0x10000,
		.type		= MT_DEVICE,
	}, {
		.virtual	= (u32)S3C24XX_VA_ISA_WORD + 0x10000,
		.pfn		= __phys_to_pfn(S3C2410_CS2 + (1<<24)),
		.length		= SZ_4M,
		.type		= MT_DEVICE,
	}, {
		.virtual	= (u32)S3C24XX_VA_ISA_BYTE,
		.pfn		= __phys_to_pfn(S3C2410_CS2),
		.length		= 0x10000,
		.type		= MT_DEVICE,
	}, {
		.virtual	= (u32)S3C24XX_VA_ISA_BYTE + 0x10000,
		.pfn		= __phys_to_pfn(S3C2410_CS2 + (1<<24)),
		.length		= SZ_4M,
		.type		= MT_DEVICE,
	}
};

#define UCON S3C2410_UCON_DEFAULT | S3C2410_UCON_UCLK
#define ULCON S3C2410_LCON_CS8 | S3C2410_LCON_PNONE | S3C2410_LCON_STOPB
#define UFCON S3C2410_UFCON_RXTRIG8 | S3C2410_UFCON_FIFOMODE

static struct s3c2410_uartcfg mini2440_uartcfgs[] __initdata = {
	[0] = {
		.hwport	     = 0,
		.flags	     = 0,
		.ucon	     = 0x3c5,
		.ulcon	     = 0x03,
		.ufcon	     = 0x51,
	},
	[1] = {
		.hwport	     = 1,
		.flags	     = 0,
		.ucon	     = 0x3c5,
		.ulcon	     = 0x03,
		.ufcon	     = 0x51,
	},
	/* IR port */
	[2] = {
		.hwport	     = 2,
		.flags	     = 0,
		.ucon	     = 0x3c5,
		.ulcon	     = 0x43,
		.ufcon	     = 0x51,
	}
};

/* LCD driver info */
#define LCD_WIDTH 240 
#define LCD_HEIGHT 320 
#define LCD_PIXCLOCK 146250//170000
#define LCD_RIGHT_MARGIN 25 
#define LCD_LEFT_MARGIN 0 
#define LCD_HSYNC_LEN 4 
#define LCD_UPPER_MARGIN 1 
#define LCD_LOWER_MARGIN 4 
#define LCD_VSYNC_LEN 1
#define LCD_CON5 (S3C2410_LCDCON5_FRM565 | S3C2410_LCDCON5_INVVDEN \
                    | S3C2410_LCDCON5_INVVFRAME | S3C2410_LCDCON5_INVVLINE \
                    | S3C2410_LCDCON5_INVVCLK | S3C2410_LCDCON5_HWSWP )


static struct s3c2410fb_display mini2440_lcd_cfg __initdata = {

	.lcdcon5	= LCD_CON5,

	.type		= S3C2410_LCDCON1_TFT,

	.width		= LCD_WIDTH,
	.height		= LCD_HEIGHT,

	.pixclock	= LCD_PIXCLOCK, /* HCLK 60 MHz, divisor 10 */
	.xres		= LCD_WIDTH,
	.yres		= LCD_HEIGHT,
	.bpp		= 16,
	.left_margin	= LCD_LEFT_MARGIN + 1,
	.right_margin	= LCD_RIGHT_MARGIN + 1,
	.hsync_len	= LCD_HSYNC_LEN + 1,
	.upper_margin	= LCD_UPPER_MARGIN + 1,
	.lower_margin	= LCD_LOWER_MARGIN + 1,
	.vsync_len	= LCD_VSYNC_LEN + 1,
};

static struct s3c2410fb_mach_info mini2440_fb_info __initdata = {
	.displays	= &mini2440_lcd_cfg,
	.num_displays	= 1,
	.default_display = 0,

	/* currently setup by downloader */
	.gpccon		= 0xaa955699,
	.gpccon_mask	= 0xffc003cc,
	.gpcup		= 0x0000ffff,
	.gpcup_mask	= 0xffffffff,
	.gpdcon		= 0xaa95aaa1,
	.gpdcon_mask	= 0xffc0fff0,
	.gpdup		= 0x0000faff,
	.gpdup_mask	= 0xffffffff,

	.lpcsel		= 0xf82,
};

/* NAND driver info */
static struct mtd_partition mini2440_default_nand_part[] = {
    [0] = {
            .name = "u-boot", //u-boot map to /dev/mtdblock0
            .size = 0x00040000,
            .offset = 0,
        },

    [1] = {
            .name = "param", //u-boot param map to /dev/mtdblock1
            .offset = 0x00040000,
            .size = 0x00020000,
        },

    [2] = {
            .name= "Kernel", // /dev/mtdblock2
            .offset = 0x00060000,
            .size = 0x00500000,
        },

    [3] = {
            .name = "root", // yaffs2 map to /dev/mtdblock3
            .offset = 0x00560000,
            .size = 1024 * 1024 * 1024,
        },

    [4] = {
            .name = "nand",
            .offset = 0x00000000,
            .size = 1024 * 1024 * 1024,
        },
};


static struct s3c2410_nand_set mini2440_nand_sets[] = {
    [0] = {
            .name = "NAND",
            .nr_chips = 1,
            .nr_partitions = ARRAY_SIZE(mini2440_default_nand_part),
            .partitions = mini2440_default_nand_part,
        },
};


static struct s3c2410_platform_nand mini2440_nand_info = {
    .tacls = 20,
    .twrph0 = 60,
    .twrph1 = 20,
    .nr_sets = ARRAY_SIZE(mini2440_nand_sets),
    .sets = mini2440_nand_sets,
    .ignore_unset_ecc = 1,
};

/* DM9000AEP 10/100 ethernet controller */ 
#define MACH_MINI2440_DM9K_BASE (S3C2410_CS4 + 0x300)

static struct resource mini2440_dm9k_resource[] = { 
    [0] = { 
        .start = MACH_MINI2440_DM9K_BASE,
        .end = MACH_MINI2440_DM9K_BASE + 3,
        .flags = IORESOURCE_MEM 
    }, 
    [1] = {
        .start = MACH_MINI2440_DM9K_BASE + 4,
        .end = MACH_MINI2440_DM9K_BASE + 7,
        .flags = IORESOURCE_MEM
    }, 
    [2] = {
        .start = IRQ_EINT7,
        .end = IRQ_EINT7,
        .flags = IORESOURCE_IRQ | IORESOURCE_IRQ_HIGHEDGE,
    } 
};

/* 
* * * The DM9000 has no eeprom, and it's MAC address is set by 
* * * the bootloader before starting the kernel. 
* * */ 
static struct dm9000_plat_data mini2440_dm9k_pdata = { 
    .flags = (DM9000_PLATF_16BITONLY | DM9000_PLATF_NO_EEPROM), 
}; 

static struct platform_device mini2440_device_eth = { 
    .name = "dm9000", 
    .id = -1, 
    .num_resources  = ARRAY_SIZE(mini2440_dm9k_resource), 
    .resource = mini2440_dm9k_resource, 
    .dev = { 
        .platform_data = &mini2440_dm9k_pdata, 
    }, 
}; 

/* MMC/SD */ 
static struct s3c24xx_mci_pdata mini2440_mmc_cfg = { 
    .gpio_detect = S3C2410_GPG(8), 
    .gpio_wprotect = S3C2410_GPH(8), 
    .set_power = NULL, 
    .ocr_avail = MMC_VDD_32_33|MMC_VDD_33_34,
}; 



static struct platform_device *mini2440_devices[] __initdata = {
	&s3c_device_ohci,
	&s3c_device_lcd,
	&s3c_device_rtc,
	&s3c_device_wdt,
	&s3c_device_i2c0,
	&s3c_device_iis,
	&mini2440_device_eth,
	&s3c_device_sdi,
    &s3c_device_nand,
};

static void __init mini2440_map_io(void)
{
	s3c24xx_init_io(mini2440_iodesc, ARRAY_SIZE(mini2440_iodesc));
	s3c24xx_init_clocks(12000000);
	s3c24xx_init_uarts(mini2440_uartcfgs, ARRAY_SIZE(mini2440_uartcfgs));
	samsung_set_timer_source(SAMSUNG_PWM3, SAMSUNG_PWM4);
}

static void __init mini2440_machine_init(void)
{
	s3c24xx_fb_set_platdata(&mini2440_fb_info);
    s3c_nand_set_platdata(&mini2440_nand_info);
    s3c24xx_mci_set_platdata(&mini2440_mmc_cfg);
	s3c_i2c0_set_platdata(NULL);

	platform_add_devices(mini2440_devices, ARRAY_SIZE(mini2440_devices));
	//smdk_machine_init();
}

MACHINE_START(MINI2440, "FriendlyARM MINI2440 development board")
	/* Maintainer: Ben Dooks <ben-linux@fluff.org> */
	.atag_offset	= 0x100,

	.init_irq	= s3c2440_init_irq,
	.map_io		= mini2440_map_io,
	.init_machine	= mini2440_machine_init,
	.init_time	= samsung_timer_init,
	.restart	= s3c244x_restart,
MACHINE_END
