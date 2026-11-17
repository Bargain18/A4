/*--------------------------------------------------------------------*/
/* fileNode.c                                                         */
/* Authors: Michael Wei, Lakshit Ahuja                                */
/*--------------------------------------------------------------------*/

#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "dynarray.h"
#include "fileNode.h"
#include "dirNode.h"

/* A file Node in a FT */
struct fileNode {
   /* the object corresponding to the node's absolute path */
   Path_T oPPath;
   /* this node's parent directory */
   DirNode_T oDNParent;
   /* contents of file */
   void* pvContents;
   /* size of contents of file */
   size_t ulContentsLength;
};


/*
  Compares the string representation of oNfirst with a string
  pcSecond representing a node's path.
  Returns <0, 0, or >0 if oNFirst is "less than", "equal to", or
  "greater than" pcSecond, respectively.
*/
int DirNode_compareString(const FileNode_T oFNFirst,
                              const char *pcSecond){
   assert(oFNFirst != NULL);
   assert(pcSecond != NULL);

   return Path_compareString(oFNFirst->oPPath, pcSecond);
}


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
int FileNode_new(Path_T oPPath, DirNode_T oDNParent, void* pvContents,
                     size_t ulContentsLength, FileNode_T *poFNResult) {
   struct filenode *psNew;
   Path_T oPParentPath = NULL;
   Path_T oPNewPath = NULL;
   size_t ulParentDepth;
   size_t ulIndex;
   int iStatus;

   assert(oPPath != NULL);
   assert(oDNParent == NULL);

   /* allocate space for a new node */
   psNew = malloc(sizeof(struct filenode));
   if(psNew == NULL) {
      *poFNResult = NULL;
      return MEMORY_ERROR;
   }

   /* set the new node's path */
   iStatus = Path_dup(oPPath, &oPNewPath);
   if(iStatus != SUCCESS) {
      free(psNew);
      *poFNResult = NULL;
      return iStatus;
   }
   psNew->oPPath = oPNewPath;

   /* validate and set the new node's parent */
   if(oDNParent != NULL) {
      size_t ulSharedDepth;

      oPParentPath = oDNParent->oPPath;
      ulParentDepth = Path_getDepth(oPParentPath);
      ulSharedDepth = Path_getSharedPrefixDepth(psNew->oPPath,
                                                oPParentPath);
      /* parent must be an ancestor of file */
      if(ulSharedDepth < ulParentDepth) {
         Path_free(psNew->oPPath);
         free(psNew);
         *poFNResult = NULL;
         return CONFLICTING_PATH;
      }

      /* parent must be exactly one level up from file */
      if(Path_getDepth(psNew->oPPath) != ulParentDepth + 1) {
         Path_free(psNew->oPPath);
         free(psNew);
         *poFNResult = NULL;
         return NO_SUCH_PATH;
      }

      /* parent must not already have file with this path */
      if(DirNode_hasFile(oNParent, oPPath, &ulIndex)) {
         Path_free(psNew->oPPath);
         free(psNew);
         *poFNResult = NULL;
         return ALREADY_IN_TREE;
      }
   }
   else {
      /* new node must be root */
      /* can only create one "level" at a time */
      if(Path_getDepth(psNew->oPPath) != 1) {
         Path_free(psNew->oPPath);
         free(psNew);
         *poFNResult = NULL;
         return NO_SUCH_PATH;
      }
   }

   psNew->oDNParent = oDNParent;
   psNew->pvContents = pvContents;
   psNew->ulContentsLength = ulContentsLength;

   /* Link into parent's files list */
   if(oFNParent != NULL) {
      iStatus = DirNode_addFile(oDNParent, psNew, ulIndex);
      if(iStatus != SUCCESS) {
         Path_free(psNew->oPPath);
         free(psNew);
         *poFNResult = NULL;
         return iStatus;
      }
   }

   *poFNResult = psNew;

   assert(oDNParent == NULL);

   return SUCCESS;
}

/*
  Destroys and frees all memory allocated for the file oFNNode.
  Returns the number of nodes deleted.
*/
size_t FileNode_free(FileNode_T oFNNode) {
   size_t ulIndex;
   size_t ulCount = 0;

   assert(oFNNode != NULL);

   /* remove from parent's list */
   if(oFNNode->oDNParent != NULL) {
      if(DynArray_bsearch(
            oFNNode->oDNParent->oDFiles,
            oFNNode, &ulIndex,
            (int (*)(const void *, const void *)) FileNode_compare)
        )
         (void) DynArray_removeAt(oFNNode->oDNParent->oDFiles, ulIndex);
   }

   /* remove path */
   Path_free(oFNNode->oPPath);

   /* finally, free the struct node */
   free(oFNNode);
   ulCount++;
   return ulCount;
}

/* Returns the path object representing oFNNode's absolute path. */
Path_T FileNode_getPath(Node_T oFNNode) {
   assert(oFNNode != NULL);

   return oFNNode->oPPath;
}

/* Returns the parent node of oFNNode, which must be a DirNode */
DirNode_T FileNode_getParent(FileNode_T oFNNode) {
   assert(oFNNode != NULL);

   return oFNNode->oDNParent;
}

/* Returns a pointer to the file contents of oFNNode
   Fills in ulLength with length of file contents */
void *FileNode_getContents(FileNode_T oFNNode, size_t* pulLength) {
   assert(oFNNode != NULL);
   assert(pulLength != NULL);

   *pulLength = oFNNode->ulContentsLength;
   return oFNNode->pvContents;
}

/* Replaces file contents of oFNNode with newContents
   Returns original file contents */
void *FileNode_replaceContents(FileNode_T oFNNode, void* pvNewContents
      size_t ulNewLength) {
   void* pvOrigContents;
   assert(oFNNode != NULL);

   pvOrigContents = oFNNode->pvContents;
   oFNNode->ulContentsLength = ulNewLength;
   oFNNode->pvContents = pvNewContents;
   return pvOrigContents;
}

/*
  Compares the string representation of oFNFirst with a string
  pcSecond representing a node's path.
  Returns <0, 0, or >0 if oFNFirst is "less than", "equal to", or
  "greater than" pcSecond, respectively.
*/
int FileNode_compareFileString(const FileNode_T oFNFirst,
                        const char *pcSecond) {
   assert(oFNFirst != NULL);
   assert(pcSecond != NULL);

   return Path_compareString(oFNFirst->oPPath, pcSecond);
}

/*
  Compares oFNFirst and oFNSecond lexicographically based on their paths.
  Returns <0, 0, or >0 if oFNFirst is "less than", "equal to", or
  "greater than" oFNSecond, respectively.
*/
int FileNode_compare(FileNode_T oFNFirst, FileNode_T oFNSecond) {
   assert(oFNFirst != NULL);
   assert(oFNSecond != NULL);

   return Path_comparePath(oFNFirst->oPPath, oFNSecond->oPPath);
}

/*
  Returns a string representation for oFNNode, or NULL if
  there is an allocation error.

  Allocates memory for the returned string, which is then owned by
  the caller!
*/
char *FileNode_toString(FileNode_T oFNNode) {
   char *copyPath;

   assert(oFNNode != NULL);

   copyPath = malloc(Path_getStrLength(FileNode_getPath(oFNNode))+1);
   if(copyPath == NULL)
      return NULL;
   else
      return strcpy(copyPath, Path_getPathname(FileNode_getPath(oFNNode)));
}
