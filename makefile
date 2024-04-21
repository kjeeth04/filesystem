filesys: test.c filesystem.c file_system_disk.c
	gcc -o filesys test.c filesystem.c file_system_disk.c -Wall -Werror 