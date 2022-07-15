#include "fs.h"

int main(int argc, char const *argv[])
{
    FILE *file;
    const char *file_name;
    int i_node_count;
    int block_size;

    //////////HANDLING COMMAND LINE ARGUAMENTS//////////
    if (argc!=3) {
        fprintf(stderr, "Program is terminated. Invalid arguaments!\n");
        exit(EXIT_FAILURE);
    }

    block_size = atoi(argv[1]) * 1024;
    if(block_size>16384){       //to make it max 16 MB
        fprintf(stderr, "Program is terminated. A file can not be larger than 16MB!\n");
        exit(EXIT_FAILURE);
    }

    file_name = argv[2];
    if(strlen(argv[2])>FILE_NAME_SIZE){
        fprintf(stderr, "The program is terminated. File name is too long!\n");
        exit(EXIT_FAILURE);
    }
    ////////////////////
    file = fopen(file_name, "wb");
    if (file==NULL) {
        fprintf(stderr, "file opening error!\n");
        exit(EXIT_FAILURE);
    }
    //dumpe2fs();
}