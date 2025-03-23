#ifndef log_h
#define log_h

#define SYSGHOST_STR "\x1b[95msysghost\x1b[0m"

void log(int level, const char *fmt, ...);
void err(const char *fmt, ...);

void log_step(char *fmt, ...);
void log_step_err(char *fmt, ...);

#endif /* log_h */
