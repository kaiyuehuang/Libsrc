#ifndef SEND_RECV_H
#define SEND_RECV_H

extern void test_send_msg_p2pserver(void);

extern int check_usrbind_state(void);

extern int start_bind_camlist(char *camlist,char passwd);

extern int start_login_p2pserver(char *camlist,char *passwd);

extern int send_demo_tcpmsg(char *buf,int size);

extern void *handle_msg(void *arg);

extern int client_recv(void);

#endif
