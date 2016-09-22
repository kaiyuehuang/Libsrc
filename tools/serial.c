#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <strings.h>
#include <termios.h>

/* 设置窗口参数:9600速率 */
static int serial_init(int fd,int speed)
{    
	//声明设置串口的结构体
	struct termios termios_new;
	//先清空该结构体
	bzero( &termios_new, sizeof(termios_new));	
	//	cfmakeraw()设置终端属性，就是设置termios结构中的各个参数。
	cfmakeraw(&termios_new);
	//设置波特率
	speed_t speedt =B0;
	switch(speed)
	{
		case 0:
			speedt =B0;
			break;	
		case 50:
			speedt =B50;
			break;
		case 75:
			speedt =B75;
			break;
        case 110:
			speedt =B110;
			break;
		case 134:
            speedt =B134;
			break;
		case 150:
            speedt =B150;
			break;
        case  200:
			speedt =B200;
			break;			
        case 300:
			speedt =B300;
			break;
		case 600:
           speedt = B600;
			break;
		case 1200:
            speedt =B1200;
			break;
		case 1800:
            speedt =B1800;
			break;
		case 2400:
            speedt =B2400;
			break;
		case 4800:
            speedt =B4800;
			break;
		case 9600:
            speedt =B9600;
			break;
		case 19200:
			speedt =B19200;
			break;
		case 38400:
            speedt =B38400;
			break;
        case 57600:
			speedt =B57600;
			break;
        case 115200:
			speedt =B115200;
			break;
		case 230400:
            speedt =B230400;
			break;
	}
	cfsetispeed(&termios_new, speedt);
	cfsetospeed(&termios_new, speedt);
	//CLOCAL和CREAD分别用于本地连接和接受使能，因此，首先要通过位掩码的方式激活这两个选项。    
	termios_new.c_cflag |= CLOCAL | CREAD;
	//通过掩码设置数据位为8位
	termios_new.c_cflag &= ~CSIZE;
	termios_new.c_cflag |= CS8; 
	//设置无奇偶校验
	termios_new.c_cflag &= ~PARENB;
	//一位停止位
	termios_new.c_cflag &= ~CSTOPB;
	tcflush(fd,TCIFLUSH);
	// 可设置接收字符和等待时间，无特殊要求可以将其设置为0
	termios_new.c_cc[VTIME] = 0;
	termios_new.c_cc[VMIN] = 0;
	// 用于清空输入/输出缓冲区
	tcflush (fd, TCIFLUSH);
	//完成配置后，可以使用以下函数激活串口设置
	if(tcsetattr(fd,TCSANOW,&termios_new) )
	{
		printf("Setting the serial1 failed!\n");
		return -1;
	}
	return 0;
}

int serial_open(char *serial,int speed)
{
	int serial_fd = open(serial,O_RDWR|O_NOCTTY|O_NONBLOCK);
	if(serial_fd<0)
	{
		perror("open failed ");
		return -1;
	}
	if(serial_init(serial_fd,speed)<0)
	{
		close(serial_fd);
		return -1;
	}
	return serial_fd;
}