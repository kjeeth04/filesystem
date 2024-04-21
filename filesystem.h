#ifndef _FILESYS_H
#define _FILESYS_H_

int make_fs(char *disk_name); 

int mount_fs(char *disk_name);  

int umount_fs(char *disk_name); 

int fs_open(char* fileName);

int fs_close(int filedes);

int fs_create(char* fileName);

int fs_delete(char *fileName); 

int fs_read(int fildes, void *buf, size_t nbyte); 

int fs_write(int filedes, void *buf, size_t nbyte); 

int fs_get_filesize(int filedes); 

int fs_lseek(int filedes, off_t offset);

int fs_truncate(int filedes, off_t length);  

#endif