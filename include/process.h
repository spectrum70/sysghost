#ifndef process_h
#define process_h

#define RUN_PID_PATH "/run/magic/"

int process_save_pid(const char *name, int pid);
int process_kill_by_name(const char *name);

#endif /* process_h */
