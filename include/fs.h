#ifndef fs_h
#define fs_h

#define MAX_PATH	512
#define MAX_LINE	1024

int fs_file_dir_exists(const char *path);
int fs_create_dir(const char *path, int mode);
int fs_create_file_write_int(char *path, int num);
int fs_create_file_write_str(char *path, char *str);
int fs_file_read_int(const char *path, int *num);
int fs_file_read_str(const char *path, char *str);

#endif /* fs_h */
