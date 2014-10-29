#include <memory.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <fcntl.h>
#include <iostream>
#include <stdio.h>
#include "page.h"
#include "buf.h"
#include <assert.h>

#define ASSERT(c)  { if (!(c)) { \
		       cerr << "At line " << __LINE__ << ":" << endl << "  "; \
                       cerr << "This condition should hold: " #c << endl; \
                       exit(1); \
		     } \
                   }

//----------------------------------------
// Constructor of the class BufMgr
//----------------------------------------
//#define DEBUG
#define DEBUG_PINPAGE
using namespace std;

BufMgr::BufMgr(const int bufs)
{
    numBufs = bufs;

    bufTable = new BufDesc[bufs];
    memset(bufTable, 0, bufs * sizeof(BufDesc));
    for (int i = 0; i < bufs; i++)
    {
        bufTable[i].frameNo = i;
        bufTable[i].valid = false;
    }

    bufPool = new Page[bufs];
    memset(bufPool, 0, bufs * sizeof(Page));

    int htsize = ((((int) (bufs * 1.2))*2)/2)+1;
    hashTable = new BufHashTbl (htsize);  // allocate the buffer hash table

    clockHand = bufs - 1;
}


BufMgr::~BufMgr() {
    //flush out all the dirty pages
    for(int i = 0; i < numBufs; i++){
        if(bufTable[i].dirty == true) {
            bufTable[i].file->writePage(bufTable[i].pageNo, &bufPool[bufTable[i].frameNo]);
        }
    }

    //deallocate all the buffer pool
    delete bufPool;
    delete hashTable;
    delete bufTable;
}


const Status BufMgr::allocBuf(int & frame) {
    //allocate a free frame using clock algorithm,
#ifdef DEBUG
    cout<< "allocBuf..." <<endl;
#endif

    Status retStat;
    bool isIdlePage = false;

    //return BUFFEREXCEEDED if all are pinned
    int pinnedPageNum = 0;
    BufDesc *curDesc;

    while(pinnedPageNum != numBufs){
        advanceClock();
#ifdef DEBUG_PINPAGE
        cout << "pinned page num " << pinnedPageNum << "/" <<numBufs<<endl;
#endif
        curDesc = &bufTable[clockHand];

        //valid bit is not set, idle page, use it, break and set()
        if( !curDesc->valid ){
            isIdlePage = true;
            break;
        }

        //refbit = true, clear it and continue
        if( curDesc->refbit ){
            bufTable[clockHand].refbit = false;
            continue;
        }

        if( curDesc->pinCnt != 0){
            pinnedPageNum ++;
            if(pinnedPageNum == numBufs){
                return BUFFEREXCEEDED;
            }
            continue;
        }else{
            break;
        }
    }


    /* find a frame, look up the bufTable and obtain the file and pageNo
     * dirty page should not be in idle page frame
     * flush the dirty page onto disk
     * return UNIXERR if IO error
     */
    if(curDesc->dirty){
        assert(!isIdlePage);
        retStat = curDesc->file->writePage(curDesc->pageNo, &bufPool[curDesc->frameNo] );
        if(retStat != OK)
            return UNIXERR;
    }

    //if it is valid, obtain the file and pageNo, remove it from the hashtable
    if(curDesc->valid){
       retStat = hashTable->remove(curDesc->file, curDesc->pageNo);
        if(retStat != OK)
            return retStat;
    }

    //invoke set(), filePtr pageNum?
    curDesc->Clear();

    //use the frame
    frame = curDesc->frameNo;

	return OK;
}


const Status BufMgr::readPage(File* file, const int PageNo, Page*& page) {
#ifdef DEBUG
    cout<< "readPage..." <<endl;
#endif

    Status retStat;
    int frameNo;
    retStat = hashTable->lookup( file, PageNo, frameNo);
    if( retStat == HASHNOTFOUND){
        //case 1: hash not found
        if( (retStat = allocBuf(frameNo)) != OK)
            return retStat;

        page = &bufPool[frameNo];
        if( (retStat = file->readPage(PageNo, page) ) != OK)
            return retStat;

        hashTable->insert( file, PageNo, frameNo);
        bufTable[frameNo].Set(file, PageNo);
    }else{
        //case 2
        bufTable[frameNo].refbit = true;
        bufTable[frameNo].pinCnt ++;
        page = &bufPool[frameNo];
    }
	return OK;
}


const Status BufMgr::unPinPage(File* file, const int PageNo,
			       const bool dirty) {
#ifdef DEBUG
    cout<< "unPinPage..." <<endl;
#endif

    int frameNo;

    if (hashTable->lookup(file, PageNo, frameNo) == HASHNOTFOUND)
        return HASHNOTFOUND;

    if( bufTable[frameNo].pinCnt == 0)
        return PAGENOTPINNED;

    if( dirty == true){
        bufTable[frameNo].dirty = dirty;
    }

    bufTable[frameNo].pinCnt --;

	return OK;
}


const Status BufMgr::allocPage(File* file, int& pageNo, Page*& page)  {
#ifdef DEBUG
    cout<< "allocPage..." <<endl;
#endif
    Status retStat;
    int frameNo;
    //allocate an empty file, pageNo is returned
    if( (retStat = file->allocatePage(pageNo)) != OK )
        return UNIXERR;

    //allocate buf to obtain a buf pool frame
    if((retStat = allocBuf(frameNo)) != OK)
        return retStat;

    if((retStat = hashTable->insert(file, pageNo, frameNo)) !=OK)
        return HASHTBLERROR;

    bufTable[frameNo].Set(file, pageNo);
    page = &bufPool[frameNo];
	return OK;
}


//check if the page is already in the buf pool
const Status BufMgr::disposePage(File* file, const int pageNo) {
#ifdef DEBUG
    cout<< "disposePage..." <<endl;
#endif
    int frameNo;
    Status retStat;
    if( hashTable->lookup( file, pageNo, frameNo) != HASHNOTFOUND ){
        bufTable[frameNo].Clear();
        hashTable->remove(file, pageNo);
    }
    retStat = file->disposePage(pageNo);
	return retStat;
}


//flush the file onto disk by calling lower IO api
//scan the buf tables and find all the entries of this file
const Status BufMgr::flushFile(const File* file) {
#ifdef DEBUG
    cout<< "flushFile..." <<endl;
#endif
    Status retStat = OK;

    //scan the bufTable
    for(int i = 0; i < numBufs ;i++){
        if( bufTable[i].file == file){
            //!if it is pinned, should we flush it and remove?
            if( bufTable[i].pinCnt != 0){
                retStat = PAGEPINNED;
                continue;
            }

            if( bufTable[i].dirty == true){
                bufTable[i].file->writePage(
                        bufTable[i].pageNo, &bufPool[bufTable[i].frameNo]);
                bufTable[i].dirty = false;
            }
            hashTable->remove(bufTable[i].file, bufTable[i].pageNo);
            bufTable[i].Clear();
        }
    }
	return retStat;
}


void BufMgr::printSelf(void)
{
    BufDesc* tmpbuf;

    cout << endl << "Print buffer...\n";
    for (int i=0; i<numBufs; i++) {
        tmpbuf = &(bufTable[i]);
        cout << i << "\t" << (char*)(&bufPool[i])
             << "\tpinCnt: " << tmpbuf->pinCnt;

        if (tmpbuf->valid == true)
            cout << "\tvalid\n";
        cout << endl;
    };
}


