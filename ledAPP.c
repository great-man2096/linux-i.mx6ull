#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* * chrdevbaseAPP.c
 * 
 * 这是一个简单的字符设备驱动程序的用户空间应用程序示例。
 * 它打开字符设备，向设备写入数据，然后从设备读取数据并打印到控制台。
 * argc - 参数个数
 * argv - 参数列表
 * ./ledAPP <file_name> <0,1>
 * 1 - 读取数据
 * 2 - 写入数据
 */
int main(int argc, char *argv[])
{
    int ret  = 0; // 返回值
    int fd = 0; // 文件描述符
    unsigned char opration[100]; // 操作类型
    char *file_name; // 设备文件名

    if(argc < 3) {
        printf("Usage: %s <file_name> <0,1>\n", argv[0]);
        return -1;
    }

    file_name = argv[1]; // 获取设备文件名
    opration[0] = atoi(argv[2]); // 获取操作类型

    fd = open(file_name, O_RDWR); // 打开设备文件
    if (fd < 0) {
        printf("cannot open %s\n", file_name);
        return -1;
    }

    /* write */
    ret = write(fd, opration, sizeof(opration)); // 向设备写入数据
    if (ret < 0) {
        printf("write file %s failed!\n", file_name);
        close(fd);
        return -1;
    }
    
    /* close */
    ret = close(fd); // 关闭设备文件
    if (ret < 0) {
        printf("close file %s failed!\n", file_name);
        return -1;
    }

    return 0;
    
}
