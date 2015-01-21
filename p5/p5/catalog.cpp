/******************************************************************************
Group member:  Jing Jing
UW netID:      jjing2

Group member:  Xi Song
UW netID:      xsong44

Course:        CS 564, Fall 2014
Assignment:    Programming Assignment 5 Front-End and DB Utilities
******************************************************************************/


#include "catalog.h"


//-----------------------------------------------------------------------------
// RelCatalog::constructor: open relation catalog
//-----------------------------------------------------------------------------
RelCatalog::RelCatalog(Status &status) :
	 HeapFile(RELCATNAME, status)
{
  // nothing should be needed here
}


//-----------------------------------------------------------------------------
// RelCatalog::getInfo: get relation descriptor for a relation
//-----------------------------------------------------------------------------
const Status RelCatalog::getInfo(const string & relation, RelDesc &record)
{

  Status status;
  Record rec;
  RID rid;
  HeapFileScan* scan;

  if (relation.empty())
      return BADCATPARM;

  // create a HeapFileScan on the relation catalog
  scan = new HeapFileScan(RELCATNAME, status);
  if (status != OK) return status;
  
  // start the scan with equality operation on the relation name
  scan->startScan(0, MAXNAME, STRING, relation.c_str(), EQ);
  // call scanNext to get the corresponding record
  while ((status = scan->scanNext(rid)) != FILEEOF) {
      // get the record
      status = scan->getRecord(rec);
      if (status == OK) {
          // copy over the data part
 	  memcpy(&record, rec.data, rec.length);
	  scan->endScan();
  	  delete scan;
          return OK;
      }
  }

  // end the scan and delete the scan object
  scan->endScan();
  delete scan;

  // return RELNOTFOUND if no relation is found
  return RELNOTFOUND;

}


//-----------------------------------------------------------------------------
// RelCatalog::addInfo: add information to catalog
//-----------------------------------------------------------------------------
const Status RelCatalog::addInfo(RelDesc & record)
{

  RID rid;
  InsertFileScan*  ifs;
  Status status;
  Record rec;

  // create an InsertFileScan on the relation catalog
  ifs = new InsertFileScan(RELCATNAME, status);
  if (status != OK) return status;

  // set the RelDesc
  rec.data = (void*) &record;
  rec.length = sizeof record;

  // insert the record into the file
  status = ifs->insertRecord(rec, rid);
  if (status != OK) return status;

  // delete the scan object
  delete ifs;

  return OK;

}


//-----------------------------------------------------------------------------
// RelCatalog::removeInfo: remove tuple from catalog
//-----------------------------------------------------------------------------
const Status RelCatalog::removeInfo(const string & relation)
{

  Status status;
  RID rid;
  HeapFileScan*  hfs;

  if (relation.empty()) 
      return BADCATPARM;

  // create a HeapFileScan on the relation catalog
  hfs = new HeapFileScan(RELCATNAME, status);
  if (status != OK) return status;
  
  // start the scan with equality operation on the relation name
  hfs->startScan(0, MAXNAME, STRING, relation.c_str(), EQ);
  // call scanNext to get the corresponding record
  while ((status = hfs->scanNext(rid)) != FILEEOF) {
      // delete the record
      status = hfs->deleteRecord();
      if (status != OK) return status;
  }

  // end the scan and delete the scan object
  hfs->endScan();
  delete hfs;

  return OK;

}


//-----------------------------------------------------------------------------
// RelCatalog::destructor: get rid of catalog
//-----------------------------------------------------------------------------
RelCatalog::~RelCatalog()
{
  // nothing should be needed here
}


//-----------------------------------------------------------------------------
// AttrCatalog::constructor: open attribute catalog
//-----------------------------------------------------------------------------
AttrCatalog::AttrCatalog(Status &status) :
	 HeapFile(ATTRCATNAME, status)
{
  // nothing should be needed here
}


//-----------------------------------------------------------------------------
// AttrCatalog::getInfo: get attribute catalog tuple
//-----------------------------------------------------------------------------
const Status AttrCatalog::getInfo(const string & relation, 
				  const string & attrName,
				  AttrDesc &record)
{

  Status status;
  RID rid;
  Record rec;
  HeapFileScan*  hfs;

  if (relation.empty() || attrName.empty()) 
      return BADCATPARM;

  // create a HeapFileScan on the attribute catalog
  hfs = new HeapFileScan(ATTRCATNAME, status);
  if (status != OK) return status;
  
  // start the scan with equality operation on the relation name
  hfs->startScan(0, MAXNAME, STRING, relation.c_str(), EQ);
  // call scanNext to get the corresponding record
  while ((status = hfs->scanNext(rid)) != FILEEOF) {
      // get the record
      status = hfs->getRecord(rec);
      if (status == OK) {
	  AttrDesc* attribute = (AttrDesc* ) rec.data;
	  // compare the attribute name to the given name
	  if (strcmp(attribute->attrName, attrName.c_str()) == 0) {
	      // copy the data part
	      memcpy(&record, rec.data, rec.length);
	      hfs->endScan();
  	      delete hfs;
              return OK;
	  }
      }
  }

  // end the scan and delete the scan object
  hfs->endScan();
  delete hfs;

  // return ATTRNOTFOUND if no attribute is found
  return ATTRNOTFOUND;

}


//-----------------------------------------------------------------------------
// AttrCatalog::addInfo: add information to catalog
//-----------------------------------------------------------------------------
const Status AttrCatalog::addInfo(AttrDesc & record)
{

  RID rid;
  InsertFileScan*  ifs;
  Status status;
  Record rec;
  AttrDesc ad;

  // create an InsertFileScan on the attribute catalog
  ifs = new InsertFileScan(ATTRCATNAME, status);
  if (status != OK) return status;

  // set the AttrDesc
  rec.data = (void*) &record;
  rec.length = sizeof record;

  // insert the record into the file
  status = ifs->insertRecord(rec, rid);
  if (status != OK) return status;

  // delete the scan object
  delete ifs;

  return OK;

}


//-----------------------------------------------------------------------------
// AttrCatalog::removeInfo: get all attributes of a relation
//-----------------------------------------------------------------------------
const Status AttrCatalog::removeInfo(const string & relation, 
			       const string & attrName)
{

  Status status;
  Record rec;
  RID rid;
  HeapFileScan*  hfs;

  if (relation.empty() || attrName.empty()) 
      return BADCATPARM;

  // create a HeapFileScan on the attribute catalog
  hfs = new HeapFileScan(ATTRCATNAME, status);
  if (status != OK) return status;
  
  // start the scan with equality operation on the relation name
  hfs->startScan(0, MAXNAME, STRING, relation.c_str(), EQ);
  // call scanNext to get the corresponding record
  while ((status = hfs->scanNext(rid)) != FILEEOF) {
      // get the record
      status = hfs->getRecord(rec);
      if (status == OK) {
	  AttrDesc* attribute = (AttrDesc* ) rec.data;
          // compare the attribute name to the given name
	  if (strcmp(attribute->attrName, attrName.c_str()) == 0) {
              // delete the record
	      status = hfs->deleteRecord();
	      if (status != OK) return status;
	  }
      }
  }

  // end the scan and delete the scan object
  hfs->endScan();
  delete hfs;

  return OK;

}


//-----------------------------------------------------------------------------
// AttrCatalog::getRelInfo: get all attributes of a relation
//-----------------------------------------------------------------------------
const Status AttrCatalog::getRelInfo(const string & relation, 
				     int &attrCnt,
				     AttrDesc *&attrs)
{

  Status status;
  RID rid;
  Record rec;
  RelDesc rd;
  HeapFileScan*  hfs;

  if (relation.empty()) 
      return BADCATPARM;

  // get information corresponding to relation from RelCatalog
  status = relCat->getInfo(relation, rd);
  if (status != OK) return status;

  // create a HeapFileScan on the attribute catalog
  hfs = new HeapFileScan(ATTRCATNAME, status);
  if (status != OK) return status;

  int index = 0;                                     // index of attributes 
  AttrDesc* attributes = new AttrDesc[rd.attrCnt];   // list of attributes

  // start the scan with equality operation on the relation name
  hfs->startScan(0, MAXNAME, STRING, relation.c_str(), EQ);
  // call scanNext to get the corresponding record
  while ((status = hfs->scanNext(rid)) != FILEEOF) {
      // get the record
      status = hfs->getRecord(rec);
      if (status == OK) {
          // copy over the data part
	  memcpy(&attributes[index], rec.data, rec.length);
	  index++;
      }
  }
  

  attrCnt = rd.attrCnt;
  attrs = attributes;

  // end the scan and delete the scan object
  hfs->endScan();
  delete hfs;

  return OK;

}


//-----------------------------------------------------------------------------
// AttrCatalog::destructor: close attribute catalog
//-----------------------------------------------------------------------------
AttrCatalog::~AttrCatalog()
{
  // nothing should be needed here
}
