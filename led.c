/*
 *  chrdevbase.c - create a simple character device driver
 */
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/uaccess.h> // 添加用户空间内存操作函数声明
#include <linux/string.h>
#include "led.h"
#define LED_MAJOR 200
#define LED_NAME "led"

char write_buf[100];               // 读写缓冲区
u8 kerneldata = 0; // 内核数据

/* switch led status */
#define LED_ON 1
#define LED_OFF 0

void led_switch(u8 sta)
{
	u32 val = 0;
	if(sta == LED_ON) {
		val = readl(GPIO1_DR);
		val &= ~(1 << 3);	
		writel(val, GPIO1_DR);
	}else if(sta == LED_OFF) {
		val = readl(GPIO1_DR);
		val |= (1 << 3);	
		writel(val, GPIO1_DR);
	}	
}


/* 打开设备 */
static int led_open(struct inode *inode, struct file *filp)
{
    /* 用户实现具体功能 */
    // printk(KERN_INFO "led_open\n");
    return 0;
}

/* 向设备写数据 */
static ssize_t led_write(struct file *filp, const char __user *buf, size_t cnt, loff_t *offt)
{
    /* 用户实现具体功能 */
    // printk(KERN_INFO "led_write\n");
    if (copy_from_user(write_buf, buf, cnt))
    { // 从用户空间复制到写缓冲区
        printk(KERN_ERR "copy_from_user failed\n");
        return -EFAULT; // 复制失败
    }
    kerneldata = write_buf[0]; // 将写缓冲区数据复制到内核数据
    printk(KERN_INFO "kerneldata = %d\n", kerneldata);
    led_switch(kerneldata); // 根据写入的数据控制LED灯

    return 0;
}
/* 关闭/释放设备 */
static int led_release(struct inode *inode, struct file *filp)
{
    /* 用户实现具体功能 */
    // printk(KERN_INFO "led_release\n");
    return 0;
}

static struct file_operations led_fops = {
    .owner = THIS_MODULE,
    .open = led_open,
    .write = led_write,
    .release = led_release,
};

/* 驱动入口函数 */
static int __init led_init(void)
{
    int retvalue = 0;
    u32 val = 0;
    printk(KERN_INFO "led_init\n");
    /* 1、初始化LED灯，这里做地址映射 */
    IMX6U_CCM_CCGR1 = ioremap(CCM_CCGR1_BASE, 4);
    SW_MUX_GPIO1_IO03 = ioremap(SW_MUX_GPIO1_IO03_BASE, 4);
    SW_PAD_GPIO1_IO03 = ioremap(SW_PAD_GPIO1_IO03_BASE, 4);
    GPIO1_DR = ioremap(GPIO1_DR_BASE, 4);
    GPIO1_GDIR = ioremap(GPIO1_GDIR_BASE, 4);
    /* 2、使能GPIO1时钟 */
	val = readl(IMX6U_CCM_CCGR1);
	val &= ~(3 << 26);	/* 清除以前的设置 */
	val |= (3 << 26);	/* bit26,27置1 */
	writel(val, IMX6U_CCM_CCGR1);
    /* 3、设置GPIO1_IO03的复用功能，将其复用为
	 *    GPIO1_IO03，最后设置IO属性。
	 */
    writel(5, SW_MUX_GPIO1_IO03);
    /*寄存器SW_PAD_GPIO1_IO03设置IO属性
	 *bit 16:0 HYS关闭
	 *bit [15:14]: 00 默认下拉
     *bit [13]: 0 kepper功能
     *bit [12]: 1 pull/keeper使能
     *bit [11]: 0 关闭开路输出
     *bit [7:6]: 10 速度100Mhz
     *bit [5:3]: 110 R0/6驱动能力
     *bit [0]: 0 低转换率
	 */
    writel(0x10B0, SW_PAD_GPIO1_IO03); // pull up, 100K, 50mA
    /* 4、设置GPIO1_IO03为输出功能 */
	val = readl(GPIO1_GDIR);
	val &= ~(1 << 3);	/* 清除以前的设置 */
	val |= (1 << 3);	/* 设置为输出 */
	writel(val, GPIO1_GDIR);
    /* 5、默认打开LED */
	val = readl(GPIO1_DR);
	val &= ~(1 << 3);	
	writel(val, GPIO1_DR);

    /* 6、注册字符设备驱动 */
    retvalue = register_chrdev(LED_MAJOR, LED_NAME, &led_fops);
    if (retvalue < 0)
    {
        /* 字符设备注册失败 */
        printk(KERN_ERR "chrdevbase: can't register device %s\n", LED_NAME);
        return retvalue;
    }

    return 0;
}

/* 驱动出口函数 */
static void __exit led_exit(void)
{
    u32 val = 0;
    /* 关闭LED */
	val = readl(GPIO1_DR);
	val |= (1 << 3);	
	writel(val, GPIO1_DR);
    /* 这里取消地址映射 */
    iounmap(IMX6U_CCM_CCGR1);
    iounmap(SW_MUX_GPIO1_IO03);
    iounmap(SW_PAD_GPIO1_IO03);
    iounmap(GPIO1_DR);
    iounmap(GPIO1_GDIR);
    /* 注销字符设备驱动 */
    unregister_chrdev(LED_MAJOR, LED_NAME);
}

/*
    模块入口与出口
*/
module_init(led_init);
module_exit(led_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("shichang.yang");
