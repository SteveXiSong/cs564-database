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
// RelCatalog::destroyRel: destroy a relation
//-----------------------------------------------------------------------------
const Status RelCatalog::destroyRel(const string & relation)
{

  Status status;
  RelDesc rd;

  if (relation.empty() || 
      relation == string(RELCATNAME) || 
      relation == string(ATTRCATNAME))
      return BADCATPARM;

  // get information about the relation from RelCatalog
  status = getInfo(relation, rd);
  if (status != OK) return status;

  // drop the relation from AttrCatalog
  status = attrCat->dropRelation(relation);
  if (status != OK) return status;

  // remove information corresponding to relation from RelCatalog
  status = removeInfo(relation);
  if (status != OK) return status;

  // destroy the heap file
  status = destroyHeapFile(relation);
  if (status != OK) return status;

  return OK;

}


//-----------------------------------------------------------------------------
// AttrCatalog::dropRelation: delete all information about a relation
//-----------------------------------------------------------------------------
const Status AttrCatalog::dropRelation(const string & relation)
{

  Status status;
  AttrDesc *attrs;
  int attrCnt;

  if (relation.empty()) 
      return BADCATPARM;

  // get the relation information from the AttrCatalog
  status = getRelInfo(relation, attrCnt, attrs);
  if (status != OK) return status;

  // remove information corresponding to each attribute from AttrCatalog
  for (int i = 0; i < attrCnt; i++)
      removeInfo(attrs[i].relName, attrs[i].attrName);

  delete[] attrs;

  return OK;

}
