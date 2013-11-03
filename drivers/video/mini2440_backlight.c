/* linux/drivers/video/mini2440_backlight.c
 *	Copyright (c) 2013 Feng Guoqing
 *
 * mini2440 LCD backlight Driver
 *
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file COPYING in the main directory of this archive for
 * more details.
 *
*/

#include <linux/errno.h> 
#include <linux/kernel.h> 
#include <linux/module.h> 
#include <linux/slab.h> 
#include <linux/input.h> 
#include <linux/init.h> 
#include <linux/serio.h> 
#include <linux/delay.h> 
#include <linux/clk.h> 
#include <linux/miscdevice.h> 
#include <linux/gpio.h> 
#include <asm/io.h> 
#include <asm/irq.h> 
#include <asm/uaccess.h> 
#include <mach/regs-clock.h> 
#include <plat/regs-timer.h> 
#include <mach/regs-gpio.h> 
#include <linux/cdev.h>

#define DEVICE_NAME  "mini2440_backlight"

static unsigned int bl_state;

static inline void set_bl(int state) { 
    bl_state = !!state;
    gpio_set_value(S3C2410_GPG(4), bl_state);
}
static inline unsigned int get_bl(void) { 
    return bl_state; 
} 

static ssize_t dev_write(struct file *file, const char *buffer, 
                                            size_t count, loff_t * ppos) { 
    unsigned char ch; 
    int ret;
    
    if (count == 0) { 
        return count; 
    } 

    ret = copy_from_user(&ch, buffer, sizeof ch) ? -EFAULT : 0; 
    if (ret) { 
        return ret; 
    } 
    
    ch &= 0x01;
    set_bl(ch);
    
    return count; 
} 

static ssize_t dev_read(struct file *filp, char *buffer, 
                                        size_t count, loff_t *ppos) { 
    int ret; 
    unsigned char str[] = {'0', '1' }; 

    if (count == 0) { 
        return 0; 
    } 

    ret = copy_to_user(buffer, str + get_bl(),sizeof(unsigned char) ) ? -EFAULT : 0; 
    if (ret) { 
        return ret; 
    }
    
    return sizeof(unsigned char); 
} 

static struct file_operations dev_fops = { 
    owner: THIS_MODULE, 
    read:dev_read, 
    write: dev_write, 
};

static struct miscdevice misc = { 
    .minor = MISC_DYNAMIC_MINOR, 
    .name = DEVICE_NAME, 
    .fops = &dev_fops, 
}; 

static int __init dev_init(void) 
{ 
    int ret; 
    ret = misc_register(&misc); 

    printk(DEVICE_NAME"\tinitialized\n");
    s3c_gpio_cfgpin(S3C2410_GPG(4), S3C2410_GPIO_OUTPUT); 
    set_bl(1); 

    return ret;
} 
static void __exit dev_exit(void) 
{ 
    misc_deregister(&misc); 
}

module_init(dev_init);
module_exit(dev_exit);
MODULE_LICENSE("GPL"); 
MODULE_AUTHOR("FriendlyARM Inc.");

