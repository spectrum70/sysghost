#ifndef halt_h
#define halt_h

int system_down(int reboot_system);
int spawn(int silent, char *prog, ...);

#endif /* halt_h */
