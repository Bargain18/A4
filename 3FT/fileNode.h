/*--------------------------------------------------------------------*/
/* fileNode.h                                                         */
/* Authors: Michael Wei, Lakshit Ahuja                                */
/*--------------------------------------------------------------------*/
#ifndef DIRECTORYFILETREES_FILENODE_H
#define DIRECTORYFILETREES_FILENODE_H

#include <stddef.h>
#include "a4def.h"
#include "path.h"
#include "fileNode.h"

/* A FileNode_T is a filenode in a File Tree */
typedef struct filenode *FileNode_T;

/*
  Creates a new fileNode in the File Tree, with path oPPath and
  parent oDNParent. Returns an int SUCCESS status and sets *poFNResult
  to be the new node if successful. Otherwise, sets *poFNResult to NULL
  and returns status:
  * MEMORY_ERROR if memory could not be allocated to complete request
  * CONFLICTING_PATH if oFNParent's path is not an ancestor of oPPath
  * NO_SUCH_PATH if oPPath is of depth 0
                 or oDNParent's path is not oPPath's direct parent
                 or oDNParent is NULL but oPPath is not of depth 1
  * ALREADY_IN_TREE if oDNParent already has a file with this path
*/
int FileNode_new(Path_T oPPath, DirNode_T oDNParent, Node_T *poFNResult);

/*
  Destroys and frees all memory allocated for the subtree rooted at
  oNNode, i.e., deletes this node and all its descendents. Returns the
  number of nodes deleted, which is always one.
*/
size_t FileNode_free(FileNode_T oFNNode);

/* Returns the path object representing oFNNode's absolute path. */
Path_T FileNode_getPath(Node_T oFNNode);

/* Returns the parent node of oFNNode. */
DirNode_T FileNode_getParent(FileNode_T oFNNode);

/* Returns a pointer to the file contents of oFNNode */
void *FileNode_getContents(FileNode_T oFNNode, size_t* pulLength);

/* Replaces file contents of oFNNode with newContents
   Returns original file contents */
void *FileNode_replaceContents(FileNode_T oFNNode, void* pvNewContents
    size_t ulNewLength);

/*
  Compares the string representation of oFNFirst with a string
  pcSecond representing a node's path.
  Returns <0, 0, or >0 if oFNFirst is "less than", "equal to", or
  "greater than" pcSecond, respectively.
*/
int FileNode_compareString(const FileNode_T oFNFirst,
                        const char *pcSecond);

/*
  Compares oFNFirst and oFNSecond lexicographically based on their paths.
  Returns <0, 0, or >0 if oFNFirst is "less than", "equal to", or
  "greater than" oFNSecond, respectively.
*/
int FileNode_compare(FileNode_T oFNFirst, FileNode_T oFNSecond);

/*
  Returns a string representation for oFNNode, or NULL if
  there is an allocation error.

  Allocates memory for the returned string, which is then owned by
  the caller!
*/
char *FileNode_toString(FileNode_T oFNNode);

/*
  Compares the string representation of oFNfirst with a string
  pcSecond representing a node's path.
  Returns <0, 0, or >0 if oNFirst is "less than", "equal to", or
  "greater than" pcSecond, respectively.
*/
int FileNode_compareString(const FileNode_T oFNFirst, const char *pcSecond);

#endif