#ifndef log_h
#define log_h

#define MAGIC_STR "\x1b[1;32mm\x1b[1;34ma\x1b[1;33mg" \
		  "\x1b[1;36mi\x1b[1;31mc\x1b[0m\x1b[1m"

void log(int level, const char *fmt, ...);
void err(const char *fmt, ...);

void log_step(char *fmt, ...);
void log_step_err(char *fmt, ...);

#endif /* log_h */
