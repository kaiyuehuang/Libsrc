#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <unistd.h>
extern "C"
{
#include "queWorkCond.h"
}

//��������
void parseMsg(const char *msg, int msgSize);

WorkQueue *qlist;
//�ඨ��
class msgHandle{    

    public:
        msgHandle(void);
        ~msgHandle(void);
        int inPutMsgQueue(const char *msg, int msgSize);   //write
		void registerWorkQueue(void handleMsg(const char *msg, int msgSize)); //parse
};

//���캯��
msgHandle::msgHandle(void)
{
	qlist = NULL;
}
//��������
msgHandle::~msgHandle(void)
{
	if(qlist != NULL)
    	CleanCondWorkPthread(qlist, parseMsg);
}

//ע��ص�����
void msgHandle::registerWorkQueue(void handleMsg(const char *msg, int msgSize))
{
    qlist = InitCondWorkPthread(handleMsg);
}

//���ݿ�����������ⲿ��Ϣ
int msgHandle::inPutMsgQueue(const char *msg, int msgSize)
{
   return putMsgQueue(qlist, msg, msgSize);
}

//�̶߳Դ˻ص���������msg���н���
void parseMsg( const char * msg, int msgSize)
{ 
 //  free_mempool(&recvhead, (queuelist *)msg);
 	printf("%s\n",msg);
 	free((char *)msg);
 	return;
}



//void DataBaseMsg_Task(void)
int main(void)
{
    msgHandle *pTask = new msgHandle();
    int i=0;
    char *msg=(char *)malloc(6); 
    pTask->registerWorkQueue(parseMsg);
    while(i<20)
    {
 		strcpy(msg,"hello");
 		snprintf(msg,9,"hello_%d",i);
   		pTask->inPutMsgQueue(msg,strlen(msg));
   		msg=(char *)malloc(10); 	
   		usleep(1);
   		i++;
    }
    sleep(1);  
    delete pTask;
	
}
