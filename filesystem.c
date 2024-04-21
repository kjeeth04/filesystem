#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <linux/limits.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h> 
#include <time.h>
#include "File System disk.h"
#include "structs.h"
#include "filesystem.h"

//Boot Block, File Allocation Table, Root Directory, Data Space, and File Description Table Objects for Filesystem
boot myboot;
fat FAT1;
fat FAT2;
root myroot;
data mydata;
file_des_table FDT;

/*  Creating File System Disk and initializing necessary metadata
    @param disk_name : name of the virtual disk
    @return -1 if error, 0 if no errors
*/
int make_fs(char *disk_name){

    //Creating virtual disk that file system will exist within
    if (make_disk(disk_name) == -1){
        fprintf(stderr, "make_fs: Could not create Filesystem\n");
        return -1;
    }

    //Opening virtual disk to write necessary metadata
    if (open_disk(disk_name) == -1){
        fprintf(stderr, "make_fs: Could not open Filesystem\n");
        return -1;
    }

    //Initializing Boot
    myboot.block_count = DISK_BLOCKS;
    myboot.boot_size = (sizeof(boot)+BLOCK_SIZE-1)/BLOCK_SIZE;
    myboot.boot_start = 0;
    myboot.free_loc = BLOCK_SIZE;

    //Forces FAT1 to begin after Boot Block in storage, then FAT2 after FAT1
    myboot.fat1_start = myboot.boot_size;
    myboot.fat_size = (sizeof(fat)+BLOCK_SIZE-1)/BLOCK_SIZE;
    myboot.fat2_start = myboot.fat1_start + myboot.fat_size;

    //Forces Root Directory to begin after FAT2
    myboot.root_start = myboot.fat2_start + myboot.fat_size;
    myboot.root_size = (sizeof(root)+BLOCK_SIZE-1)/BLOCK_SIZE;
    
    //Forces Data Space to begin at Block 4096
    myboot.data_start = BLOCK_SIZE;
    myboot.data_size = DISK_BLOCKS - myboot.data_start;

    //Initializing FAT
    for (int i = 0; i < BLOCK_SIZE; ++i){
        FAT1.file_ptrs[i] = -2;
        FAT2.file_ptrs[i] = -2;
    }

    //Initializing Root Directory
    myroot.entry_count = 0;
    for (int i = 0; i < 64; ++i){
        myroot.dir[i].active = 0;
        strcpy(myroot.dir[i].name, "");
        myroot.dir[i].size = 1;
        myroot.dir[i].start_block = -2; 
        myroot.dir[i].filedes = -1;
        myroot.dir[i].offset = 0;
        strcpy(myroot.dir[i].lastaccessed, "");
        strcpy(myroot.dir[i].moddate, "");
        strcpy(myroot.dir[i].modtime, "");
    }

    //Initializing Data Space
    mydata.free_data = BLOCK_SIZE;
    for (int i = 0; i < BLOCK_SIZE; ++i){
        mydata.data_space[i].free_blocks = BLOCK_SIZE;
        for (int j = 0; j < BLOCK_SIZE; ++j){
            mydata.data_space[i].space[j] = -2;
        }
    }

    //Initializing FDT
    FDT.active_count = 0;
    for (int i = 0; i < 32; ++i){
        FDT.table[i].isActive = 0;
    }

    //Write Boot, FAT, Root, and Data Space to Virtual Disk
    if (block_write(myboot.boot_start, (char*)&myboot) == -1 ||
        block_write(myboot.fat1_start, (char*)&FAT1) == -1 ||
        block_write(myboot.fat2_start, (char*)&FAT2) == -1 ||
        block_write(myboot.root_start, (char*)&myroot) == -1){
        fprintf(stderr, "make_fs: Could not write to Filesystem\n");
        return -1;
    }
    //Write Data Space
    for (int i = 0; i < BLOCK_SIZE; ++i){
        if(block_write(myboot.data_start+i, (char*)&mydata.data_space[i]) == -1){
            fprintf(stderr, "make_fs: Could not write to Filesystem\n");
            return -1;
        }
    }


    //Close Virtual Disk
    if (close_disk(disk_name) == -1){
        fprintf(stderr, "make_fs: Could not close Filesystem\n");
        return -1;
    }

    return 0;
}

/*  Mounts Filesystem, making it ready for use
    @param disk_name : name of the virtual disk
    @return -1 if error, 0 if no errors
*/
int mount_fs(char *disk_name){
    //Opening virtual disk to mount it
    if (open_disk(disk_name) == -1){
        fprintf(stderr, "mount_fs: Could not open Filesystem\n");
        return -1;
    }

    //Reading Boot, FAT, Root Directory, and Data Space Data
    if (block_read(myboot.boot_start, (char*)&myboot) == -1 ||
        block_read(myboot.fat1_start, (char*)&FAT1) == -1 ||
        block_read(myboot.fat2_start, (char*)&FAT2) == -1 ||
        block_read(myboot.root_start, (char*)&myroot) == -1){
        fprintf(stderr, "make_fs: Could not write to Filesystem\n");
        return -1;
    }

    //Read Data Space
    for (int i = 0; i < BLOCK_SIZE; ++i){
        if(block_read(myboot.data_start+i, (char*)&mydata.data_space[i]) == -1){
            fprintf(stderr, "make_fs: Could not write to Filesystem\n");
            return -1;
        }
    }
    
    return 0;
 }

/*  Unmounts Filesystem, writing data back to the Virtual Disk
    @param disk_name : name of the virtual disk
    @return -1 if error, 0 if no errors
*/
 int umount_fs(char *disk_name){
    //Write back Metadata to Disk
    if (block_write(myboot.boot_start, (char*)&myboot) == -1 ||
        block_write(myboot.fat1_start, (char*)&FAT1) == -1 ||
        block_write(myboot.fat2_start, (char*)&FAT2) == -1 ||
        block_write(myboot.root_start, (char*)&myroot) == -1){
        fprintf(stderr, "umount_fs: Could not write to Filesystem\n");
        return -1;
    }
    
    //Write Data Space
    for (int i = 0; i < BLOCK_SIZE; ++i){
        if(block_write(myboot.data_start+i, (char*)&mydata.data_space[i]) == -1){
            fprintf(stderr, "umount_fs: Could not write to Filesystem\n");
            return -1;
        }
    }

    //Close Virtual Disk
    if (close_disk(disk_name) == -1){
        fprintf(stderr, "unmount_fs: Could not close Filesystem\n");
        return -1;
    }

    return 0;
}

/*  Gives File Descriptor to a File, if any are available
    @param fileName : name of file to be granted a File Descriptor
    @return -1 if error, or File Descriptor on success
*/
int fs_open(char* fileName){
    //Checks to see if there are any open file descriptors
    if (FDT.active_count == 32){
        fprintf(stderr, "fs_open: No File Descriptors Available\n");
        return -1;
    }

    //Checks to see if File Exists, and then assigns it a File Descriptor
    for (int i = 0; i < 64; ++i){
        if (strcmp(myroot.dir[i].name, fileName) == 0){
            //Looks for and returns first available File Descriptor
            for (int i = 0; i < 32; ++i){
                if (!(FDT.table[i].isActive)){
                    FDT.table[i].isActive = 1;
                    FDT.active_count += 1;
                    myroot.dir[i].filedes = i;
                    return i;
                }
            }
        }
    }

    //Error Message if File does not exist
    fprintf(stderr, "fs_open: File does not exist\n");
    return -1;
}

/*  Closes an opened File Descriptor
    @param filedes : File Descriptor to be closed
    @return -1 if error, or 1 on success
*/
int fs_close(int filedes){
    //Checks to see if filedes is a valid number
    if (filedes < 0 || filedes > 31){
        fprintf(stderr, "fs_close: Invalid File Descriptor\n");
        return -1;
    }

     //Getting File attached to File Descriptor, and removing File Descriptor
    for (int i = 0; i < 64; ++i){
        if (myroot.dir[i].filedes == filedes){
            myroot.dir[i].filedes = -1;
            break;
        }
    }
    //Checks to see if filedes is already open, and closes it if so
    if (FDT.table[filedes].isActive){
        FDT.table[filedes].isActive = 0;
        FDT.active_count -= 1;
        return 1;
    }

    //Error message if filedes is already closed
    fprintf(stderr, "fs_close: File Descriptor already closed\n");
    return -1;
}

/*  Creates a new File, if conditions allow for a File to be created
    @param fileName : name of File to be created
    @return -1 if error, or 1 on success
*/
int fs_create(char* fileName){
    //Checks to see if fileName is too long or too short
    if (strlen(fileName) > 14 || strlen(fileName) <1){
        fprintf(stderr, "fs_create: File Name is not valid\n");
        return -1;
    }
    //Checks to see if there is any space in the Root for the File System
    if (myroot.entry_count == 64){
        fprintf(stderr, "fs_create: File System is full\n");
        return -1;
    }
    //Checks to see if File Name is already in use
    for (int i = 0; i < 64; ++i){
        if (strcmp(fileName, myroot.dir[i].name) == 0){
            fprintf(stderr, "fs_create: File Name alreadu in Use\n");
            return -1;
        }
    }

    //Creating New File
    for (int i = 0; i < 64; ++i){
        //Finding first open space for File, then creating File Data
        if (!(myroot.dir[i].active)){
            //Switching File to active, incrementing the entry counter, and setting the File Name
            myroot.entry_count += 1;
            myroot.dir[i].active = 1;
            strcpy(myroot.dir[i].name, fileName);

            //Setting Creation Time
            time_t tempTime = time(NULL);
            struct tm time1 = *localtime(&tempTime);
            snprintf(myroot.dir[i].lastaccessed, 16, "%d-%d-%d", time1.tm_year+1900, time1.tm_mon+1, time1.tm_mday);
            snprintf(myroot.dir[i].moddate, 16, "%d-%d-%d", time1.tm_year+1900, time1.tm_mon+1, time1.tm_mday);
            snprintf(myroot.dir[i].modtime, 16, "%d:%d:%d", time1.tm_hour, time1.tm_min, time1.tm_sec);
            //Finding First available place to start the block
            for (int j = 0; j < BLOCK_SIZE; ++j){
                if (mydata.data_space[j].free_blocks == BLOCK_SIZE){
                    //Setting the Starting Block
                    myroot.dir[i].start_block = j;
                    //Setting the next block pointer to EOC (-1) in the FATs at j
                    FAT1.file_ptrs[j] = -1;
                    FAT2.file_ptrs[j] = -1;
                    //Decrementing count of free blocks
                    mydata.free_data -= 1;
                    return 1;
                }
            }

            //Error Message if No Space
            fprintf(stderr, "fs_create: No Data Space Left\n");
            return -1;
        }
    }
    return -1;
}

/*  Deletes File, as long as it does not have an open File Descriptor
    @param fileName : Name of File to be deleted
    @return -1 if error, or 1 on success
*/
 int fs_delete(char *fileName){
    //Find File based on its name
    for (int i = 0; i < 64; ++i){
        if (strcmp(myroot.dir[i].name, fileName) == 0){
            //Checks to see if File has an associated open file descriptor
            if (myroot.dir[i].filedes != -1){
                fprintf(stderr, "fs_close: File currently has an open file descriptor\n");
                return -1;
            }
            //Decrements entry counter, sets File as inactive, sets size to 0, and resets name and time data
            myroot.entry_count -= 1;
            myroot.dir[i].active = 0;
            myroot.dir[i].size = 0;
            memset(myroot.dir[i].name, 0, 15);
            memset(myroot.dir[i].lastaccessed, 0, 16);
            memset(myroot.dir[i].moddate, 0, 16);
            memset(myroot.dir[i].modtime, 0, 16);

            //Resetting starting block value
            int block_ptr = myroot.dir[i].start_block;
            myroot.dir[i].start_block = -2;

            //Unreserving all allocated Blocks
            while (block_ptr != -2){
                int temp = block_ptr;
                block_ptr = FAT1.file_ptrs[block_ptr];
                //Setting Block to -2 (Neutral)
                FAT1.file_ptrs[temp] = -2;
                FAT2.file_ptrs[temp] = -2;
                //Incrementing amount of free data blocks, and restoring free space left in block to full 
                mydata.free_data += 1;
                mydata.data_space[temp].free_blocks = BLOCK_SIZE;
            }
            return 1;
        }
    }
    //Error message if File does not exist
    fprintf(stderr, "fs_close: File does not exist\n");
    return -1;
 }

/*  Reads from File, if possible
    @param fildes : File Descriptor of File to be read from
    @param buf : Buffer to read data into
    @param nbyte : Number of bytes to read
    @return -1 if error, or number of bytes actually read on success
*/
int fs_read(int filedes, void *buf, size_t nbyte){
    //Validate File Descriptor
    if(filedes < 0 || filedes > 31){
        fprintf(stderr, "fs_read: File Descriptor does not exist\n");
        return -1;
    }
    if (!(FDT.table[filedes].isActive)){
        fprintf(stderr, "fs_read: File Descriptor is not open\n");
        return -1;
    }

    //Getting File attached to File Descriptor
    for (int i = 0; i < 64; ++i){
        if (myroot.dir[i].filedes == filedes){
            //Updated Access Time
            time_t tempTime = time(NULL);
            struct tm time1 = *localtime(&tempTime);
            snprintf(myroot.dir[i].lastaccessed, 16, "%d-%d-%d", time1.tm_year+1900, time1.tm_mon+1, time1.tm_mday);

            //Read all the contents of the file, and then take only what is needed, truncating the end if necessary based on nbyte value, and
            //truncating the beginning based on offset value
            char* temp = (char*) malloc(myroot.dir[i].size+BLOCK_SIZE);
            int block = myroot.dir[i].start_block;
            int left_to_read = myroot.dir[i].size;
            while (block != -1 && block != -2 && left_to_read != 0 && myroot.dir[i].size != 1){
                //Copy Data over to temp
                if (left_to_read >= BLOCK_SIZE){
                    strcat(temp, mydata.data_space[block].space);
                    left_to_read -= BLOCK_SIZE;
                }
                else {
                    strncat(temp, mydata.data_space[block].space, left_to_read);
                    left_to_read = 0;
                }
                //Moving block to the next pointer per the FAT
                block = FAT1.file_ptrs[block];
            }
                //Copying the contents of the file contents based on the offset
                strncpy((char*)buf, (temp + myroot.dir[i].offset), nbyte);
                temp = 0; free(temp);
                return strlen(buf);
            }
        }
    return -1;
}

/*  Writes to a file, if possible
    @param fildes : File Descriptor of File to write to
    @param buf : Buffer to write data from
    @param nbyte : Number of bytes to write
    @return -1 if error, or number of bytes actually written on success
*/
int fs_write(int filedes, void *buf, size_t nbyte){
    //Validate File Descriptor
    if(filedes < 0 || filedes > 31){
        fprintf(stderr, "fs_write: File Descriptor does not exist\n");
        return -1;
    }
    if (!(FDT.table[filedes].isActive)){
        fprintf(stderr, "fs_write: File Descriptor is not open\n");
        return -1;
    }
    //Getting File attached to File Descriptor
    for (int i = 0; i < 64; ++i){
        if (myroot.dir[i].filedes == filedes){
            //Updating Access and Modification Times
            time_t tempTime = time(NULL);
            struct tm time1 = *localtime(&tempTime);
            snprintf(myroot.dir[i].lastaccessed, 16, "%d-%d-%d", time1.tm_year+1900, time1.tm_mon+1, time1.tm_mday);
            snprintf(myroot.dir[i].moddate, 16, "%d-%d-%d", time1.tm_year+1900, time1.tm_mon+1, time1.tm_mday);
            snprintf(myroot.dir[i].modtime, 16, "%d:%d:%d", time1.tm_hour, time1.tm_min, time1.tm_sec);

            //Find out the number of blocks needed for writing
            int blocks_needed = nbyte/BLOCK_SIZE;
            if (nbyte % BLOCK_SIZE > 0){
                ++blocks_needed;
            }

            //Uses fs_read to get file data
            char* finaltemp = (char*) malloc(myroot.dir[i].size + nbyte);
            off_t tempoffset =  myroot.dir[i].offset;

            //Offset is temporarily set to 0 if not already so in case it is not already, to ensure the entire file content is read
            myroot.dir[i].offset = 0;
            if (fs_read(filedes, finaltemp, nbyte) == -1){
                fprintf(stderr, "fs_write: Could not read\n");
                return -1;
            }
            myroot.dir[i].offset = tempoffset;

            //Copying data to be written, truncating at the front based on offset, and truncating at the end based on nbyte value
            strncpy((finaltemp + myroot.dir[i].offset), (char*) buf, nbyte);
            if (strlen(finaltemp) > nbyte){
                *(finaltemp + nbyte) = '\0';
            }

            //Holds the write-to block info
            int write_to = myroot.dir[i].start_block;
            //starting and ending position of the buffer to be written into each block
            int start = 0;
            int end = BLOCK_SIZE;

            //Writes data one block at a time
            for (int j = 0; j < blocks_needed; ++j){
                //temp holds the data to be written
                char* temp = (char*) malloc(BLOCK_SIZE);
                strncpy(temp, (finaltemp + start), end);

                //Format Block to be empty before writing
                for (int j = 0; j < BLOCK_SIZE; ++j){
                    mydata.data_space[write_to].space[j] = -2;
                }

                //Write temp data
                strcpy(mydata.data_space[write_to].space, temp);

                //Adjust amount of free space left
                //If not on last iteration, strlen(temp) will equal 4096 (aka BLOCK_SIZE)
                mydata.data_space[write_to].free_blocks = BLOCK_SIZE;
                mydata.data_space[write_to].free_blocks -= strlen(temp);

                //Frees temp
                temp = 0; free(temp);

                //Increases start and end position
                start += BLOCK_SIZE;
                if (nbyte-end > BLOCK_SIZE){
                    end += BLOCK_SIZE;
                }
                else{
                    end += (nbyte-end);
                }

                //Go to next Block, or find a free block if this file does not have any more reserved blocks. This only happens if not on the last iteration
                if (j != blocks_needed-1){
                    //Finding free block if necessary
                    if (FAT1.file_ptrs[write_to] == -1 || FAT1.file_ptrs[write_to] == -2){
                        //if no more free blocks
                        if (mydata.free_data == 0){
                            //Error Message since no space, then return total number of bytes actually read up to this point
                            fprintf(stderr, "fs_write: WARNING: No Data Space Left\n");
                            //Adjust size of file
                            myroot.dir[i].size = BLOCK_SIZE*(j+1);
                            return BLOCK_SIZE*(j+1);
                        }

                        for (int k = 0; k < BLOCK_SIZE; ++k){
                            if (mydata.data_space[k].free_blocks == BLOCK_SIZE){
                                //Setting write_to to the new next block
                                FAT1.file_ptrs[write_to] = k;
                                //Setting the next block pointer to EOC (-1) in the FATs at j
                                FAT1.file_ptrs[k] = -1;
                                FAT2.file_ptrs[k] = -1;
                                //Decrementing the counter for the amount of free blocks remaining
                                mydata.free_data -= 1;
                                break;
                            }          
                        }
                    }

                    //Finally moving to next block
                    write_to = FAT1.file_ptrs[write_to];
                }
            }

            //Removing any extra data blocks previously attached to the file that are no longer needed
            int firstIteration = 1;
            while (write_to != -1){
                int temp = write_to;
                write_to = FAT1.file_ptrs[write_to];

                //Setting block truncation began in to -1 (EOC) if on the first iteration of the loop
                if (firstIteration){
                    FAT1.file_ptrs[temp] = -1;
                    FAT2.file_ptrs[temp] = -1;
                    firstIteration = 0;
                    continue;
                }

                //Setting Block to -2 (Neutral)
                FAT1.file_ptrs[temp] = -2;
                FAT2.file_ptrs[temp] = -2;
                //Incrementing amount of free data blocks
                mydata.free_data += 1;
            }

            int length = strlen(finaltemp);
            //Adjusting size of file,
            myroot.dir[i].size = length+1;
            //Returning number of bytes read
            finaltemp = 0; free(finaltemp);
            return length;
        }
    }
    return -1;
}

/*  Gets the size of a File based on its attached File Descriptor
    @param fildes : File Descriptor of File to get the size of
    @return -1 if error, or size of file on success
*/
int fs_get_filesize(int filedes){
    //Validate File Descriptor
    if(filedes < 0 || filedes > 31){
        fprintf(stderr, "fs_get_filesize: File Descriptor does not exist\n");
        return -1;
    }
    if (!(FDT.table[filedes].isActive)){
        fprintf(stderr, "fs_get_filesize: File Descriptor is not open\n");
        return -1;
    }

    //Find File attached to the File Descriptor
    for (int i = 0; i < 64; ++i){
        if (myroot.dir[i].filedes == filedes){
            //Returns size once File is identified
            return myroot.dir[i].size;
        }
    }
    return -1;
}

/*  Moves offset in the file to the specified offset argument
    @param fildes : File Descriptor of File to adjust offset of
    @param offset : amount to offset the file start by
    @return -1 if error, or 0 on success
*/
int fs_lseek(int filedes, off_t offset){
    //Validate File Descriptor
    if(filedes < 0 || filedes > 31){
        fprintf(stderr, "fs_lseek: File Descriptor does not exist\n");
        return -1;
    }
    if (!(FDT.table[filedes].isActive)){
        fprintf(stderr, "fs_lseek: File Descriptor is not open\n");
        return -1;
    }
    //Validate Offset value
    if (offset > fs_get_filesize(filedes)){
        fprintf(stderr, "fs_lseek: Offset cannot be larger than File size\n");
        return -1;
    }
    if (offset < 0 ){
        fprintf(stderr, "fs_lseek: Offset must be greater than or equal to 0\n");
        return -1;
    }

    //Find File attached to the File Descriptor, and set its offset to the argument offset
    for (int i = 0; i < 64; ++i){
        if (myroot.dir[i].filedes == filedes){
            myroot.dir[i].offset = offset;
            return 0;
        }
    }

    return -1;
}

/*  Truncates File to specified size
    @param fildes : File Descriptor of File to truncate
    @param offset : position to truncate File at
    @return -1 if error, or 0 on success
*/
int fs_truncate(int filedes, off_t length){
    //Validate File Descriptor
    if(filedes < 0 || filedes > 31){
        fprintf(stderr, "fs_truncate: File Descriptor does not exist\n");
        return -1;
    }
    if (!(FDT.table[filedes].isActive)){
        fprintf(stderr, "fs_truncate: File Descriptor is not open\n");
        return -1;
    }
    //Validate length value
    if (length > fs_get_filesize(filedes)){
        fprintf(stderr, "fs_truncate: length cannot be larger than File size\n");
        return -1;
    }
    if (length < 0 ){
        fprintf(stderr, "fs_truncate: length must be greater than or equal to 0\n");
        return -1;
    }
    //Find File attached to the File Descriptor
    for (int i = 0; i < 64; ++i){
        if (myroot.dir[i].filedes == filedes){
            //if truncation position is the same as the size, nothing has to be changed
            if (length == fs_get_filesize(filedes)){
                return 0;
            }

            //If truncation position is less than the size
            else {
                //block will be the Block to begin truncating at, and start_truncate_at is the position in the block to begin the truncation
                int block = myroot.dir[i].start_block;
                int start_truncate_at = length % BLOCK_SIZE;

                //Iterate to the block you need to begin truncation within
                for (int j = 0; j < length/BLOCK_SIZE; ++j){
                    //Checks to make sure block points to another block in the FAT
                    if (block == -1 || block == -2){
                        fprintf(stderr, "fs_truncate: Could not locate Block\n");
                        return -1;
                    }

                    //Advances to the next block as per the FAT
                    block = FAT1.file_ptrs[block];
                }

                //Use fs_read to get the data in the block up to that point and empty out the rest of the block
                char* temp = (char*) malloc(myroot.dir[i].size);

                //Temporarily switch offset to 0 for read and write, if it is not already 0
                off_t tempoffset = myroot.dir[i].offset;
                myroot.dir[i].offset = 0;
                if (fs_read(filedes, temp, (size_t) start_truncate_at) == -1){
                    fprintf(stderr, "fs_truncate: Can not read File\n");
                    return -1;
                }

                *(temp+start_truncate_at) = '\0';

                //Use fs_write to read truncated data back to block
                if (fs_write(filedes, temp, (size_t) length) == -1){
                    fprintf(stderr, "fs_truncate: Can not write to File\n");
                    return -1;
                }
                free(temp);

                //Restore offset value
                myroot.dir[i].offset = tempoffset;

                //Update free space in the block you begin truncation at
                mydata.data_space[i].free_blocks = BLOCK_SIZE - start_truncate_at;

                return 0;
            }
        }
    }
    return -1;
}