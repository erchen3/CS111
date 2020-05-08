 #include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include <stdint.h>
#include "ext2_fs.h"




#define SUPERBLOCK 1024
#define GROUP 2048

int myFile; //name of our file desriptor
int totGroup;
struct ext2_super_block superblockSum;
struct ext2_group_desc* groupSum;


void superblock(){
    if((pread(myFile, superblockSum,  sizeof(struct ext2_super_block), SUPERBLOCK )) < 0){
        fprintf(stderr, "ERROR: superblock pread fail.\n");
        free(superblockSum);
        exit(1);
    }
    
    fprintf(stdout, "SUPERBLOCK,%d,%d,%d,%d,%d,%d,%d\n", superblockSum.s_blocks_count, superblockSum.s_inodes_count, SUPERBLOCK << superblockSum.s_log_block_size, superblockSum.s_inode_size, superblockSum.s_blocks_per_group, superblockSum.s_inodes_per_group, superblockSum.s_first_ino);
   
}


void group(){
	/*
		block 1 = super block
		block >1 group blocks
		so use +1
     Due to volume size boundaries, the last block group might have a smaller number of blocks than what is specified in this field.
    Integer division will truncate so +1 to include last group

	*/
    totGroup = superblockSum.s_blocks_count / superblockSum.s_blocks_per_group + 1; //check
    groupSum = malloc(totGroup * sizeof(struct ext2_group_desc));
    if(!groupSum){
        fprintf(stderr, "ERROR: group malloc fail.\n");
        exit(1);
    }
    int a = 0;
/*
go through each group
*/
    while (a < totGroup){ 
        int myBlocks;
        int myInodes;
/*
For every group block
it gets read from the image to the group descriptor and incrementally offsets
*/
        if((pread(myFile, &groupSum[a],  sizeof(struct ext2_group_desc), + a*sizeof(struct ext2_group_desc) + GROUP )) < 0){
            fprintf(stderr, "ERROR: group pread fail.\n");
            free(groupSum);
            exit(1);
        }
	/*
		Either at last group
			Either you're at the last block of a group
			Or you're in between a group
			Either you're in the last inode in group
			Or you're in between a group
		Not last group
			Report total blocks in group

			Slide 16 of Alex's, all groups have  same number of blocks excpet for the last
	
	*/
        if(a == totGroups -1 ){ 
            if(  superblockSum.s_blocks_count % superblockSum.s_blocks_per_group == 0){
                 myBlocks = superblockSum.s_blocks_per_group;
            }else{
                myBlocks = superblockSum.s_blocks_count % superblockSum.s_blocks_per_group ;
            }
            if (superblockSum.s_inodes_count % superblockSum.s_inodes_per_group == 0){
                myInodes = superblockSum.s_inodes_per_group;
            }else{
                myInodes = superblockSum.s_inodes_count % superblockSum.s_inodes_per_group;
            }
        }else{
            myBlocks = superblockSum.s_blocks_per_group;
            myInodes = superblockSum.s_inodes_per_group;
        }
        
        fprintf(stdout, "GROUP,%d,%d,%d,%d,%d,%d,%d\n", a, myBlocks, myInodes, groupSum[a].bg_free_blocks_count,groupSum[a].bg_free_inodes_count,groupSum[a].bg_block_bitmap,groupSum[a].bg_inode_bitmap,groupSum[a].bg_inode_table);
        a++:
    }
}

void inode(){
    
    struct ext2_inode *node;
    unsigned int inodeTableOffset = groupSum.bg_inode_table * (SUPERBLOCK << superblockSum.s_log_block_size);
    unsigned int i=0;
    while(i< totGroup){ //for each block group descriptor
        for( unsigned int j=0; j < superblockSum.s_inodes_count; j++){ //for each inode
            if(pread(myFile,node,sizeof(struct ext2_inode),inodeTableOffset+ j * sizeof(struct ext2_inode)) <0){
                fprintf(stderr,"ERROR: Inode pread fail\n");
                free(node);
                exit(1);
            }
            char fileType ='?'; // anything else
            if(*node.i_mode !=0  && *node.i_links_count!=0){
                fprintf(STDOUT,"INODE,%d",j+1);
                if(S_ISREG(*node.i_mode)){
                    fileType='f';
                }
                else if(S_ISDIR(*node.i_mode)){
                    fileType='d';
                }
                else if(S_ISLNK(*node.i_mode)){
                    fileType='s';
                }
                fprintf(STDOUT,"%c,%o,%d,%d",fileType, *node.i_mode,*node.i_uid,*node.i_gid,*node.i_links_count);
               
                char changeTime[20];
                char modTime[20];
                char accessTime[20];
                
                time_t theTime = *node.i_ctime;
                struct tm *content = gmtime(&theTime);
                strftime(changeTime,80,"%m/%d/%y %H:%M:%S", content);
                
                theTime=*node.i_mtime;
                content = gmtime(&theTime);
                strftime(modTime,80,"%m/%d/%y %H:%M:%S", content);
                
                theTime=*node.i_mtime;
                content= gmtime(&theTime);
                strftime(accessTime,80,"%m/%d/%y %H:%M:%S", content);
                
                fprintf(STDOUT,"%s,%s,%s,%d,%d", changeTime,modTime,accessTime, *node.i_size, *node.i_blocks);
            }
        }
        i++;
    }
    if(fileType ='s' && *node.i_size < 60){
        fprintf(STDOUT,"%d", *node.i_block[0]);
    }
    else{
        for(unsigned int i=0; i< EXT2_N_BLOCK; i++)
            fprintf(STDOUT,"%d",*node.i_block[i]);
    }
    fprintf(STDOUT,"\n");
}


int main(int argc, char* argv[]){
    if(argc != 2){
        fprintf(stderr, "ERROR: Wrong number of arguments for ./lab3a.\n");
        exit(1);
    }
    
    myFile = open(argv[1], O_RDONLY);
    if(myFile == -1){
        fprintf(stderr, "ERROR: Can't open file \n");
        exit(1);
    }
    
    
    superblock(); //done
    group(); //done
    /*
     http://cs.smith.edu/~nhowe/Teaching/csc262/oldlabs/ext2.html
     For each group there is a bit map
     each bitmap has many blocks for it  and we must traverse the 8 bytes to  check if theres a free block
     see the diagram in the link for understanding
     */
    freeBlock();// julia
    /*
     1 group
     For every byte
    go through each bit doing the bitwise AND to check if empty and if so (0)
     then print location
     */
    freeInode();
    inode();  //Eric
    directory();
    indirectBlockRef();
    exit(0);
}
