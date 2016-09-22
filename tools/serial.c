#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <strings.h>
#include <termios.h>

/* ���ô��ڲ���:9600���� */
static int serial_init(int fd,int speed)
{    
	//�������ô��ڵĽṹ��
	struct termios termios_new;
	//����ոýṹ��
	bzero( &termios_new, sizeof(termios_new));	
	//	cfmakeraw()�����ն����ԣ���������termios�ṹ�еĸ���������
	cfmakeraw(&termios_new);
	//���ò�����
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
	//CLOCAL��CREAD�ֱ����ڱ������Ӻͽ���ʹ�ܣ���ˣ�����Ҫͨ��λ����ķ�ʽ����������ѡ�    
	termios_new.c_cflag |= CLOCAL | CREAD;
	//ͨ��������������λΪ8λ
	termios_new.c_cflag &= ~CSIZE;
	termios_new.c_cflag |= CS8; 
	//��������żУ��
	termios_new.c_cflag &= ~PARENB;
	//һλֹͣλ
	termios_new.c_cflag &= ~CSTOPB;
	tcflush(fd,TCIFLUSH);
	// �����ý����ַ��͵ȴ�ʱ�䣬������Ҫ����Խ�������Ϊ0
	termios_new.c_cc[VTIME] = 0;
	termios_new.c_cc[VMIN] = 0;
	// �����������/���������
	tcflush (fd, TCIFLUSH);
	//������ú󣬿���ʹ�����º������������
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