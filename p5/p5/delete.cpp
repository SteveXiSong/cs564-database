/******************************************************************************
Group member:  Jing Jing
UW netID:      jjing2

Group member:  Xi Song
UW netID:      xsong44

Course:        CS 564, Fall 2014
Assignment:    Programming Assignment 5 Front-End and DB Utilities
******************************************************************************/


#include "catalog.h"
#include "query.h"
#include <stdlib.h>


//-----------------------------------------------------------------------------
// QU_Delete: delete records from a specified relation
//-----------------------------------------------------------------------------
const Status QU_Delete(const string & relation, 
		       const string & attrName, 
		       const Operator op,
		       const Datatype type, 
		       const char *attrValue)
{

  Status status;
  Record rec;
  RID rid;
  AttrDesc ad;
  HeapFileScan* scan;

  if (relation.empty()) 
      return BADCATPARM;

  // if WHERE clause is empty, delete all records in the relation
  if (attrName.empty()) {

      // create a HeapFileScan on the relation
      scan = new HeapFileScan(relation, status);
      if (status != OK) return status;

      // start the scan
      scan->startScan(0, 0, STRING, NULL, EQ);
      // call scanNext to get the corresponding record
      while ((status = scan->scanNext(rid)) != FILEEOF) {
	  // delete the record
          status = scan->deleteRecord();
	  if (status != OK) return status;
      }

  }

  // otherwise, delete corresponding records
  else {

      // get attribute information
      status = attrCat->getInfo(relation, attrName, ad);
      if (status != OK) return status;

      // create a HeapFileScan on the relation
      scan = new HeapFileScan(relation, status);
      if (status != OK) return status;
  
      // start the scan with equality operation on the record data
      // and the data type
      switch (type) {
          case INTEGER:
	       int tempi;
	       tempi = atoi(attrValue);
	       scan->startScan(ad.attrOffset, ad.attrLen, type, 
                               (char*) &tempi, op);
	       break;
	  case FLOAT:
	       float tempf;
	       tempf = atof(attrValue);
	       scan->startScan(ad.attrOffset, ad.attrLen, type, 
                               (char*) &tempf, op);
	       break;
	  default:
	       scan->startScan(ad.attrOffset, ad.attrLen, type, 
                                attrValue, op);
	       break;
      }

      // call scanNext to get the corresponding record
      while ((status = scan->scanNext(rid)) != FILEEOF) {
          // delete the record
          status = scan->deleteRecord();
	  if (status != OK) return status;
      }

  }

  // end the scan and delete the scan object
  scan->endScan();
  delete scan;

  return OK;

}
