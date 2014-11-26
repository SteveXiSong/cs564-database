#include <memory.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <fcntl.h>
#include <iostream>
#include <stdio.h>
#include "page.h"
#include "buf.h"

#define ASSERT(c)  { if (!(c)) { \
		       cerr << "At line " << __LINE__ << ":" << endl << "  "; \
                       cerr << "This condition should hold: " #c << endl; \
                       exit(1); \
		     } \
                   }

//----------------------------------------
// Constructor of the class BufMgr
//----------------------------------------

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
	for (int i = 0; i < numBufs; i++) {
		if (bufTable[i].valid == true)
			if (bufTable[i].dirty == true)
				bufTable[i].file->writePage(bufTable[i].pageNo, &bufPool[i]);
	}
	delete bufTable;
	delete bufPool;
	delete hashTable;
}


const Status BufMgr::allocBuf(int & frame) {
	// TODO: Implement this method by looking at the description in the writeup.
    cout << "allocBuf ..."<<endl;

	int counter = 0;

	while (true) {
		advanceClock();
		counter++;
		if (counter > 2 * numBufs)
			return BUFFEREXCEEDED;
		if (bufTable[clockHand].valid == false)
			break;
		if (bufTable[clockHand].refbit == true) {
			bufTable[clockHand].refbit = false;
			continue;
		}
		if (bufTable[clockHand].pinCnt != 0)
			continue;
		break;
	}

	if (bufTable[clockHand].dirty == true) {
		if (bufTable[clockHand].file->writePage(bufTable[clockHand].pageNo, &bufPool[clockHand]) != OK)
			return UNIXERR;
	}

    if(bufTable[clockHand].valid == true){
        if( hashTable->remove(bufTable[clockHand].file, bufTable[clockHand].pageNo) != OK)
            return HASHTBLERROR;
    }

	bufTable[clockHand].Clear();

	frame = clockHand;

	return OK;
}


const Status BufMgr::readPage(File* file, const int PageNo, Page*& page) {
	// TODO: Implement this method by looking at the description in the writeup.
    cout << "readPage ... " <<endl;
	int FrameNo = -1;
	Status info = NOTUSED1;

	if (hashTable->lookup(file, PageNo, FrameNo) != OK) {
		info = allocBuf(FrameNo);
		if (info != OK)
			return info;
		if (file->readPage(PageNo, &bufPool[FrameNo]) != OK)
			return UNIXERR;
		if (hashTable->insert(file, PageNo, FrameNo) != OK)
			return HASHTBLERROR;
		bufTable[FrameNo].Set(file, PageNo);
		page = &bufPool[FrameNo];
	}
	else {
		bufTable[FrameNo].refbit = true;
		bufTable[FrameNo].pinCnt++;
		page = &bufPool[FrameNo];
	}

	return OK;
}


const Status BufMgr::unPinPage(File* file, const int PageNo,
			       const bool dirty) {
	// TODO: Implement this method by looking at the description in the writeup.
	int FrameNo = -1;

	if (hashTable->lookup(file, PageNo, FrameNo) != OK)
		return HASHNOTFOUND;

	if (bufTable[FrameNo].pinCnt == 0)
		return PAGENOTPINNED;
	bufTable[FrameNo].pinCnt--;
	if (dirty == true)
		bufTable[FrameNo].dirty = true;

	return OK;
}


const Status BufMgr::allocPage(File* file, int& pageNo, Page*& page)  {
	// TODO: Implement this method by looking at the description in the writeup.

	int FrameNo = -1;
	Status info = NOTUSED1;

	if (file->allocatePage(pageNo) == UNIXERR)
		return UNIXERR;

	info = allocBuf(FrameNo);
	if (info != OK)
		return info;

	if (hashTable->insert(file, pageNo, FrameNo) != OK)
		return HASHTBLERROR;

	bufTable[FrameNo].Set(file, pageNo);
    page = &bufPool[FrameNo];

	return OK;
}


const Status BufMgr::disposePage(File* file, const int pageNo) {
	// TODO: Implement this method by looking at the description in the writeup.
	int FrameNo = -1;

	if (hashTable->lookup(file, pageNo, FrameNo) == OK) {
		bufTable[FrameNo].Clear();
		hashTable->remove(file, pageNo);
		return file->disposePage(pageNo);
	}

	return OK;
}


const Status BufMgr::flushFile(const File* file) {
	// TODO: Implement this method by looking at the description in the writeup.
	for (int i = 0; i < numBufs; i++) {
		if (bufTable[i].file == file && bufTable[i].valid == true) {
			if (bufTable[i].pinCnt != 0)
				return PAGEPINNED;
			if (bufTable[i].dirty == true)
				bufTable[i].file->writePage(bufTable[i].pageNo, &bufPool[i]);
			hashTable->remove(file, bufTable[i].pageNo);
			bufTable[i].Clear();
		}
	}

	return OK;
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


