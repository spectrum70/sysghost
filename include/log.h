#ifndef log_h
#define log_h

void sg_log(int level, const char *fmt, ...);
void msg(char *fmt, ...);
void err(const char *fmt, ...);

void log_skip(char *fmt, ...);
void log_step(char *fmt, ...);
void log_step_success();
void log_step_err();
void log_date_time();
void log_sysghost_start(char *version);

void dbg(char *fmt, ...);

extern char *__progname;

#endif /* log_h */
