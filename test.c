#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <linux/limits.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h> 
#include "File System disk.h"
#include "structs.h"
#include "filesystem.h"

//Test Program
int main(int argc, char *argv[]){
    //Checks for valid number of inputs
    if (argc != 2){
        perror("filesys: Invalid number of arguments");
        exit(1);
    }

    //Initializing Virtual Disk File Name
    char* disk_name = argv[1];

    //Making File System
    if (make_fs(disk_name) == -1){
        perror("Could not make file system");
        exit(1);
    }

    //Mounting File System
    if (mount_fs(disk_name) == -1){
        perror("Could not mount file system");
        exit(1);
    }

    //Creating File
    char filename[7] = "myFile";
    if (fs_create(filename) == -1){
        perror("Could not Create File");
        exit(1);
    }

    //Opening File (Getting File Descriptor)
    int des = 0;
    if ((des = fs_open(filename)) == -1){
        perror("Could not give File Descriptor to file");
        exit(1);
    }

    //Writing Data to File
    char write[24] = "This data is not a Test";
    printf("This is what is being written to the File: %s\n", write);
    if (fs_write(des, write, strlen(write)) == -1){
        perror("Could not write to File");
        exit(1);
    }

    //Closing File
    if (fs_close(des) == -1){
        perror("Could not close File Descriptor");
        exit(1);
    }
    //Unmounting File System
    if (umount_fs(disk_name) == -1){
        perror("Could not unmount File System");
        exit(1);
    }

    //Forking
    int myfork = fork();
    if (myfork == -1){
        perror("Could not fork");
        exit(1);
    }
    
    //Child Process
    if (myfork == 0){
        //Mounting File System
        if (mount_fs(disk_name) == -1){
            perror("Could not mount file system");
            exit(1);
        }

        //Getting File Descriptor for file
        int des2 = 0;
        if ((des2 = fs_open(filename)) == -1){
            perror("Could not give File Descriptor to file");
            exit(1);
        }

        //Reading all contents from file
        char testbuf[24];
        if (fs_read(des2, testbuf, fs_get_filesize(des2)) == -1){
            perror("Could not read file");
            exit(1);
        }
        printf("This is what was read from the File: %s\n", testbuf);

        //Creating new File
        char filename2[7] = "myCopy";
        if (fs_create(filename2) == -1){
            perror("Could not Create File");
            exit(1);
        }

        //Opening new File (Getting File Descriptor)
        int des3 = 0;
        if ((des3 = fs_open(filename2)) == -1){
            perror("Could not give File Descriptor to file");
            exit(1);
        }

        //Writing Data from old file to new File
        if (fs_write(des3, testbuf, strlen(testbuf)) == -1){
            perror("Could not write to File");
            exit(1);
        }  
        char testbuf2[24];
        if (fs_read(des3, testbuf2, fs_get_filesize(des3)) == -1){
            perror("Could not read to File");
            exit(1);
        }
        printf("Data From Copied File: %s\n", testbuf2);

        //Close Old File
        if (fs_close(des2) == -1){
            perror("Could not close File");
            exit(1);
        }

        //Delete old File
        if (fs_delete(filename) == -1){
            perror("Could not delete File");
            exit(1);
        }

        //Close New File
        if (fs_close(des3) == -1){
            perror("Could not close File");
            exit(1);
        }

        //Unmounting File System
        if (umount_fs(disk_name) == -1){
            perror("Could not unmount File System");
            exit(1);
        }
        exit(0);
    }
    //Parent Process
    else {
        //Wait for Child Process to finish
        wait(NULL);

        //Outputing Hexdump to .txt file
        printf("Sending hexdump to 'hexdump.txt'\n");
        int hexdump = open("hexdump.txt", O_WRONLY | O_CREAT, 0777); 
        dup2(hexdump, 1); 
	    close(hexdump);
        char* args[] = {"/usr/bin/hexdump", "-C", disk_name};
        execv(args[0], args);

        //If this line is reached, execv did not work, and error message is outputed
        perror("Could not output hexdump to file");
        exit(1);
    }
}