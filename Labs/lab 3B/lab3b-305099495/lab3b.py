#NAME: JULIA WANG, ERIC CHEN
#EMAIL:julia.wang.ca@gmail.com, erchen3pro@gmail.com
#UID: 904995934, 305099495

#To-Do:
#
#   Structure:
#       1. Read CSV and group similar data (BFREE,IFREE, ...) into lists/dictionaries/structures
#       2. pass these structures into auditing functions and they will process them accordingly.
#
import csv
import sys


dirents=[]
blockBitmap=[]
inodes={} #dictionary, key = inode #  | value = inode
myBlocks = dict() 
inodeBitmap=[]
group = None
superBlook= None
allocatedInodes = set() # global list for allocated iNodes holding inodeNumber

hasIncons = False;
class superBlock():
    def __init__(self, totBlock, totInode, blockSize, inodeSize, firstBlcInode):
        self.totBlock = totBlock
        self.totInode = totInode
        self.blockSize = blockSize
        self.inodeSize = inodeSize
        self.firstBlcInode = firstBlcInode

class Group():
    def __init__(self,totBlock,totInode, blockOfInode):
        self.totBlock = totBlock  
        self.totInode = totInode
        self.blockOfInode = blockOfInode

class inode():
    def __init__(self,iNum,group, linkCount):
        self.iNum =iNum
        self.group =group
        self.linkCount=linkCount
        self.direct= []
        self.single=0
        self.double =0
        self.triple= 0

class myBlockData():
    def __init__(self,blckNum,iNum, offset, lev):
        self.blckNum = blckNum
        self.iNum = iNum
        self.offset = offset
        self.lev = lev

class directory():
    def __init__(self,pNum,iNum,entryLength,nameLength,name):
        self.pNum=pNum
        self.iNum= iNum
        self.entryLength=entryLength
        self.nameLength =nameLength
        self.name= name

        
#Function that Audit the Inodes
#Arguments:
#   inodeList, a  dictionary of inodes 
#   superBlock, a superBlock object
#   inodeMap, inode Bitmap list of inode Numbers that tells us which inodes are noted as free
def inodeAudit(superBlock, inodeMap):
    for iNumber in allocatedInodes: # Iterate through each inode and compare inode Number of allocated to free list inode Number
        if iNumber in inodeMap: #allocated but also in free list
            sys.stdout.write("ALLOCATED INODE {0} ON FREELIST\n".format(iNumber))

    for iNumber  in range(superBlock.firstBlcInode,superBlock.totInode):#iterate through all inodes and if they arent allocated nor  in free list we mark as unallocated
        if iNumber not in allocatedInodes and iNumber not in inodeMap:
            sys.stdout.write("UNALLOCATED INODE {0} NOT ON FREELIST\n".format(iNumber))

#Function that audits the blocks
#Arguments:
#   superBlock, a superblock
#   group, takes a group object
def blockConst(superBlock, group):
    global hasIncons
    checkInv = int(superBlock.totBlock)
    startOfNonReservedBlock = int(group.blockOfInode + (superBlock.inodeSize * group.totInode)/superBlock.blockSize)
    for a in myBlocks.keys():
        tempBlock = list(myBlocks[a])
        makeBlock = tempBlock[0];
        
        numBlock = makeBlock.blckNum
        inode = makeBlock.iNum
        offset = makeBlock.offset
        levelBlock = makeBlock.lev
        
        
        if levelBlock == 1:
            typeBlock = "INDIRECT BLOCK"
        elif levelBlock == 2:
            typeBlock = "DOUBLE INDIRECT BLOCK"
        elif levelBlock == 3:
            typeBlock = "TRIPLE INDIRECT BLOCK"
        else:
            typeBlock = "BLOCK"
        
        change = True
        if a < 0 or a >= checkInv:
            sys.stdout.write("INVALID {0} {1} IN INODE {2} AT OFFSET {3}\n".format(typeBlock, numBlock, inode, offset))
        elif numBlock in blockBitmap: # check the block free list
            sys.stdout.write("ALLOCATED BLOCK {0} ON FREELIST\n".format(numBlock))
        elif a < startOfNonReservedBlock:
            sys.stdout.write("RESERVED {0} {1} IN INODE {2} AT OFFSET {3}\n".format(typeBlock, numBlock, inode, offset))
        elif len(tempBlock) > 1:
            for dup in tempBlock:
                level = dup.lev
                dupNumBlock = dup.blckNum
                dupInode = dup.iNum
                dupOff = dup.offset
       
                if level == 1:
                    sys.stdout.write("DUPLICATE INDIRECT BLOCK {0} IN INODE {1} AT OFFSET {2}\n".format( dupNumBlock, dupInode, dupOff))
                elif level == 2:
                    sys.stdout.write("DUPLICATE DOUBLE INDIRECT BLOCK {0} IN INODE {1} AT OFFSET {2}\n".format( dupNumBlock, dupInode, dupOff))
                elif level == 3:
                    sys.stdout.write("DUPLICATE TRIPLE INDIRECT BLOCK {0} IN INODE {1} AT OFFSET {2}\n".format( dupNumBlock, dupInode, dupOff))
                else:
                    sys.stdout.write("DUPLICATE BLOCK {0} IN INODE {1} AT OFFSET {2}\n".format( dupNumBlock, dupInode, dupOff))

        else:
            change = False
        if change == True:
            hasIncons = True
    
                
    for b in range(startOfNonReservedBlock+1,checkInv):
        if b not in blockBitmap and b not in myBlocks.keys():
            sys.stdout.write("UNREFERENCED BLOCK {0}\n".format(b))
            hasIncons = True

#Function that audits the directories
#Arguments:
#   inodeList, a dictionary of inodes
#   superBlock, a superblock object
#   directory, a list of dirents
#   inodeMap, the inode bitmap list of inode numbers that tells us which inodes are free
def direntAudit(inodeList, superBlock, directory, inodeMap):
    linkCounts =[0] *superBlock.totInode # discovered link count of inode's existing in directories 
    parents = [0] * superBlock.totInode
    parents[2] =2 

    for dirent in directory:
        if dirent.iNum not in allocatedInodes and dirent.iNum in inodeMap: #not allocated and in the inode bitmap as free
            sys.stdout.write("DIRECTORY INODE {0} NAME {1} UNALLOCATED INODE {2}\n".format(dirent.pNum,dirent.name,dirent.iNum))
        elif dirent.iNum > superBlock.totInode or dirent.iNum <1:
            sys.stdout.write("DIRECTORY INODE {0} NAME {1} INVALID INODE {2}\n".format(dirent.pNum,dirent.name,dirent.iNum))
        else:
            linkCounts[dirent.iNum] +=1 #enumeration of links

    for dirent in directory:#save pointer to parent
            if dirent.name != "'.'" and dirent.name !="'..'":
                parents[dirent.iNum]= dirent.pNum

    for inode in inodeList.values():
        if inode.linkCount != linkCounts[inode.iNum]:
            sys.stdout.write("INODE {0} HAS {1} LINKS BUT LINKCOUNT IS {2}\n".format(inode.iNum,linkCounts[inode.iNum],inode.linkCount))
    
    for  dirent in directory: #check the links of . & ..
        if dirent.name == "'.'" and dirent.iNum !=dirent.pNum: #refer to itself
            sys.stdout.write("DIRECTORY INODE {0} NAME '.' LINK TO INODE {1} SHOULD BE {2}\n".format(dirent.pNum, dirent.iNum,dirent.pNum))
        elif dirent.name =="'..'" and dirent.iNum != parents[dirent.iNum]: # if i'm parent directory, the referred file num should be the same as the referred file num's parent num
            sys.stdout.write("DIRECTORY INODE {0} NAME '..' LINK TO INODE {1} SHOULD BE {2}\n".format(dirent.pNum,dirent.iNum,parents[dirent.iNum]))

def main():
    if len(sys.argv) < 2:
        sys.stderr.write("Error: Not enough arguments passed \n")
        sys.exit(1)

                  
    with open(sys.argv[1],'r') as mycsv:
        csvFile = csv.reader(mycsv)

        for cell in csvFile: #go through each row in csv File
            check = cell[0]
            if check== 'SUPERBLOCK':
                superBlook =superBlock(int(cell[1]), int(cell[2]), int(cell[3]), int(cell[4]), int(cell[7])) #assign superblock object - J
            elif check =='GROUP':
                group = Group(int(cell[2]),int(cell[3]),int(cell[8]))
            elif check == 'BFREE':
                blockBitmap.append(int(cell[1])) #append just number of free block -!
            elif check =='IFREE':
                inodeBitmap.append(int(cell[1])) #append just the number of free inode -!
            elif check =='INODE':  # Build inode dictionary, key values are the inode number
                unoInode = inode(int(cell[1]),int(cell[5]),int(cell[6]))
                unoInode.direct = [int(i) for i in cell[12:-3]]
                unoInode.single =int(cell[-3])
                unoInode.double =int(cell[-2])
                unoInode.triple= int(cell[-1])
                #finished initialization of a single inode
                inodes[int(cell[1])]= unoInode #put into the dictionary
                for a in range(15):
                    address = int(cell[12+a])
                    if a >= 12:
                        if a - 11 == 1:
                            curOffset = 12
                        elif a - 11 == 2:
                            curOffset = 268
                        elif a - 11 == 3:
                            curOffset = 65804
                        else:
                            curOffset = a
                        if address > 0:
                            newBlock = myBlockData(address, int(cell[1]), curOffset,  a - 11 )
                    else:
                        if address > 0:
                            newBlock = myBlockData(address, int(cell[1]), a, 0 )
                    if address > 0:
                        if address not in myBlocks:
                            myBlocks[address] = set()
                        myBlocks[address].add(newBlock)
                  
            elif check =='DIRENT':
                dirents.append(directory(int(cell[1]),int(cell[3]),int(cell[4]),int(cell[5]),cell[6])) #append dirent object
            elif check =='INDIRECT':
                
                newBlock = myBlockData(int(cell[5]), int(cell[1]), int(cell[3]),int(cell[2]) - 1 )
                if int(cell[5]) not in myBlocks:
                    myBlocks[int(cell[5])] = set()
                myBlocks[int(cell[5])].add(newBlock)
                
    for inodeNum in inodes:#initialized set of allocatedInodes
        allocatedInodes.add(inodeNum) #puts the key in the set 
   
    #for index, value in myBlocks.items():
    	#print (index, "=>",value)


    blockConst(superBlook,group)
    inodeAudit(superBlook,inodeBitmap)
    direntAudit(inodes,superBlook, dirents,inodeBitmap)

    if hasIncons ==True:
        sys.exit(2)
    if hasIncons == False:
        sys.exit(0)

if __name__ =='__main__':
    main()

