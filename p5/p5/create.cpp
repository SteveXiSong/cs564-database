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
// RelCatalog::createRel: create a new relation
//-----------------------------------------------------------------------------
const Status RelCatalog::createRel(const string & relation, 
				   const int attrCnt,
				   const attrInfo attrList[])
{

  Status status;
  RelDesc rd;
  AttrDesc ad;
  Error e;

  if (relation.empty() || attrCnt < 1)
      return BADCATPARM;

  if (relation.length() >= sizeof rd.relName)
      return NAMETOOLONG;

  // check if the relation exists, return RELEXISTS if it does
  status = getInfo(relation, rd);
  if (status != RELNOTFOUND) return RELEXISTS;

  // check if the sum of length of attributes exceeds page size,
  // return RELTOOLONG if it does
  int length = 0;
  for (int i = 0; i < attrCnt; i++)
      length += attrList[i].attrLen;
  if (length > PAGESIZE)
      return RELTOOLARGE;

  // check if duplicate attributes exist, return DUPLATTR if it does
  for (int i = 0; i < attrCnt; i++)
      for (int j = 0; j < attrCnt; j++)
	  if (j != i)
	      if (strcmp(attrList[i].attrName, attrList[j].attrName) == 0)
	          return DUPLATTR;

  // create a RelDesc
  strcpy(rd.relName, relation.c_str());
  rd.attrCnt = attrCnt;

  // add it to the RelCatalog
  status = addInfo(rd);
  if (status != OK) return status;

  // create a AttrDesc for each attribute and add it to the AttrCatalog
  int offset = 0;
  for (int i = 0; i < attrCnt; i++) {
      strcpy(ad.relName, attrList[i].relName);
      strcpy(ad.attrName, attrList[i].attrName);
      ad.attrOffset = offset;
      offset += attrList[i].attrLen;
      ad.attrType = attrList[i].attrType;
      ad.attrLen = attrList[i].attrLen;
      status = attrCat->addInfo(ad);
      if (status != OK) return status;
  }

  // create a heap file
  status = createHeapFile(relation);
  if (status != OK) return status;

  return OK;

}
