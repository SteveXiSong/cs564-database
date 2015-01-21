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
// QU_Insert: insert a record into the specified relation
//-----------------------------------------------------------------------------
const Status QU_Insert(const string & relation, 
	const int attrCnt, 
	const attrInfo attrList[])
{

  Status status;
  RelDesc rd;
  AttrDesc* attrs;
  Record rec;
  RID rid;
  int attributes;
  InsertFileScan* scan;

  // get information corresponding to relation from RelCatalog
  status = attrCat->getRelInfo(relation, attributes, attrs);
  if (status != OK) return status;

  // get total length of the attributes
  int length = 0;
  for (int i = 0; i < attrCnt; i++)
	length += attrs[i].attrLen;
  
  char* records = new char[length];   // new record for added attribute

  for (int i = 0; i < attrCnt; i++) {

      // check if added attribute is null, return ATTRNULL if it is
      if (attrList[i].attrValue == NULL) {
	    delete[] records;
	    return ATTRNULL;
      }

      // for each attribute, add data to corresponding offset
      for (int j = 0; j < attrCnt; j++) {
	  // match attribute name to get the offset
          if (strcmp(attrList[i].attrName, attrs[j].attrName) == 0) {
              // copy over the data part based on the type
	      switch (attrs[j].attrType) {
	          case INTEGER:
		       int tempi;
		       tempi = atoi((char*) attrList[i].attrValue);
		       memcpy(&records[attrs[j].attrOffset], &tempi, 
                              attrs[j].attrLen);
		       break;
		  case FLOAT:
		       float tempf;
		       tempf = atof((char*)  attrList[i].attrValue);
		       memcpy(&records[attrs[j].attrOffset], &tempf, 
                              attrs[j].attrLen);
		       break;
		  default:
		       memcpy(&records[attrs[j].attrOffset], attrList[i].attrValue,
                              attrs[j].attrLen);
		       break;
	      }
	  }
      }

  }

  // set the record
  rec.data = (void*) records;
  rec.length = length;

  // create an InsertFileScan on the relation 
  scan = new InsertFileScan(relation, status);
  if (status != OK) return status;

  // insert the record into the file
  status = scan->insertRecord(rec, rid);
  if (status != OK) return status;

  delete[] records;
  delete[] attrs;
  delete scan;

  return OK;

}
