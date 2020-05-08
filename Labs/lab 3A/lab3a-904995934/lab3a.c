/*
NAME: ERIC CHEN, JULIA WANG
EMAIL: erchen3pro@gmail.com, julia.wang.ca@gmail.com 
UID: 


*/
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

/*
 To-Do  
    Directory code inside make sure its in scope
    Comment out below functions in main

*/



#define SUPERBLOCK 1024
#define GROUP 2048

int myFile; //name of our file desriptor
int totGroup;
struct ext2_super_block superblockSum;
struct ext2_group_desc* groupSum;


void superblock(){
  if((pread(myFile,&superblockSum,sizeof(struct ext2_super_block), SUPERBLOCK )) < 0){
    fprintf(stderr, "ERROR: superblock pread fail.\n");
    
    exit(1);
  }
  fprintf(stdout, "SUPERBLOCK,%d,%d,%d,%d,%d,%d,%d\n", superblockSum.s_blocks_count, superblockSum.s_inodes_count, SUPERBLOCK << superblockSum.s_log_block_size, superblockSum.s_inode_size, superblockSum.s_blocks_per_group, superblockSum.s_inodes_per_group, superblockSum.s_first_ino);
}


void group(){
  totGroup = superblockSum.s_blocks_count / superblockSum.s_blocks_per_group + 1; //integer division block count can be > blocks per group, can make it 0 but minimum is 1 so +1
  groupSum = malloc(totGroup * sizeof(struct ext2_group_desc));
  if(!groupSum){
    fprintf(stderr, "ERROR: group malloc fail.\n");
    exit(1);
  }
  int a = 0;
  while (a < totGroup){
    int myBlocks;
    int myInodes;
    if((pread(myFile,&groupSum[a],sizeof(struct ext2_group_desc), a*sizeof(struct ext2_group_desc) + GROUP )) < 0){
      fprintf(stderr, "ERROR: group pread fail.\n");
      free(groupSum);
      exit(1);
    }
    if(a == totGroup -1 ){
      if(superblockSum.s_blocks_count % superblockSum.s_blocks_per_group == 0){
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
    
    fprintf(stdout, "GROUP,%d,%d,%d,%d,%d,%d,%d,%d\n", a, myBlocks, myInodes, groupSum[a].bg_free_blocks_count, groupSum[a].bg_free_inodes_count , groupSum[a].bg_block_bitmap, groupSum[a].bg_inode_bitmap, groupSum[a].bg_inode_table);
    a++;
  }
}

void freeBlock(){
  
  int a = 0;
     int sizeBlock = SUPERBLOCK << superblockSum.s_log_block_size;
     while (a < totGroup){
       __u32 curr = groupSum[a].bg_block_bitmap ;
       int offset = a * superblockSum.s_blocks_per_group;
       int b = 0;
       while ( b < sizeBlock){
	 uint8_t myBlockBitmap;
	 if (pread(myFile, &myBlockBitmap, 1, (curr * sizeBlock) + b) < 0){
	   fprintf(stderr, "ERROR: freeBlock pread fail.\n");
	   exit(1);
	 }
	 
	 int c = 0;
	 while ( c < 8){ 
	   int curBloc = (c+1) + (b * 8);
	   if (!(myBlockBitmap & (1 << c))){
	     fprintf(stdout, "BFREE,%d\n", (offset) + curBloc);
	   }
	   
	   c++;
	 }
	 b++;	
       }
       a++;
     }
}
void freeInode(){
  int a = 0;
  int sizeBlock = SUPERBLOCK << superblockSum.s_log_block_size;
  while (a < totGroup){
    
    __u32 curr = groupSum[a].bg_inode_bitmap ;
    int offset = a * superblockSum.s_blocks_per_group;
    int b = 0;
    while ( b < sizeBlock){
      uint8_t myInodeBitmap;
      if (pread(myFile, &myInodeBitmap, 1, (curr * sizeBlock) + b) < 0){
	fprintf(stderr, "ERROR: freeInode pread fail.\n");
	exit(1);
      }
      int c = 0;
      while ( c < 8){ 
	int curBloc = (c+1) + (b * 8);               
	if (!(myInodeBitmap & (1 << c))){
	  fprintf(stdout, "IFREE,%d\n", (offset) + curBloc);
	}
	c++;
      }
      b++;
    }
    a++;
  }
}


void printIndirect(int inode, int blockOffset, int levels, int blockNumber){
  int addresses = (SUPERBLOCK << superblockSum.s_log_block_size)/4;  // 1024 bytes(blocksize)/ 4 bytes(size of element in i_block) = #addresses
    int *blook = malloc(SUPERBLOCK << superblockSum.s_log_block_size); //The block to read level 1/2/3 block references

    if(pread(myFile,blook,(SUPERBLOCK << superblockSum.s_log_block_size),blockNumber * (SUPERBLOCK << superblockSum.s_log_block_size)) <0){
      fprintf(stdout,"Error: indirect block pread issue\n");
      exit(1);
    }
    int index =0;
    while(index < addresses){ // for each address
      switch(blook[index]){
      case 0://move on to the next block at 1st indirection level block when we're invalid
	if(levels ==1){
	  blockOffset+=1;
	}
	else if(levels ==2){
	  blockOffset+=256;
	}
	else if(levels==3){
	  blockOffset+=(256*256);
	}
	break;
      default: //valid
	fprintf(stdout,"INDIRECT,%d,%d,%d,%d,%d\n",inode,levels,blockOffset,blockNumber,blook[index]);
	if(levels ==1){ //valid now move onto next at leaf level 
	  blockOffset+=1;
	}
	else if(levels == 2 || levels ==3){//Recurse a down to leaf level
	  printIndirect(inode,blockOffset,levels-1,blook[index]);
	}
	break;
      }
      index++;
    }
    free(blook);
}
void inode(){
  
  struct ext2_inode node;
  unsigned int inodeTableOffset = groupSum->bg_inode_table * (SUPERBLOCK << superblockSum.s_log_block_size);
  int i=0;
  while(i< totGroup){ //for each block group descriptor
    for( unsigned int j=0; j < superblockSum.s_inodes_count; j++){ //for each inode
	  if(pread(myFile,&node,sizeof(struct ext2_inode),inodeTableOffset+ j * sizeof(struct ext2_inode)) <0){
	    fprintf(stderr,"ERROR: Inode pread fail\n");
	    exit(1);
	  }
	  char fileType ='?'; // anything else
	  if(node.i_mode !=0  && node.i_links_count!=0){
	    fprintf(stdout,"INODE,%d,",j+1);
	    if(S_ISREG(node.i_mode) && !S_ISDIR(node.i_mode) && !S_ISLNK(node.i_mode)){
	      fileType='f';
	    }
	    else if(S_ISDIR(node.i_mode) && !S_ISREG(node.i_mode) && !S_ISLNK(node.i_mode)){
	      fileType='d';
	    }
		else if(S_ISLNK(node.i_mode) && !S_ISREG(node.i_mode) && !S_ISDIR(node.i_mode)){
	      fileType='s';
	    }
	    fprintf(stdout,"%c,%o,%d,%d,%d,",fileType, node.i_mode & 0x0FFF, node.i_uid, node.i_gid, node.i_links_count);
        //0x0FFF , lower order twelve bits
	    char changeTime[20];
	    char modTime[20];
	    char accessTime[20];
            
	    time_t theTime = node.i_ctime;
	    struct tm *content = gmtime(&theTime);
	    strftime(changeTime,80,"%m/%d/%y %H:%M:%S", content);
            
	    theTime= node.i_mtime;
	    content = gmtime(&theTime);
	    strftime(modTime,80,"%m/%d/%y %H:%M:%S", content);
                
	    theTime= node.i_atime;
	    content= gmtime(&theTime);
	    strftime(accessTime,80,"%m/%d/%y %H:%M:%S", content);
            
	    fprintf(stdout,"%s,%s,%s,%d,%d", changeTime,modTime,accessTime, node.i_size, node.i_blocks);
	    
	    
	    //next 15 block addresses

	    if(((fileType =='f') || (fileType =='d')) && (fileType !='s')){
	      for (int i = 0; i < EXT2_N_BLOCKS; i++) {
		fprintf(stdout,",%d", node.i_block[i]);
	      }
	    }
	      fprintf(stdout,"\n");
	    /*
	/////////
	DIRECTORY
	/////////
	*/
	    if(fileType == 'd'){
	      int t =0;
	      while( t < EXT2_NDIR_BLOCKS){
		struct ext2_dir_entry myDirectory;
		unsigned int logicalOffset=0;
		if(node.i_block[t] != 0){
		  int offset = node.i_block[t] * 1024;
		  while(logicalOffset < 1024){
		    if(pread(myFile, &myDirectory, sizeof(struct ext2_dir_entry), offset+logicalOffset) < 0 ){
		      fprintf(stderr,"ERROR: pread directory fail\n");
		      exit(2);
		    }
		    if(myDirectory.inode !=0){
		      fprintf(stdout, "DIRENT,%d,%d,%d,%d,%d,'%s'\n",
			      j+1,
			      logicalOffset,
			      myDirectory.inode,
			      myDirectory.rec_len,
			      myDirectory.name_len,
			      myDirectory.name);
		    }
		    logicalOffset+=myDirectory.rec_len;
		  }		   
		}else{
		  break;
		}
		t++;
	      }
	    }
	    
	    /*
///////////////
INDIRECT BLOCKS
//////////////
   */
	    if(fileType =='f' || fileType=='d'){
	      //Reminder inode number starts at 1
	      printIndirect(j+1,12,1, node.i_block[12]);
	      printIndirect(j+1,268,2,node.i_block[13]);
	      printIndirect(j+1,65804,3,node.i_block[14]);
	    }

	  }
    }
    i++;
  }
  
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
  
  freeBlock(); //done --need to check
  freeInode(); //done --need to check
  
  inode(); //DONE

  
  
  free(groupSum);
  exit(0);
  
}
