#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#ifndef _MYLIB_H_
#define _MYLIB_H_
#define FILE_NAME_SIZE 112 //in bits
#define BLOCK_SIZE_MAX 131072 //in KB
#define KB (1024)
#define MB (KB*1024)
#define TOTAL_INODE_NUMBER 256 //assumed
#define TOTAL_BITMAP_BLOCKS_NUMBER 256 //assumed


typedef struct st_inode
{
    unsigned int file_size;

    unsigned int file_creation_second;
    unsigned int file_creation_minute;
    unsigned int file_creation_hour;

    unsigned int last_modify_second;
    unsigned int last_modify_minute;
    unsigned int last_modfiy_hour;

    unsigned int last_modify_day;
    unsigned int last_modify_month;
    unsigned int last_modfiy_year;

    unsigned int indirect_block_single;
    unsigned int indirect_block_double;
    unsigned int indirect_block_triple;

    unsigned int inode_location;    //keeps the location of files
    bool isFile;                    //Since a file structer is not defined in the homework, i check that if it is a file or not in the directory structure.


    unsigned file_page_number; // in i-nodes there can be many more metadata such as owner,group,security data etc.. file_page_number and owner is implemented here juss as an example.
    char *owner;

}_inode;


typedef struct st_directoryEntry
{
    unsigned char file_name[14];    // should be 14 bytes
    int8_t inode_number;            // should be 2 bytes

}_directoryEntry;

typedef struct st_directory
{
    _inode inode;
    char *directory_path;
    unsigned int size;
    bool isEmpty;                   //check that if the directory is enpty or not.
}_directory;

typedef struct st_superblock            //superblock is essentially a system metadata
{
    unsigned int magic_number;          //the magic number to identify the system type (executable or not or something else).
    unsigned int number_of_blocks;      //keeps the total number of blocks.
    unsigned int number_of_datablocks;  //keeps the total number of datablock.
    unsigned int number_of_inodes;      //keeps the total number of inodes.
    unsigned int block_size;            //keeps the block size.
    unsigned int root_directory_position;//keeps the root directory position.
    unsigned int data_block_position;    //keeps the position of datablocks.
    //unsigned int *free_block_list;      //free blocks can be kept in free block list
    unsigned int bitmap_blocks[TOTAL_BITMAP_BLOCKS_NUMBER];     //free blocks can be also kept in bitmap as array
    unsigned int first_block_address;
    unsigned int first_inode_address;
}_superblock;
#endif