#ifndef halt_h
#define halt_h

int system_down(int reboot_system);
int spawn(int silent, char *prog, ...);
void close_all_streams(bool redirect_tty);
void finalize_reboot(int type);
#endif /* halt_h */
