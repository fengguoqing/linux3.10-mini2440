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
//#include "common-smdk.h"

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

static struct s3c2410fb_display mini2440_lcd_cfg __initdata = {

	.lcdcon5	= S3C2410_LCDCON5_FRM565 |
			  S3C2410_LCDCON5_INVVLINE |
			  S3C2410_LCDCON5_INVVFRAME |
			  S3C2410_LCDCON5_PWREN |
			  S3C2410_LCDCON5_HWSWP,

	.type		= S3C2410_LCDCON1_TFT,

	.width		= 240,
	.height		= 320,

	.pixclock	= 166667, /* HCLK 60 MHz, divisor 10 */
	.xres		= 240,
	.yres		= 320,
	.bpp		= 16,
	.left_margin	= 20,
	.right_margin	= 8,
	.hsync_len	= 4,
	.upper_margin	= 8,
	.lower_margin	= 7,
	.vsync_len	= 4,
};

static struct s3c2410fb_mach_info mini2440_fb_info __initdata = {
	.displays	= &mini2440_lcd_cfg,
	.num_displays	= 1,
	.default_display = 0,

#if 0
	/* currently setup by downloader */
	.gpccon		= 0xaa940659,
	.gpccon_mask	= 0xffffffff,
	.gpcup		= 0x0000ffff,
	.gpcup_mask	= 0xffffffff,
	.gpdcon		= 0xaa84aaa0,
	.gpdcon_mask	= 0xffffffff,
	.gpdup		= 0x0000faff,
	.gpdup_mask	= 0xffffffff,
#endif

	.lpcsel		= ((0xCE6) & ~7) | 1<<4,
};

static struct mtd_partition mini2440_default_nand_part[] = {
    [0] = {
            .name = "supervivi", //这里是 bootloader 所在的分区，可以放置 u-boot, supervivi 等内容，对应/dev/mtdblock0
            .size = 0x00040000,
            .offset = 0,
        },

    [1] = {
            .name = "param", //这里是 supervivi 的参数区，其实也属于 bootloader 的一部分，如果 u-boot 比较大，可以把此区域覆盖掉，不会影响系统启动，对应/dev/mtdblock1
            .offset = 0x00040000,
            .size = 0x00020000,
        },

    [2] = {
            .name= "Kernel", //内核所在的分区，大小为 5M，足够放下大部分自己定制的巨型内核了，比如内核使用了更大的 Linux Logo 图片等，对应/dev/mtdblock2
            .offset = 0x00060000,
            .size = 0x00500000,
        },

    [3] = {
            .name = "root", //文件系统分区，友善之臂主要用来存放 yaffs2 文件系统内容，对应/dev/mtdblock3
            .offset = 0x00560000,
            .size = 1024 * 1024 * 1024,
        },

    [4] = {
            .name = "nand", //此区域代表了整片的 nand flash，主要是预留使用，比如以后可以通过应用程序访问读取/dev/mtdblock4 就能实现备份整片 nand flash 了。
            .offset = 0x00000000,
            .size = 1024 * 1024 * 1024,
        },
};

//这里是开发板的 nand flash 设置表，因为板子上只有一片，因此也就只有一个表
static struct s3c2410_nand_set mini2440_nand_sets[] = {
    [0] = {
            .name = "NAND",
            .nr_chips = 1,
            .nr_partitions = ARRAY_SIZE(mini2440_default_nand_part),
            .partitions = mini2440_default_nand_part,
        },
};

//这里是 nand flash 本身的一些特性，一般需要对照 datasheet 填写，大部分情况下按照以下参数填写即可
static struct s3c2410_platform_nand mini2440_nand_info = {
    .tacls = 20,
    .twrph0 = 60,
    .twrph1 = 20,
    .nr_sets = ARRAY_SIZE(mini2440_nand_sets),
    .sets = mini2440_nand_sets,
    .ignore_unset_ecc = 1,
};

static struct platform_device *mini2440_devices[] __initdata = {
	&s3c_device_ohci,
	&s3c_device_lcd,
	&s3c_device_wdt,
	&s3c_device_i2c0,
	&s3c_device_iis,
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
