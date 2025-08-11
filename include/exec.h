#ifndef exec_h
#define exec_h
int exec_wait_exit(char *cmd_line);
int exec_nowait(char *cmd_line);
int exec_daemon(char *cmd_line);
int exec_script(char *name);
#endif /* exec_h */
