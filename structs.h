#ifndef _STRUCT_H
#define _STRUCT_H_

/*  Struct for the File Address Table
    @param file_ptrs : List of file pointer
*/
typedef struct fat {
    int file_ptrs[4096];
} fat;

/*  Struct for directory entry
    @param start_block : location of starting block
    @param active : activity status of entry
    @param size : size of entry
    @param filedes : File Descriptor, if one is currently in use
    @param offset : offset for read or writing the file
    @name : entry name
    @param lastaccessed : date that file was last accessed
    @param modtime : time that file was last modified
    @param moddate : date that file was last modified
*/
typedef struct entry {
    int start_block;
    int active;
    int size;
    int filedes;
    off_t offset;
    char name[15];
    char lastaccessed[16];
    char modtime[16];
    char moddate[16];
} entry;

/*  Struct for root directory 
    @param dir : directory of entries
    @param entry_count : current amount of entries in dir
*/
typedef struct root {
    entry dir[64];
    int entry_count;
} root;

/*  Struct for empty block spaces
    @param space : array that represents the blocks of empty space; 1 for Empty, 0 for Full
    @param free_blocks : number of free_blocks still available in space
*/
typedef struct empty_space {
    char space[BLOCK_SIZE];
    int free_blocks;
} empty_space;

/*  Struct for boot block
    @param block_count : Number of Blocks
    @param boot_start : Where the Boot Begins
    @param fat1_start : Where FAT1 begins
    @param fat2_start : Where FAT2 begins
    @param root_start : Where Root Directory begins
    @param data_start : Where Data Space begins
    @param free_loc : Number of Free locations
    @param boot_size : Size of Boot 
    @param fat_size : Size of a FAT
    @param root_size : Size of Root Directory
    @param data_size : Size of Data Space

*/
typedef struct boot {
    int block_count;
    int boot_start;
    int fat1_start;
    int fat2_start;
    int root_start;
    int data_start;
    
    int free_loc;
    int boot_size;
    int fat_size;
    int root_size;
    int data_size;
} boot;

/*  Struct for data space holding empty space
    @param data_space : array of empty space
    @param free_data : Points to the first available data position
*/
typedef struct data {
    empty_space data_space[BLOCK_SIZE];
    int free_data;
} data;

/*  Struct for File Descriptor
    @param isActive : usage status of file description; 0 for inactive, 1 for active
    @param num : Number that the descriptor is using
*/
typedef struct file_des {
    int isActive;
} file_des;

/*  Struct for File Descriptor Table
    @param table : Array of file descriptors
    @param filled : Amount of file descriptors active
*/
typedef struct file_des_table {
    file_des table[32];
    int active_count;
} file_des_table;
#endif