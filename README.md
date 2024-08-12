# Filesystem Project

## Make sure to only compile in a Linux/UNIX environment, as this program will not compile in Windows

## Functions

# make_fs(char* disk_name)

make_fs creates a virtual disk, initializes all necessary metadata and writes the metadata to the virtual disk.

# mount_fs(char* disk_name)

mount_fs reads metadata information back from virtual disk, and leaves the disk open, allowing for it to be changed.

# umount_fs(char* disk_name)

umount_fs writes back all metadata and data to the virtual disk, and closes the disk.

# fs_open(char* name)

fs_open assigns a file descriptor to an existing file, if there are any available. A maximum of 32 file descriptors can be open at once. A file must be opened before it can be modified.

# fs_close(int filedes)

fs_close removes a file descriptor from an existing file, meaning it will not be modifiable until the file is reopened.

# fs_create(char* name)

Creates a file if possible. There can be at most 64 files in the filesystem at one time.

# fs_delete(char* name)

Deletes a file from the filesystem.

# fs_read(int filedes, void* buf, size_t nbyte)

Reads nbyte bytes of data (buf) from an open file.

# fs_write(int filedes, void* buf, size_t nbyte)

Writes nbyte bytes of data (buf) to an open file.

# fs_get_filesize(int filedes)

Get the size of an open file.

# fs_lseek(int filedes, off_t offset)

Moves the position that fs_read and fs_write begin reading or writing from to a different position in an open file. By default, files will read from the very beginning, and write data to the beginning of the file. The starting position must be within the size of the file.

# fs_truncate(int filedes, off_t length)

Truncates the data in an open file at the specific length. The truncation value must be within the size of the file.

## Test Program

# An example program is provided and it does the following:

1. A virtual disk file is created and mounted.
2. A file is created and is given a file descriptor.
3. Data is written to the file, and the file descriptor is then removed.
4. The virtual disk file is unmounted.
5. A second process is opened using fork(), and the virtual disk file is mounted inside the child process.
6. The previously created file is given a file descriptor, and data is read from the file.
7. A new file is created and given a file descriptor.
8. The data from the original file in step 6 is then written to the new file.
9. The old file is closed and deleted.
10. The new file is closed, and the file system is unmounted.
11. The parent process waits for the child process (Steps 5-10) to complete, and execv() and dup2() are used to output the hexdump of the unmounted virtual disk file to a .txt file.
