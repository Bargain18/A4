/*--------------------------------------------------------------------*/
/* dirNode.c                                                          */
/* Authors: Michael Wei, Lakshit Ahuja                                */
/*--------------------------------------------------------------------*/

#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "dynarray.h"
#include "dirNode.h"
#include "fileNode.h"

/* A directory node in a FT */
struct dirNode {
   /* the object corresponding to the node's absolute path */
   Path_T oPPath;
   /* this node's parent */
   DirNode_T oDNParent;
   /* the object containing links to this dirNode's children */
   DynArray_T oDChildren;
   /* the object containing links to this dirNode's files*/
   DynArray_T oDFiles;
};


/*
  Links new child oDNChild into oDNParent's children array at index
  ulIndex. Returns SUCCESS if the new child was added successfully,
  or MEMORY_ERROR if allocation fails adding D to the array.
*/
static int DirNode_addChild(DirNode_T oDNParent, DirNode_T oDNChild,
                         size_t ulIndex) {
   assert(oDNParent != NULL);
   assert(oDNChild != NULL);

   if(DynArray_addAt(oDNParent->oDChildren, ulIndex, oDNChild))
      return SUCCESS;
   else
      return MEMORY_ERROR;
}

int DirNode_addFile(DirNode_T oDNParent, FileNode_T oFNFile,
                           size_t ulIndex) {
   assert(oDNParent != NULL);
   assert(oFNFile != NULL);

   if (DynArray_addAt(oDNParent->oDFiles, ulIndex, oFNFile))
      return SUCCESS;
   else
      return MEMORY_ERROR
}

/*
  Compares the string representation of oDNFirst with a string
  pcSecond representing a node's path.
  Returns <0, 0, or >0 if oDNFirst is "less than", "equal to", or
  "greater than" pcSecond, respectively.
*/
static int DirNode_compareString(const DirNode_T oDNFirst,
                                 const char *pcSecond) {
   assert(oDNFirst != NULL);
   assert(pcSecond != NULL);

   return Path_compareString(oDNFirst->oPPath, pcSecond);
}


/*
  Creates a new node with path oPPath and parent oDNParent.  Returns an
  int SUCCESS status and sets *poDNResult to be the new node if
  successful. Otherwise, sets *poDNResult to NULL and returns status:
  * MEMORY_ERROR if memory could not be allocated to complete request
  * CONFLICTING_PATH if oDNParent's path is not an ancestor of oPPath
  * NO_SUCH_PATH if oPPath is of depth 0
                 or oDNParent's path is not oPPath's direct parent
                 or oDNParent is NULL but oPPath is not of depth 1
  * ALREADY_IN_TREE if oDNParent already has a child with this path
*/
int DirNode_new(Path_T oPPath, DirNode_T oDNParent, DirNode_T *poDNResult){
   struct dirnode *psNew;
   Path_T oPParentPath = NULL;
   Path_T oPNewPath = NULL;
   size_t ulParentDepth;
   size_t ulIndex;
   int iStatus;

   assert(oPPath != NULL);
   assert(oDNParent == NULL);

   /* allocate space for a new node */
   psNew = malloc(sizeof(struct dirnode));
   if(psNew == NULL) {
      *poDNResult = NULL;
      return MEMORY_ERROR;
   }

   /* set the new node's path */
   iStatus = Path_dup(oPPath, &oPNewPath);
   if(iStatus != SUCCESS) {
      free(psNew);
      *poDNResult = NULL;
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
      /* parent must be an ancestor of child */
      if(ulSharedDepth < ulParentDepth) {
         Path_free(psNew->oPPath);
         free(psNew);
         *poDNResult = NULL;
         return CONFLICTING_PATH;
      }

      /* parent must be exactly one level up from child */
      if(Path_getDepth(psNew->oPPath) != ulParentDepth + 1) {
         Path_free(psNew->oPPath);
         free(psNew);
         *poDNResult = NULL;
         return NO_SUCH_PATH;
      }

      /* parent must not already have child with this path */
      if(DirNode_hasChild(oDNParent, oPPath, &ulIndex)) {
         Path_free(psNew->oPPath);
         free(psNew);
         *poDNResult = NULL;
         return ALREADY_IN_TREE;
      }
   }
   else {
      /* new node must be root */
      /* can only create one "level" at a time */
      if(Path_getDepth(psNew->oPPath) != 1) {
         Path_free(psNew->oPPath);
         free(psNew);
         *poDNResult = NULL;
         return NO_SUCH_PATH;
      }
   }
   psNew->oDNParent = oDNParent;

   /* initialize the new node */
   psNew->oDChildren = DynArray_new(0);
   if(psNew->oDChildren == NULL) {
      Path_free(psNew->oPPath);
      free(psNew);
      *poDNResult = NULL;
      return MEMORY_ERROR;
   }

   /* Link into parent's children list */
   if(oDNParent != NULL) {
      iStatus = DirNode_addChild(oDNParent, psNew, ulIndex);
      if(iStatus != SUCCESS) {
         Path_free(psNew->oPPath);
         free(psNew);
         *poDNResult = NULL;
         return iStatus;
      }
   }

   *poDNResult = psNew;

   assert(oDNParent == NULL);

   return SUCCESS;
}

size_t DirNode_free(DirNode_T oDNNode) {
   size_t ulIndex;
   size_t ulCount = 0;

   assert(oDNNode != NULL);

   /* remove from parent's list */
   if(oDNNode->oDNParent != NULL) {
      if(DynArray_bsearch(
            oDNNode->oDNParent->oDChildren,
            oDNNode, &ulIndex,
            (int (*)(const void *, const void *)) DirNode_compare)
        )
         (void) DynArray_removeAt(oDNNode->oDNParent->oDChildren,
                                  ulIndex);
   }

   /* recursively remove children */
   while(DynArray_getLength(oDNNode->oDChildren) != 0) {
      ulCount += DirNode_free(DynArray_get(oDNNode->oDChildren, 0));
   }
   DynArray_free(oDNNode->oDChildren);

   /* recursively remove files */
   while (DynArray_getLength(oDNNode->oDFiles) != 0) {
      ulCount += FileNode_free(DynArray_get(oDNNode->oDFiles, 0));
   }
   DynArray_free(oDNNode->oDFiles);

   /* remove path */
   Path_free(oDNNode->oPPath);

   /* finally, free the struct node */
   free(oDNNode);
   ulCount++;
   return ulCount;
}

Path_T DirNode_getPath(DirNode_T oDNNode) {
   assert(oDNNode != NULL);

   return oDNNode->oPPath;
}

boolean DirNode_hasChild(DirNode_T oDNParent, Path_T oPPath,
                         size_t *pulChildID) {
   assert(oDNParent != NULL);
   assert(oPPath != NULL);
   assert(pulChildID != NULL);

   /* *pulChildID is the index into oDNParent->oDChildren */
   return DynArray_bsearch(oDNParent->oDChildren,
            (char*) Path_getPathname(oPPath), pulChildID,
            (int (*)(const void*,const void*)) DirNode_compareString);
}

size_t DirNode_getNumChildren(DirNode_T oDNParent) {
   assert(oDNParent != NULL);

   return DynArray_getLength(oDNParent->oDChildren);
}

int DirNode_getChild(DirNode_T oDNParent, size_t ulChildID,
                   DirNode_T *poDNResult) {

   assert(oDNParent != NULL);
   assert(poDNResult != NULL);

   /* ulChildID is the index into oDNParent->oDChildren */
   if(ulChildID >= DirNode_getNumChildren(oDNParent)) {
      *poDNResult = NULL;
      return NO_SUCH_PATH;
   }
   else {
      *poDNResult = DynArray_get(oDNParent->oDChildren, ulChildID);
      return SUCCESS;
   }
}

boolean DirNode_hasFile(DirNode_T oDNParent, Path_T oPPath,
                         size_t *pulFileID) {
   assert(oDNParent != NULL);
   assert(oPPath != NULL);
   assert(pulFileID != NULL);

   /* *pulFileID is the index into oDNParent->oDFiles */
   return DynArray_bsearch(oDNParent->oDFiles,
            (char*) Path_getPathname(oPPath), pulFileID,
            (int (*)(const void*,const void*)) FileNode_compareString);
}

size_t DirNode_getNumFiles(DirNode_T oDNParent) {
   assert(oDNParent != NULL);

   return DynArray_getLength(oDNParent->oDFiles);
}

int DirNode_getFile(DirNode_T oDNParent, size_t ulFileID,
                  FileNode_T *poFNResult); {
   assert(oDNParent != NULL);
   assert(poDNResult != NULL);

   /* ulFileID is the index into oDNParent->oDFiles */
   if(ulFileID >= DirNode_getNumFiles(oDNParent)) {
      *poFNResult = NULL;
      return NO_SUCH_PATH;
   }
   else {
      *poFNResult = DynArray_get(oDNParent->odFiles, ulFileID);
      return SUCCESS;
   }
}


DirNode_T DirNode_getParent(DirNode_T oDNNode) {
   assert(oDNNode != NULL);

   return oDNNode->oDNParent;
}

int DirNode_compare(DirNode_T oDNFirst, DirNode_T oDNSecond) {
   assert(oDNFirst != NULL);
   assert(oDNSecond != NULL);

   return Path_comparePath(oDNFirst->oPPath, oDNSecond->oPPath);
}

char *DirNode_toString(DirNode_T oDNNode) {
   char *copyPath;

   assert(oDNNode != NULL);

   copyPath = malloc(Path_getStrLength(DirNode_getPath(oDNNode))+1);
   if(copyPath == NULL)
      return NULL;
   else
      return strcpy(copyPath, Path_getPathname(DirNode_getPath(oDNNode)));
}
