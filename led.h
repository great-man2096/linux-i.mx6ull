#ifndef _LED_H
#define _LED_H

#include <linux/io.h>
#include <linux/cdev.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/uaccess.h> // 添加用户空间内存操作函数声明
#include <linux/string.h>
#include <linux/device.h>

#define LED_MAJOR 200
#define LED_NAME "led"

/* 定义led寄存器物理地址 */
#define CCM_CCGR1_BASE				(0X020C406C)
#define SW_MUX_GPIO1_IO03_BASE		(0X020E0068)
#define SW_PAD_GPIO1_IO03_BASE		(0X020E02F4)
#define GPIO1_DR_BASE				(0X0209C000)
#define GPIO1_GDIR_BASE				(0X0209C004)

/* 映射后的寄存器虚拟地址指针 */
static void __iomem *IMX6U_CCM_CCGR1;
static void __iomem *SW_MUX_GPIO1_IO03;
static void __iomem *SW_PAD_GPIO1_IO03;
static void __iomem *GPIO1_DR;
static void __iomem *GPIO1_GDIR;

/* LED设备结构体 */
struct newchrled_dev{
    dev_t devid; // 设备号
    struct cdev cdev; // 字符设备结构体
    struct class *class; // 类结构体
    struct device *device; // 设备结构体
    unsigned char major; // 主设备号
    unsigned char minor; // 次设备号
};

static struct newchrled_dev newchrled; // LED设备结构体实例

#endif