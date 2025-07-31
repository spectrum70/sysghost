#ifndef __socket_h
#define __socket_h
int ux_server_create(char *name);
int ux_server_accept(int fd);
int ux_server_read_cmd(int fd, char *buff);
int ux_client_write(char *name, char *buff);
#endif /* __socket_h */
