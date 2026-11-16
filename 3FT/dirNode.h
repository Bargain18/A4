/*--------------------------------------------------------------------*/
/* dirNode.h                                                          */
/* Authors: Michael Wei, Lakshit Ahuja                                */
/*--------------------------------------------------------------------*/
#ifndef DIRECTORYFILETREES_DIRNODE_H
#define DIRECTORYFILETREES_DIRNODE_H

#include <stddef.h>
#include "a4def.h"
#include "path.h"
#include "fileNode.h"

/* A DirNode_T is a dirNode in a File Tree */
typedef struct dirNode *DirNode_T;

/*
  Creates a new dirNode in the File Tree, with path oPPath and
  parent oDNParent. Returns an int SUCCESS status and sets *poDNResult
  to be the new node if successful. Otherwise, sets *poDNResult to NULL
  and returns status:
  * MEMORY_ERROR if memory could not be allocated to complete request
  * CONFLICTING_PATH if oDNParent's path is not an ancestor of oPPath
  * NO_SUCH_PATH if oPPath is of depth 0
                 or oDNParent's path is not oPPath's direct parent
                 or oDNParent is NULL but oPPath is not of depth 1
  * ALREADY_IN_TREE if oDNParent already has a child with this path
*/
int DirNode_new(Path_T oPPath, DirNode_T oDNParent, DirNode_T *poDNResult);

/*
  Destroys and frees all memory allocated for the subtree rooted at
  oDNNode, i.e., deletes this node and all its descendents. Returns the
  number of dirNodes and fileNodes deleted.
*/
size_t DirNode_free(DirNode_T oDNNode);

/* Returns the path object representing oDNNode's absolute path. */
Path_T DirNode_getPath(DirNode_T oDNNode);

/*
  Returns TRUE if oDNParent has a child with path oPPath. Returns
  FALSE if it does not.

  If oDNParent has such a child, stores in *pulChildID the child's
  identifier (as used in DirNode_getChild). If oDNParent does not have
  such a child, stores in *pulChildID the identifier that such a
  child _would_ have if inserted.
*/
boolean DirNode_hasChild(DirNode_T oDNParent, Path_T oPPath,
                         size_t *pulChildID);

/* Returns the number of children that oNParent has. */
size_t DirNode_getNumChildren(DirNode_T oDNParent);

/*
  Returns an int SUCCESS status and sets *poDNResult to be the child
  node of oDNParent with identifier ulChildID, if one exists.
  Otherwise, sets *poDNResult to NULL and returns status:
  * NO_SUCH_PATH if ulChildID is not a valid child for oDNParent
*/
int DirNode_getChild(DirNode_T oNParent, size_t ulChildID,
                  DirNode_T *poDNResult);

boolean DirNode_hasFile(DirNode_T oDNParent, Path_T oPPath,
                         size_t *pulFileID);

size_t DirNode_getNumFiles(DirNode_T oDNParent);

int DirNode_getFile(DirNode_T oDNParent, size_t ulFileID,
                  FileNode_T *poFNResult);

/*
  Returns a the parent node of oDNNode.
  Returns NULL if oDNNode is the root and thus has no parent.
*/
DirNode_T DirNode_getParent(DirNode_T oDNNode);

/*
  Compares oDNFirst and oDNSecond lexicographically based on their paths.
  Returns <0, 0, or >0 if oDNFirst is "less than", "equal to", or
  "greater than" oDNSecond, respectively.
*/
int DirNode_compare(DirNode_T oDNFirst, DirNode_T oDNSecond);

/*
  Returns a string representation for oDNNode, or NULL if
  there is an allocation error.

  Allocates memory for the returned string, which is then owned by
  the caller!
*/
char *DirNode_toString(DirNode_T oDNNode);

#endif