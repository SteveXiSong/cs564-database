/******************************************************************************
Group member:  Jing Jing
UW netID:      jjing2

Group member:  Xi Song
UW netID:      xsong44

Course:        CS 564, Fall 2014
Assignment:    Programming Assignment 5 Front-End and DB Utilities
******************************************************************************/


#include <sys/types.h>
#include <functional>
#include <string.h>
#include <stdio.h>

using namespace std;

#include "error.h"
#include "utility.h"
#include "catalog.h"


//-----------------------------------------------------------------------------
// RelCatalog::help: print catalog information (relation may be an empty string)
//-----------------------------------------------------------------------------
const Status RelCatalog::help(const string & relation)
{

  Status status;
  RelDesc rd;
  AttrDesc *attrs;
  int attrCnt;
  HeapFileScan *scan;
  RID rid;
  Record rec;

  if (relation.empty()) 
      return UT_Print(RELCATNAME);

  // get the relation information
  status = getInfo(relation, rd);
  if (status != OK) return status;
  attrCnt = rd.attrCnt;

  // print the relation information
  printf("Relation name: %s (%d attributes)\n", rd.relName, attrCnt);
  printf("  %-17s%-6s%-4s%s\n", "Attribute name", "Off", "T", "Len");
  printf("---------------- ----- --- -----\n");

  // create a HeapFileScan on the attribute catalog
  scan = new HeapFileScan(ATTRCATNAME, status);
  if (status != OK) return status;
  
  // start the scan with equality operation on the relation name
  scan->startScan(0, MAXNAME, STRING, relation.c_str(), EQ);
  // call scanNext to get the attribute information
  while ((status = scan->scanNext(rid)) != FILEEOF) {
      // get the record
      status = scan->getRecord(rec);
      // print the attribute information
      if (status == OK) {
	  attrs = (AttrDesc* ) rec.data;
	  printf("  %*s", 14, attrs->attrName);
          printf("%*d", 6, attrs->attrOffset);
          switch(attrs->attrType) {
	      case INTEGER:
		   printf("%*s", 4, "i");
		   break;
	      case FLOAT:
		   printf("%*s", 4, "f");
		   break;
	      default:
		   printf("%*s", 4, "s");
		   break;
          }
      	  printf("%*d\n", 6, attrs->attrLen);
      }
  }

  // end the scan and delete the scan object
  scan->endScan();
  delete scan;

  return OK;

}
