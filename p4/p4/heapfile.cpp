/******************************************************************************
Group member:  JING JING
UW netID:      jjing2

Group member:  Xi Song
UW netID:      xsong44

Course:        CS 564, Fall 2014
Assignment:    Programming Assignment 4 The Heap File 
******************************************************************************/


//-----------------------------------------------------------------------------
// File manager: manager heap files that also provides a scan mechanism
// 1). FileHdrPage class
//	createHeapFile()
//	destroyHeapFiile()
// 2). HeapFile class
//	HeapFile()
//	~HeapFile()
//	getRecCnt()
//	getRecord()
// 3). HeapFileScan class
//	HeapFileScan()
//	~HeapFileScan()
//	startScan()
//	endScan()
//	markScan()
//	resetScan()
//	scanNext()
//	getRecord()
//	deleteRecord()
//	markDirty()
//	matchRec()     
//-----------------------------------------------------------------------------


#include "heapfile.h"
#include "error.h"


Error e;


//-----------------------------------------------------------------------------
// Create a heapfile
//-----------------------------------------------------------------------------
const Status createHeapFile(const string fileName)
{

    File* 		file;
    Status 		status;
    FileHdrPage*	hdrPage;
    int			hdrPageNo;
    int			newPageNo;
    Page*		newPage;

    // open the file, if exists, return FILEEXISTS
    status = db.openFile(fileName, file);

    // if not exist, do following 
    if (status != OK)
    {

	// create a new heapfile
	status = db.createFile(fileName);
	if (status != OK) return status;

	// open the created heapfile
        status = db.openFile(fileName, file);
	if (status != OK) return status;

	// allocate header page
	status = bufMgr->allocPage(file, hdrPageNo, newPage);
	if (status != OK) return status;
	hdrPage = (FileHdrPage *) newPage;

	// initialize header page values
	strcpy(hdrPage->fileName, fileName.c_str());
	hdrPage->firstPage = -1;
	hdrPage->lastPage = -1;
	hdrPage->pageCnt = 0;
	hdrPage->recCnt = 0;

	// allocate first data page
	status = bufMgr->allocPage(file, newPageNo, newPage);
	if (status != OK) return status;
	
	// initialize first data page values
	newPage->init(newPageNo);
	hdrPage->firstPage = newPageNo;
	hdrPage->lastPage = newPageNo;
	hdrPage->pageCnt++;

        // unpin header page and mark as dirty
	status = bufMgr->unPinPage(file, hdrPageNo, true);
	if (status != OK) return status;

	// unpin first data page and mark as dirty
	status = bufMgr->unPinPage(file, newPageNo, true);
	if (status != OK) return status;

	// write header page
        status = file->writePage(hdrPageNo, (Page*)hdrPage);
	if (status != OK) return status;

	// write first data page
        status = file->writePage(newPageNo, newPage);
	if (status != OK) return status;
        
        // close the heap file
        status = db.closeFile(file);
        if( status != OK) return status;

	return OK;

    }

    return (FILEEXISTS);

}


// routine to destroy a heapfile
const Status destroyHeapFile(const string fileName)
{
	return (db.destroyFile (fileName));
}


//-----------------------------------------------------------------------------
// Open the underlying file
//-----------------------------------------------------------------------------
HeapFile::HeapFile(const string & fileName, Status& returnStatus)
{

    Status 	status;
    Page*	pagePtr;

    // open the file, if successful, get and set header page, first data page
    if ((status = db.openFile(fileName, filePtr)) == OK)
    {

	// get the header page
	int pageNo = -1;
	status = filePtr->getFirstPage(pageNo);
	if (status != OK) 
	{
                returnStatus = status;
		return;
	}

        // read in header page
	status = bufMgr->readPage(filePtr, pageNo, pagePtr);
	if (status != OK) 
	{
                returnStatus = status;
		return;
	}

	// set header page
	headerPage = (FileHdrPage *) pagePtr;
	headerPageNo = pageNo;
	hdrDirtyFlag = false;

        // read in first data page
	status = bufMgr->readPage(filePtr, headerPage->firstPage, pagePtr);
	if (status != OK) 
	{
                returnStatus = status;
		return;
	}

	// set first data page
	curPage = pagePtr;
	curPageNo = headerPage->firstPage;
	curDirtyFlag = false;

	// set current record
	curRec = NULLRID;

        returnStatus = OK;

    }

    // if unsuccessful, set returnStatus and return
    else
    {
    	cerr << "open of heap file failed\n";
		returnStatus = status;
		return;
    }
    
}


// the destructor closes the file
HeapFile::~HeapFile()
{

    Status status;
    cout << "invoking heapfile destructor on file " 
         << headerPage->fileName << endl;

    // see if there is a pinned data page. If so, unpin it
    if (curPage != NULL)
    {
    	status = bufMgr->unPinPage(filePtr, curPageNo, curDirtyFlag);
	curPage = NULL;
	curPageNo = 0;
	curDirtyFlag = false;
	if (status != OK) cerr << "error in unpin of date page\n";
    }

    // unpin the header page
    status = bufMgr->unPinPage(filePtr, headerPageNo, hdrDirtyFlag);
    if (status != OK) cerr << "error in unpin of header page\n";

    // make sure all pages of the file are flushed to disk
    // status = bufMgr->flushFile(filePtr);
    // if (status != OK) cerr << "error in flushFile call\n";
    // before close the file
    status = db.closeFile(filePtr);
    if (status != OK)
    {
		cerr << "error in closefile call\n";
		Error e;
		e.print (status);
    }

}


// Return number of records in heap file
const int HeapFile::getRecCnt() const
{
  return headerPage->recCnt;
}


//-----------------------------------------------------------------------------
// Read record with a given RID from file
//-----------------------------------------------------------------------------
const Status HeapFile::getRecord(const RID &  rid, Record & rec)
{

    Status status;

    // if current page number is different from given rid.pageNo,
    // replace current page
    if (curPageNo != rid.pageNo) 
    {
	
	// unpin current page
	status = bufMgr->unPinPage(filePtr, curPageNo, curDirtyFlag);
	if (status != OK) return status;

	// read in the right page
	status = bufMgr->readPage(filePtr, rid.pageNo, curPage);
	if (status != OK) return status;

	// set the right page
	curPageNo = rid.pageNo;
	curDirtyFlag = false;

    }

    // get the record and set curRec
    status = curPage->getRecord(rid, rec);
    if (status != OK) return status;
    curRec = rid;

    return OK;

}


HeapFileScan::HeapFileScan(const string & name,
			   Status & status) : HeapFile(name, status)
{
    filter = NULL;
}


const Status HeapFileScan::startScan(const int offset_,
				     const int length_,
				     const Datatype type_,
				     const char* filter_,
				     const Operator op_)
{

    // no filtering requested
    if (!filter_) {                        
        filter = NULL;
        return OK;
    }

    if ((offset_ < 0 || length_ < 1) ||
        (type_ != STRING && type_ != INTEGER && type_ != FLOAT) ||
        ((type_ == INTEGER && length_ != sizeof(int))
         || (type_ == FLOAT && length_ != sizeof(float))) ||
        (op_ != LT && op_ != LTE && op_ != EQ && op_ != GTE && op_ != GT 
	&& op_ != NE))
    {
        return BADSCANPARM;
    }

    offset = offset_;
    length = length_;
    type = type_;
    filter = filter_;
    op = op_;

    return OK;

}


const Status HeapFileScan::endScan()
{

    Status status;

    // generally must unpin last page of the scan
    if (curPage != NULL)
    {
        status = bufMgr->unPinPage(filePtr, curPageNo, curDirtyFlag);
        curPage = NULL;
        curPageNo = 0;
		curDirtyFlag = false;
        return status;
    }

    return OK;

}


HeapFileScan::~HeapFileScan()
{
    endScan();
}


const Status HeapFileScan::markScan()
{
    // make a snapshot of the state of the scan
    markedPageNo = curPageNo;
    markedRec = curRec;
    return OK;
}


const Status HeapFileScan::resetScan()
{

    Status status;

    if (markedPageNo != curPageNo)
    {

	if (curPage != NULL)
	{
		status = bufMgr->unPinPage(filePtr, curPageNo, curDirtyFlag);
		if (status != OK) return status;
	}

	// restore curPageNo and curRec values
	curPageNo = markedPageNo;
	curRec = markedRec;

	// then read the page
	status = bufMgr->readPage(filePtr, curPageNo, curPage);
	if (status != OK) return status;
	// it will be clean
	curDirtyFlag = false; 

    }

    else curRec = markedRec;

    return OK;

}


//-----------------------------------------------------------------------------
// Return RID of next satisfied record
//-----------------------------------------------------------------------------
const Status HeapFileScan::scanNext(RID& outRid)
{

    Status 	status;
    RID		nextRid;
    RID		tmpRid;
    int 	nextPageNo;
    Record      rec;

    // start at curPage
    nextPageNo = curPageNo;

    // scan through until end of file
    while (true) {

	bool readFirstRecord = true;   // whether read first record
    	bool replaceCurPage = false;   // whether replace current page

	// if next page does not exist, break
	if (nextPageNo == -1) break;

	// if the page is in buffer pool, check curRec
	if (nextPageNo == curPageNo) {

		// if curRec is set, scan from curRec
		if (curRec.pageNo != -1 && curRec.slotNo != -1) {
			nextPageNo = curRec.pageNo;
			tmpRid = curRec;
			readFirstRecord = false;
			replaceCurPage = true;
		}
		
	}
	// otherwise, replace current page
	else
		replaceCurPage = true;

	// if replaceCurPage is true, replace current page
	if (replaceCurPage) {		

		// unpin current page
		status = bufMgr->unPinPage(filePtr, curPageNo, curDirtyFlag);
		if (status != OK) return status;

		// read in next page
		status = bufMgr->readPage(filePtr, nextPageNo, curPage);
		if (status != OK) return status;

		// set next page
		curPageNo = nextPageNo;
		curDirtyFlag = false;

	}

	// if readFirstRecord is true, scan from first record
	if (readFirstRecord) {

		// get first record, if not exist, jump to next page
		status = curPage->firstRecord(tmpRid);
		if (status == NORECORDS) {
			status = curPage->getNextPage(nextPageNo);
			if (status != OK) return status;
			continue;
		}

		// get record
		status = curPage->getRecord(tmpRid, rec);
		if (status != OK) return status;

		// if a matching record is found, set outRid and curRec
		if (matchRec(rec)) {
			curRec = tmpRid;
			outRid = tmpRid;
			return OK;
		}

	}

	// scan through until end of page
	while (curPage->nextRecord(tmpRid, nextRid) != ENDOFPAGE) {

		// get next record
		status = curPage->getRecord(nextRid, rec);
		if (status != OK) return status;
			
		if (matchRec(rec)) {
			curRec = nextRid;
			outRid = nextRid;
			return OK;
		}

		tmpRid = nextRid;
	}

	// get next page
	status = curPage->getNextPage(nextPageNo);
	if (status != OK) return status;	

    }

    return FILEEOF;

}


// returns pointer to the current record.  page is left pinned
// and the scan logic is required to unpin the page
const Status HeapFileScan::getRecord(Record & rec)
{
    return curPage->getRecord(curRec, rec);
}


// delete record from file.
const Status HeapFileScan::deleteRecord()
{

    Status status;

    // delete the "current" record from the page
    status = curPage->deleteRecord(curRec);
    curDirtyFlag = true;

    // reduce count of number of records in the file
    headerPage->recCnt--;
    hdrDirtyFlag = true;
    return status;

}


// mark current page of scan dirty
const Status HeapFileScan::markDirty()
{
    curDirtyFlag = true;
    return OK;
}


const bool HeapFileScan::matchRec(const Record & rec) const
{

    // no filtering requested
    if (!filter) return true;

    // see if offset + length is beyond end of record
    // maybe this should be an error???
    if ((offset + length -1 ) >= rec.length)
	return false;

    // < 0 if attr < fltr
    float diff = 0;                       
    switch(type) {

    case INTEGER:
        int iattr, ifltr;
	// word-alignment problem possible
        memcpy(&iattr,
               (char *)rec.data + offset,
               length);
        memcpy(&ifltr,
               filter,
               length);
        diff = iattr - ifltr;
        break;

    case FLOAT:
        float fattr, ffltr;
	// word-alignment problem possible
        memcpy(&fattr,
               (char *)rec.data + offset,
               length);
        memcpy(&ffltr,
               filter,
               length);
        diff = fattr - ffltr;
        break;

    case STRING:
        diff = strncmp((char *)rec.data + offset,
                       filter,
                       length);
        break;
    }

    switch(op) {
    case LT:  if (diff < 0.0) return true; break;
    case LTE: if (diff <= 0.0) return true; break;
    case EQ:  if (diff == 0.0) return true; break;
    case GTE: if (diff >= 0.0) return true; break;
    case GT:  if (diff > 0.0) return true; break;
    case NE:  if (diff != 0.0) return true; break;
    }

    return false;

}


InsertFileScan::InsertFileScan(const string & name,
                               Status & status) : HeapFile(name, status)
{
  //Do nothing. Heapfile constructor will read the header page and the first
  // data page of the file into the buffer pool
}


InsertFileScan::~InsertFileScan()
{

    Status status;

    // unpin last page of the scan
    if (curPage != NULL)
    {
        status = bufMgr->unPinPage(filePtr, curPageNo, true);
        curPage = NULL;
        curPageNo = 0;
        if (status != OK) cerr << "error in unpin of data page\n";
    }

}


//-----------------------------------------------------------------------------
// Insert a record into the file
//-----------------------------------------------------------------------------
const Status InsertFileScan::insertRecord(const Record & rec, RID& outRid)
{

    Page*	newPage;
    int		newPageNo;
    Status	status;
    RID		rid;

    // check for very large records
    if ((unsigned int) rec.length > PAGESIZE-DPFIXED)
    {
        // will never fit on a page, so don't even bother looking
        return INVALIDRECLEN;
    }

    // if current page is not the last page, replace current page
    if (curPageNo != headerPage->lastPage) 
    {
	
	// unpin current page
	status = bufMgr->unPinPage(filePtr, curPageNo, curDirtyFlag);
	if (status != OK) return status;

	// read in last page
	status = bufMgr->readPage(filePtr, headerPage->lastPage, curPage);
	if (status != OK) return status;

	// set last page
	curPageNo = headerPage->lastPage;
	curDirtyFlag = false;

    }

    // insert record
    status = curPage->insertRecord(rec, rid);

    // if not enough space available, allocate a new page and insert
    if (status == NOSPACE) {
        
	// allocate a new page
	status = bufMgr->allocPage(filePtr, newPageNo, newPage);
	if (status != OK) return status;

	// update new page and header page
	newPage->init(newPageNo);
	headerPage->lastPage = newPageNo;
	headerPage->pageCnt++;
        curPage->setNextPage(newPageNo);
	hdrDirtyFlag = true;
        curDirtyFlag = true;

	// unpin current page
	status = bufMgr->unPinPage(filePtr, curPageNo, curDirtyFlag);
	if (status != OK) return status;

	// set new page
	curPage = newPage;
	curPageNo = newPageNo;
	curDirtyFlag = false;

	// insert record and set dirty flag
	curPage->insertRecord(rec, rid);
	curDirtyFlag = true;

    }

    // set outRid and update header page
    outRid = rid;
    headerPage->recCnt++;
    hdrDirtyFlag = true;

    return OK;

}
