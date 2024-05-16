#ifndef fs_h
#define fs_h

#define MAX_PATH	512

int fs_dir_exists(const char *path);
int fs_create_dir(const char *path);
int fs_create_file_write_int(char *path, int num);
int fs_file_read_int(const char *path, int *num);

#endif /* fs_h */
