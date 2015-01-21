/******************************************************************************
Group member:  Jing Jing
UW netID:      jjing2

Group member:  Xi Song
UW netID:      xsong44

Course:        CS 564, Fall 2014
Assignment:    Programming Assignment 5 Front-End and DB Utilities
******************************************************************************/

#include <unistd.h>
#include <fcntl.h>
#include "catalog.h"
#include "utility.h"

//
// Loads a file of (binary) tuples from a standard file into the relation.
// Any indices on the relation are updated appropriately.
//
// Returns:
// 	OK on success
// 	an error code otherwise
//

const Status UT_Load(const string & relation, const string & fileName)
{
  Status status;
  RelDesc rd;
  AttrDesc *attrs;
  int attrCnt;
  InsertFileScan * iFile;
  int width = 0;

  if (relation.empty() || fileName.empty() || relation == string(RELCATNAME)
      || relation == string(ATTRCATNAME))
    return BADCATPARM;

  // open Unix data file
  int fd;
  if ((fd = open(fileName.c_str(), O_RDONLY, 0)) < 0)
    return UNIXERR;

  // get information corresponding to the relation from the relation catalog
  status = relCat->getInfo(relation, rd);
  if (status != OK) return status;

  // get relation information from the attribute catalog
  status = attrCat->getRelInfo(relation, attrCnt, attrs);
  if (status != OK) return status;

  // calculate the width of a record
  for (int i = 0; i < attrCnt; i++) 
      width += attrs[i].attrLen;

  // start insertFileScan on relation
  iFile = new InsertFileScan(relation, status);

  // allocate buffer to hold record read from unix file
  char *record;
  if (!(record = new char [width])) return INSUFMEM;

  int records = 0;
  int nbytes;
  Record rec;

  // read next input record from Unix file and insert it into relation
  while((nbytes = read(fd, record, width)) == width) {
    RID rid;
    rec.data = record;
    rec.length = width;
    if ((status = iFile->insertRecord(rec, rid)) != OK) return status;
    records++;
  }

  // close heap file and unix file
  if (close(fd) < 0) return UNIXERR;

  delete[] attrs;
  delete[] record;
  delete iFile;

  return OK;
}

