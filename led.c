/*
 *  chrdevbase.c - create a simple character device driver
 */

#include "led.h"

char write_buf[100];               // 读写缓冲区
u32 kerneldata = 0; // 内核数据

/* switch led status */
#define LED_ON 0
#define LED_OFF 1

#if 0
// void led_switch(u8 sta)
// {
// 	u32 val = 0;
// 	if(sta == LED_ON) {
// 		val = readl(GPIO1_DR);
// 		val &= ~(1 << 3);	
// 		writel(val, GPIO1_DR);
// 	}else if(sta == LED_OFF) {
// 		val = readl(GPIO1_DR);
// 		val |= (1 << 3);	
// 		writel(val, GPIO1_DR);
// 	}	
// }


/* 获取设备树属性实验 */
static int get_device_tree_property(void)
{
    struct device_node *node;
    struct property *prop;
    const char *str;
    int ret = 0;
    int bsize = 0;
    u32 *brival;

    node = of_find_node_by_path("/backlight");
    if (node == NULL) {
        printk(KERN_ERR "can't find /backlight\n");
        return -1;
    }
    prop = of_find_property(node, "compatible", NULL);
    if (prop == NULL) {
        printk(KERN_ERR "can't find property\n");
        return -1;
    }
    printk(KERN_INFO "%s\n", (char*)prop->value);
    ret = of_property_read_string(node, "status", &str);
    if (ret) {
        printk(KERN_ERR "can't read property\n");
        return -1;
    } else {
        printk(KERN_INFO "status = %s\n", str);
    }
    ret = of_property_read_u32(node, "default-brightness-level", &kerneldata);
    if (ret) {
        printk(KERN_ERR "can't read property\n");
        return -1;
    } else {
        printk(KERN_INFO "default-brightness-level = %d\n", kerneldata);
    }
    bsize = of_property_count_elems_of_size(node, "brightness-levels", sizeof(u32));
    if (bsize < 0) {
        printk(KERN_ERR "can't read property\n");
        return -1;
    } else {
        printk(KERN_INFO "brightness-levels count = %d\n", bsize);
    }
    /* 申请内存 */
    brival = kmalloc(bsize * sizeof(u32), GFP_KERNEL);
    if (brival == NULL) {
        printk(KERN_ERR "can't alloc memory\n");
        return -1;
    }
    /* 读取数组 */
    ret = of_property_read_u32_array(node, "brightness-levels", brival, bsize);
    if (ret) {
        printk(KERN_ERR "can't read property\n");
        kfree(brival);
        return -1;
    } else {
        int i;
        printk(KERN_INFO "brightness-levels = ");
        for (i = 0; i < bsize; i++) {
            printk(KERN_CONT "%d ", brival[i]);
        }
        printk(KERN_CONT "\n");
    }

    return 0;
}
#endif

/* 打开设备 */
static int led_open(struct inode *inode, struct file *filp)
{
    /* 用户实现具体功能 */
    printk(KERN_INFO "led_open\n");
    filp->private_data = &newchrled; // 将设备结构体指针赋值给文件私有数据
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
    if (kerneldata == 1)
    {
        kerneldata = LED_ON; // 如果数据为1，则点亮LED灯
    }
    else if (kerneldata == 0)
    {
        kerneldata = LED_OFF; // 如果数据为0，则熄灭LED灯
    }
    else
    {
        printk(KERN_ERR "invalid data\n");
        return -EINVAL; // 数据无效
    }
    gpio_set_value(newchrled.led_gpio, kerneldata); // 根据写入的数据控制LED灯

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
    // u32 val = 0;
    // const char *str;
    int ret = 0;
    // u32 regdata[10] = {0};

    // get_device_tree_property(); // 获取设备树属性实验
    
    printk(KERN_INFO "led_init\n");

    newchrled.major = 0;
    /* 6、注册字符设备驱动 */
    // retvalue = register_chrdev(LED_MAJOR, LED_NAME, &led_fops);
    if (newchrled.major) {
		newchrled.devid = MKDEV(newchrled.major, 0);
		retvalue = register_chrdev_region(newchrled.devid, 1, LED_NAME);
	} else {
		retvalue = alloc_chrdev_region(&newchrled.devid, 0, 1, LED_NAME);
		newchrled.major = MAJOR(newchrled.devid);
        newchrled.minor = MINOR(newchrled.devid);
	}
    
    if (retvalue < 0)
    {
        /* 字符设备注册失败 */
        printk(KERN_ERR "chrdevbase: can't register device %s\n", LED_NAME);
        return retvalue;
    }
    newchrled.cdev.owner = THIS_MODULE;
    cdev_init(&newchrled.cdev, &led_fops); // 初始化字符设备结构体变量
    cdev_add(&newchrled.cdev, newchrled.devid, 1); // 添加字符设备

    newchrled.class = class_create(THIS_MODULE, LED_NAME); // 创建类
    if (IS_ERR(newchrled.class))
    {
        /* 创建类失败 */
        printk(KERN_ERR "chrdevbase: can't create class %s\n", LED_NAME);
        return PTR_ERR(newchrled.class);
    }
    newchrled.device = device_create(newchrled.class, NULL, newchrled.devid, NULL, LED_NAME); // 创建设备节点
    if (IS_ERR(newchrled.device))
    {
        /* 创建设备节点失败 */
        printk(KERN_ERR "chrdevbase: can't create device %s\n", LED_NAME);
        return PTR_ERR(newchrled.device);
    }

    
#if 0
    newchrled.nd = of_find_node_by_path("/alphaled");
    if (newchrled.nd == NULL)
    {
        /* 设备树节点不存在 */
        printk(KERN_ERR "chrdevbase: can't find node %s\n", "/led");
        return -1;
    }
    ret = of_property_read_string(newchrled.nd, "compatible", &str);
    if (ret) {
        /* 读取设备树属性失败 */
        printk(KERN_ERR "chrdevbase: can't read property %s\n", "compatible");
        return ret;
    } else {
        printk(KERN_INFO "chrdevbase: compatible = %s\n", str);
    }
    // ret = of_property_read_u32_array(newchrled.nd, "reg", regdata, 10);
    // if (ret < 0) {
    //     /* 读取设备树属性失败 */
    //     printk(KERN_ERR "chrdevbase: can't read property %s\n", "reg");
    //     return ret;
    // } else {
    //     int i;
    //     printk(KERN_INFO "chrdevbase: reg = ");
    //     for (i = 0; i < 10; i++) {
    //         printk(KERN_CONT "%#X ", regdata[i]);
    //     }
    //     printk(KERN_CONT "\n");
    // }
    /* 1、初始化LED灯，这里做地址映射 */
    // IMX6U_CCM_CCGR1 = ioremap(regdata[0], regdata[1]);
    // SW_MUX_GPIO1_IO03 = ioremap(regdata[2], regdata[3]);
    // SW_PAD_GPIO1_IO03 = ioremap(regdata[4], regdata[5]);
    // GPIO1_DR = ioremap(regdata[6], regdata[7]);
    // GPIO1_GDIR = ioremap(regdata[8], regdata[9]);
    IMX6U_CCM_CCGR1 = of_iomap(newchrled.nd, 0);
    SW_MUX_GPIO1_IO03 = of_iomap(newchrled.nd, 1);
    SW_PAD_GPIO1_IO03 = of_iomap(newchrled.nd, 2);
    GPIO1_DR = of_iomap(newchrled.nd, 3);
    GPIO1_GDIR = of_iomap(newchrled.nd,4);
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
#endif

    newchrled.nd = of_find_node_by_path("/gpioled");
    if (newchrled.nd == NULL)
    {
        /* 设备树节点不存在 */
        printk(KERN_ERR "chrdevbase: can't find node %s\n", "/led");
        return -1;
    }

    //获取LED灯GPIO编号
    newchrled.led_gpio = of_get_named_gpio(newchrled.nd, "led-gpios", 0);
    if (newchrled.led_gpio < 0)
    {
        /* 读取设备树属性失败 */
        printk(KERN_ERR "chrdevbase: can't read property %s\n", "led-gpios");
        return newchrled.led_gpio;
    } else {
        printk(KERN_INFO "chrdevbase: led-gpios = %d\n", newchrled.led_gpio);
    }
    //申请IO
    ret = gpio_request(newchrled.led_gpio, "led-gpios");
    if (ret < 0)
    {
        /* 申请IO失败 */
        printk(KERN_ERR "chrdevbase: can't request gpio %d\n", newchrled.led_gpio);
        gpio_free(newchrled.led_gpio); // 释放GPIO
        return ret;
    }
    //设置IO方向输出
    ret = gpio_direction_output(newchrled.led_gpio, 1);
    if (ret < 0)
    {
        /* 设置IO方向失败 */
        printk(KERN_ERR "chrdevbase: can't set gpio %d direction\n", newchrled.led_gpio);
        return ret;
    }
    //设置IO电平低，开灯
    gpio_set_value(newchrled.led_gpio, LED_ON);

    return 0;
}

/* 驱动出口函数 */
static void __exit led_exit(void)
{
    // u32 val = 0;
    /* 关闭LED */
	// val = readl(GPIO1_DR);
	// val |= (1 << 3);	
	// writel(val, GPIO1_DR);
    gpio_set_value(newchrled.led_gpio, LED_OFF);
    /* 这里取消地址映射 */
    // iounmap(IMX6U_CCM_CCGR1);
    // iounmap(SW_MUX_GPIO1_IO03);
    // iounmap(SW_PAD_GPIO1_IO03);
    // iounmap(GPIO1_DR);
    // iounmap(GPIO1_GDIR);
    /* 注销字符设备驱动 */
    // unregister_chrdev(LED_MAJOR, LED_NAME);

    gpio_free(newchrled.led_gpio); // 释放GPIO
    device_destroy(newchrled.class, newchrled.devid); // 删除设备节点
    class_destroy(newchrled.class); // 销毁类

    cdev_del(&newchrled.cdev); // 删除字符设备
    unregister_chrdev_region(newchrled.devid, 1); // 注销字符设备
    

}

/*
    模块入口与出口
*/
module_init(led_init);
module_exit(led_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("shichang.yang");
