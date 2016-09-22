#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>
#include <netdb.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pthread.h>
#include <sys/time.h>

//保持相应头信息
struct resp_header{    
	int status_code;//HTTP/1.1 '200' OK    
	char content_type[128];//Content-Type: application/gzip    
	long content_length;//Content-Length: 11683079    
	char file_name[256];
};
static struct resp_header resp;//全剧变量以便在多个进程中使用

static void parse_url(const char *url, char *domain, int *port, char *file_name)
{	 
	/*通过url解析出域名, 端口, 以及文件名*/	
	int j = 0;	  
	int start = 0;
	*port = 80;
	char *patterns[] = {"http://", "https://", NULL};
	int i;
	for(i = 0; patterns[i]; i++)
		if (strncmp(url, patterns[i], strlen(patterns[i])) == 0)
			start = strlen(patterns[i]);
	//解析域名, 这里处理时域名后面的端口号会保留
	for( i = start; url[i] != '/' && url[i] != '\0'; i++, j++)
		domain[j] = url[i];
	domain[j] = '\0';
	//解析端口号, 如果没有, 那么设置端口为80
	char *pos = strstr(domain, ":");
    if (pos)        
		sscanf(pos, ":%d", port);
	//删除域名端口号
	for( i = 0; i < (int)strlen(domain); i++)
    {        
    	if (domain[i] == ':')        
		{            
			domain[i] = '\0';
			break;
		}
	}
    //获取下载文件名    
    j = 0;    
	for ( i = start; url[i] != '\0'; i++)
	{        
		if (url[i] == '/')        
		{            
			if (i !=  strlen(url) - 1)                
				j = 0;            
				continue;        
		}        
		else            
			file_name[j++] = url[i];    
	}    
		file_name[j] = '\0';
}

static struct resp_header get_resp_header(const char *response)
{    
	/*获取响应头的信息*/    
	struct resp_header resp;    
	char *pos = strstr(response, "HTTP/");    
	if (pos)        
		sscanf(pos, "%*s %d", &resp.status_code);//返回状态码    
		pos = strstr(response, "Content-Type:");//返回内容类型    
		if (pos)        
			sscanf(pos, "%*s %s", resp.content_type);    
		pos = strstr(response, "Content-Length:");//内容的长度(字节)    
		if (pos)        
			sscanf(pos, "%*s %ld", &resp.content_length);    
		return resp;
}
static void get_ip_addr(char *domain, char *ip_addr)
{    
	/*通过域名得到相应的ip地址*/    
	struct hostent *host = gethostbyname(domain);    
	int i;	    
	if (!host)    
	{        
		ip_addr = NULL;        
		return;    
	}    
	for (i = 0; host->h_addr_list[i]; i++)    
	{        
		strcpy(ip_addr, inet_ntoa( * (struct in_addr*) host->h_addr_list[i]));        
		break;    
	}
}
static void progressBar(long cur_size, long total_size)
{    
	/*用于显示下载进度条*/    
	float percent = (float) 
	cur_size / total_size;    
	const int numTotal = 50;    
	int numShow = (int)(numTotal * percent);    
	if (numShow == 0)        
		numShow = 1;    
	if (numShow > numTotal)        
		numShow = numTotal;    
	char sign[51] = {0};    
	memset(sign, '=', numTotal);    
	printf("\r%.2f%%\t[%-*.*s] %.2f/%.2fMB", percent * 100, numTotal, numShow, sign, cur_size / 1024.0 / 1024.0, total_size / 1024.0 / 1024.0);    fflush(stdout);    
	if (numShow == numTotal)        
		printf("\n");
}

int DownhttpFile(const char *url)
{
	char domain[64] = {0};	  
	char ip_addr[16] = {0};    
	int port = 80;	  
	char file_name[256] = {0};	  
 	int ret=0;
	
	printf("1: Parsing url...\n");	  
	parse_url(url, domain, &port, file_name);	   
	printf("2: Get ip address...\n");	 
	get_ip_addr(domain, ip_addr);	 
	if (strlen(ip_addr) == 0)	 
	{		 
		printf("can not get ip address\n"); 	   
		return -1;	  
	}	 
	printf("\n>>>>Detail<<<<");	 
	printf("URL: %s\n", url);	 
	printf("DOMAIN: %s\n", domain);    
	printf("IP: %s\n", ip_addr);	
	printf("PORT: %d\n", port);    
	printf("FILENAME: %s\n\n", file_name);	  
	//设置http请求头信息	
	char header[1024] = {0};	
	sprintf(header,"GET %s HTTP/1.1\r\n"\
		"Accept:text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,*/*;q=0.8\r\n"\			
		"User-Agent:Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537(KHTML, like Gecko) Chrome/47.0.2526Safari/537.36\r\n"\			  
		"Host:%s\r\n"\			  
		"Connection:close\r\n"\ 		   
		"\r\n"\ 	   
		,url, domain);	  
	//printf("%s\n%d", header, (int) strlen(header));	 
	//创建套接字	
	int client_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);	  
	if (client_socket < 0)	  
	{		 
		printf("invalid socket descriptor: %d\n", client_socket);		 
		return -1;
	}	 
	//创建地址结构体	 
	struct sockaddr_in addr;	
	memset(&addr, 0, sizeof(addr));    
	addr.sin_family = AF_INET;	  
	addr.sin_addr.s_addr = inet_addr(ip_addr);	  
	addr.sin_port = htons(port);	//连接服务器	
	printf("3: Connect server...\n");	 
	int res = connect(client_socket, (struct sockaddr *) &addr, sizeof(addr));
	if (res == -1)	  
	{		 
		printf("connect failed, return: %d\n", res);		
		return -1;	 
	}	 
	printf("4: Send request...\n");
	//向服务器发送下载请求	  
	write(client_socket, header, strlen(header));	 
	int mem_size = 4096;	
	int length = 0;    
	int len;	
	char *buf = (char *) malloc(mem_size * sizeof(char));	 
	char *response = (char *) malloc(mem_size * sizeof(char));	  
	char * temp =NULL;
	//每次单个字符读取响应头信息, 仅仅读取的是响应部分的头部, 后面单独开线程下载	
	while ((len = read(client_socket, buf, 1)) != 0)	
	{		 
		if (length + len > mem_size)		
		{			 
			//动态内存申请, 因为无法确定响应头内容长度			  
			mem_size *= 2;			  
			temp= (char *) realloc(response, sizeof(char) * mem_size);	
			if (temp == NULL)			 
			{				 
				printf("realloc failed\n"); 
				ret=-1;
				goto exit0;			 
			}			 
			response = temp;		
		}		 
		buf[len] = '\0';		
		strcat(response, buf);		  
		//找到响应头的头部信息, 两个"\n\r"为分割点		  
		int flag = 0;	
		int i;			
		for (i = strlen(response) - 1; response[i] == '\n' || response[i] == '\r'; i--, flag++);
		if (flag == 4)			  
			break;		  
		length += len;	  
	}	 
	//printf("\n>>>>Response header:<<<<\n%s", response);	 
	resp = get_resp_header(response);	 
	strcpy(resp.file_name, file_name);	

	
	printf("5: Start thread to download...\n");    
	int buf_len = mem_size;//read 4k each time    
	int fd = open(resp.file_name, O_CREAT | O_WRONLY, S_IRWXG | S_IRWXO | S_IRWXU);    
	if (fd < 0)    
	{        
		printf("Create file failed\n");   
		ret=-1;
		goto exit0;  
	}    
	length = 0;    
	while ((len = read(client_socket, buf, buf_len)) != 0 && length < resp.content_length)    
	{        
		write(fd, buf, len);        
		length += len;        
		//printf("len = %d\n",len);
		//progressBar(length, resp.content_length);    
	}    
	if (length == resp.content_length)        
		printf("Download successful ^_^\n\n");

exit1:
	close(fd);
exit0:
	free(buf);
	free(response);
	return ret;
}

#if 0
int main(int argc, char  *argv[])
{    
	if (argc == 1)    
	{        
		printf("Input a valid URL please\n");        
		exit(0);    
	} 
	DownhttpFile((const char *)argv[1]);
}
#endif
