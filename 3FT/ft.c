/*--------------------------------------------------------------------*/
/* ft.c                                                               */
/* Authors: Michael Wei, Lakshit Ahuja                                */
/*--------------------------------------------------------------------*/

#include <stddef.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "dynarray.h"
#include "path.h"
#include "fileNode.h"
#include "dirNode.h"
#include "ft.h"

/*
  A  Tree is a representation of a hierarchy of directories and files,
  represented as an AO with 3 state variables:
*/

/* 1. a flag for being in an initialized state (TRUE) or not (FALSE) */
static boolean bIsInitialized;
/* 2. a pointer to the root node in the hierarchy,
      which must be a Directory */
static DirNode_T oDNRoot;
/* 3. a counter of the number of nodes in the hierarchy */
static size_t ulCount;


/*
  Traverses the FT via DirNodes starting at the root as far as
  possible towards absolute path oPPath. If able to traverse,
  returns an int SUCCESS status and sets *poDNFurthest to the
  furthest DirNode reached (which may be only a prefix of oPPath,
  or even NULL if the root is NULL). Otherwise, sets *poDNFurthest
  to NULL and returns with status:
  * CONFLICTING_PATH if the root's path is not a prefix of oPPath
  * MEMORY_ERROR if memory could not be allocated to complete request
*/
static int FT_traverseDirPath(Path_T oPPath, DirNode_T *poDNFurthest) {
   int iStatus;
   Path_T oPPrefix = NULL;
   DirNode_T oDNCurr;
   DirNode_T oDNChild = NULL;
   size_t ulDepth;
   size_t i;
   size_t ulChildID;

   assert(oPPath != NULL);
   assert(poDNFurthest != NULL);

   /* root is NULL -> won't find anything */
   if(oDNRoot == NULL) {
      *poDNFurthest = NULL;
      return SUCCESS;
   }

   iStatus = Path_prefix(oPPath, 1, &oPPrefix);
   if(iStatus != SUCCESS) {
      *poDNFurthest = NULL;
      return iStatus;
   }

   if(Path_comparePath(DirNode_getPath(oDNRoot), oPPrefix)) {
      Path_free(oPPrefix);
      *poDNFurthest = NULL;
      return CONFLICTING_PATH;
   }
   Path_free(oPPrefix);
   oPPrefix = NULL;

   oDNCurr = oDNRoot;
   ulDepth = Path_getDepth(oPPath);
   for(i = 2; i <= ulDepth; i++) {
      iStatus = Path_prefix(oPPath, i, &oPPrefix);
      if(iStatus != SUCCESS) {
         *poDNFurthest = NULL;
         return iStatus;
      }
      if(DirNode_hasChild(oDNCurr, oPPrefix, &ulChildID)) {
         /* go to that child and continue with next prefix */
         Path_free(oPPrefix);
         oPPrefix = NULL;
         iStatus = DirNode_getChild(oDNCurr, ulChildID, &oDNChild);
         if(iStatus != SUCCESS) {
            *poDNFurthest = NULL;
            return iStatus;
         }
         oDNCurr = oDNChild;
      }
      else {
         /* oDNCurr doesn't have child with path oPPrefix:
            this is as far as we can go */
         break;
      }
   }

   Path_free(oPPrefix);
   *poDNFurthest = oDNCurr;
   return SUCCESS;
}

/*
  Traverses the FT to find a DirNode with absolute path pcPath. Returns a
  int SUCCESS status and sets *poDNResult to be the node, if found.
  Otherwise, sets *poDNResult to NULL and returns with status:
  * INITIALIZATION_ERROR if the FT is not in an initialized state
  * BAD_PATH if pcPath does not represent a well-formatted path
  * CONFLICTING_PATH if the root's path is not a prefix of pcPath
  * NO_SUCH_PATH if no DirNode with pcPath exists in the hierarchy
  * MEMORY_ERROR if memory could not be allocated to complete request
 */
static int FT_findDirNode(const char *pcPath, DirNode_T *poDNResult) {
   Path_T oPPath = NULL;
   DirNode_T oDNFound = NULL;
   int iStatus;

   assert(pcPath != NULL);
   assert(poDNResult != NULL);

   if(!bIsInitialized) {
      *poDNResult = NULL;
      return INITIALIZATION_ERROR;
   }

   iStatus = Path_new(pcPath, &oPPath);
   if(iStatus != SUCCESS) {
      *poDNResult = NULL;
      return iStatus;
   }

   iStatus = FT_traverseDirPath(oPPath, &oDNFound);
   if(iStatus != SUCCESS)
   {
      Path_free(oPPath);
      *poDNResult = NULL;
      return iStatus;
   }

   if(oDNFound == NULL) {
      Path_free(oPPath);
      *poDNResult = NULL;
      return NO_SUCH_PATH;
   }

   if(Path_comparePath(DirNode_getPath(oDNFound), oPPath) != 0) {
      Path_free(oPPath);
      *poDNResult = NULL;
      return NO_SUCH_PATH;
   }

   Path_free(oPPath);
   *poDNResult = oDNFound;
   return SUCCESS;
}

/*
  Traverses the FT to find a FileNode with absolute path pcPath. Returns a
  int SUCCESS status and sets *poFNResult to be the node, if found.
  Otherwise, sets *poFNResult to NULL and returns with status:
  * INITIALIZATION_ERROR if the FT is not in an initialized state
  * BAD_PATH if pcPath does not represent a well-formatted path
  * CONFLICTING_PATH if the root's path is not a prefix of pcPath
  * NO_SUCH_PATH if no FileNode with pcPath exists in the hierarchy
  * MEMORY_ERROR if memory could not be allocated to complete request
 */
static int FT_findFileNode(const char *pcPath, FileNode_T *poFNResult) {
   Path_T oPPath = NULL;
   FileNode_T oFNFound = NULL;
   DirNode_T oDNClosest = NULL;
   int iStatus;
   boolean bInside;
   size_t ulFileID;

   assert(pcPath != NULL);
   assert(poFNResult != NULL);

   if(!bIsInitialized) {
      *poFNResult = NULL;
      return INITIALIZATION_ERROR;
   }

   iStatus = Path_new(pcPath, &oPPath);
   if(iStatus != SUCCESS) {
      *poFNResult = NULL;
      return iStatus;
   }

   iStatus = FT_traverseDirPath(oPPath, &oDNClosest);
   if(iStatus != SUCCESS)
   {
      Path_free(oPPath);
      *poFNResult = NULL;
      return iStatus;
   }

   bInside = DirNode_hasFile(oDNClosest, oPPath, &ulFileID);
   if(!bInside) {
      Path_free(oPPath);
      *poFNResult = NULL;
      return NO_SUCH_PATH;
   }

   iStatus = DirNode_getFile(oDNClosest, ulFileID, &oFNFound);
   if(iStatus != SUCCESS) {
      Path_free(oPPath);
      *poFNResult = NULL;
      return iStatus;
   }

   if(Path_comparePath(FileNode_getPath(oFNFound), oPPath) != 0) {
      Path_free(oPPath);
      *poDNResult = NULL;
      return NO_SUCH_PATH;
   }

   if(oFNFound == NULL) {
      Path_free(oPPath);
      *poFNResult = NULL;
      return NO_SUCH_PATH;
   }

   Path_free(oPPath);
   *poFNResult = oFNFound;
   return SUCCESS;
}

/*
   Inserts a new directory into the FT with absolute path pcPath.
   Returns SUCCESS if the new directory is inserted successfully.
   Otherwise, returns:
   * INITIALIZATION_ERROR if the FT is not in an initialized state
   * BAD_PATH if pcPath does not represent a well-formatted path
   * CONFLICTING_PATH if the root exists but is not a prefix of pcPath
   * NOT_A_DIRECTORY if a proper prefix of pcPath exists as a file
   * ALREADY_IN_TREE if pcPath is already in the FT (as dir or file)
   * MEMORY_ERROR if memory could not be allocated to complete request
*/
int FT_insertDir(const char *pcPath) {
   int iStatus;
   Path_T oPPath = NULL;
   DirNode_T oDNFirstNew = NULL;
   DirNode_T oDNCurr = NULL;
   size_t ulDepth, ulIndex;
   size_t ulNewNodes = 0;

   assert(pcPath != NULL);

   /* validate pcPath and generate a Path_T for it */
   if(!bIsInitialized)
      return INITIALIZATION_ERROR;

   iStatus = Path_new(pcPath, &oPPath);
   if(iStatus != SUCCESS)
      return iStatus;

   /* find the closest ancestor of oPPath already in the tree */
   iStatus= FT_traverseDirPath(oPPath, &oDNCurr);
   if(iStatus != SUCCESS)
   {
      Path_free(oPPath);
      return iStatus;
   }

   /* no ancestor node found, so if root is not NULL,
      pcPath isn't underneath root. */
   if(oDNCurr == NULL && oDNRoot != NULL) {
      Path_free(oPPath);
      return CONFLICTING_PATH;
   }

   ulDepth = Path_getDepth(oPPath);
   if(oDNCurr == NULL) /* new root! */
      ulIndex = 1;
   else {
      ulIndex = Path_getDepth(DirNode_getPath(oDNCurr))+1;

      /* oDNCurr is the node we're trying to insert */
      if(ulIndex == ulDepth+1 && !Path_comparePath(oPPath,
                                       DirNode_getPath(oDNCurr))) {
         Path_free(oPPath);
         return ALREADY_IN_TREE;
      }
   }

   /* starting at oDNCurr, build rest of the path one level at a time */
   while(ulIndex <= ulDepth) {
      Path_T oPPrefix = NULL;
      DirNode_T oDNNewNode = NULL;
      boolean isFile;
      size_t ulFileID;

      /* generate a Path_T for this level */
      iStatus = Path_prefix(oPPath, ulIndex, &oPPrefix);
      if(iStatus != SUCCESS) {
         Path_free(oPPath);
         if(oDNFirstNew != NULL)
            (void) DirNode_free(oDNFirstNew);
         return iStatus;
      }

      /* check if the generated Path_T is a file */
      isFile = DirNode_hasFile(oDNCurr, oPPrefix, &ulFileID);
      if (isFile) {
         Path_free(oPPath);
         Path_free(oPPrefix);
         if (oDNFirstNew != NULL)
            (void) DirNode_free(oDNFirstNew);
         /* Proper prefix existing means a file exists along path */
         if (ulIndex < ulDepth)
            return NOT_A_DIRECTORY;
         /* However, if full path is file, this is a different error*/
         else
            return ALREADY_IN_TREE;
      }

      /* insert the new node for this level */
      iStatus = DirNode_new(oPPrefix, oDNCurr, &oDNNewNode);
      if(iStatus != SUCCESS) {
         Path_free(oPPath);
         Path_free(oPPrefix);
         if(oDNFirstNew != NULL)
            (void) DirNode_free(oDNFirstNew);
         return iStatus;
      }

      /* set up for next level */
      Path_free(oPPrefix);
      oDNCurr = oDNNewNode;
      ulNewNodes++;
      if(oDNFirstNew == NULL)
         oDNFirstNew = oDNCurr;
      ulIndex++;
   }

   Path_free(oPPath);
   /* update FT state variables to reflect insertion */
   if(oDNRoot == NULL)
      oDNRoot = oDNFirstNew;
   ulCount += ulNewNodes;

   return SUCCESS;
}

/*
  Returns TRUE if the FT contains a directory with absolute path
  pcPath and FALSE if not or if there is an error while checking.
*/
boolean FT_containsDir(const char *pcPath) {
   int iStatus;
   DirNode_T oDNFound = NULL;

   assert(pcPath != NULL);

   iStatus = FT_findDirNode(pcPath, &oDNFound);
   return (boolean) (iStatus == SUCCESS);
}

/*
  Removes the FT hierarchy (subtree) at the directory with absolute
  path pcPath. Returns SUCCESS if found and removed.
  Otherwise, returns:
  * INITIALIZATION_ERROR if the FT is not in an initialized state
  * BAD_PATH if pcPath does not represent a well-formatted path
  * CONFLICTING_PATH if the root exists but is not a prefix of pcPath
  * NO_SUCH_PATH if absolute path pcPath does not exist in the FT
  * NOT_A_DIRECTORY if pcPath is in the FT as a file not a directory
  * MEMORY_ERROR if memory could not be allocated to complete request
*/
int FT_rmDir(const char *pcPath) {
   int iStatus;
   DirNode_T oDNFound = NULL;

   assert(pcPath != NULL);

   iStatus = FT_findDirNode(pcPath, &oDNFound);
   if(iStatus != SUCCESS)
      return iStatus;

   ulCount -= DirNode_free(oDNFound);
   if(ulCount == 0)
      oDNRoot = NULL;

   return SUCCESS;
}


/*
   Inserts a new file into the FT with absolute path pcPath, with
   file contents pvContents of size ulLength bytes.
   Returns SUCCESS if the new file is inserted successfully.
   Otherwise, returns:
   * INITIALIZATION_ERROR if the FT is not in an initialized state
   * BAD_PATH if pcPath does not represent a well-formatted path
   * CONFLICTING_PATH if the root exists but is not a prefix of pcPath,
                      or if the new file would be the FT root
   * NOT_A_DIRECTORY if a proper prefix of pcPath exists as a file
   * ALREADY_IN_TREE if pcPath is already in the FT (as dir or file)
   * MEMORY_ERROR if memory could not be allocated to complete request
*/
int FT_insertFile(const char *pcPath, void *pvContents,
                  size_t ulLength) {
   int iStatus;
   Path_T oPPath = NULL;
   FileNode_T oFNResult;
   DirNode_T oDNFirstNew = NULL;
   DirNode_T oDNCurr = NULL;
   size_t ulDepth, ulIndex;
   size_t ulNewNodes = 0;

   assert(pcPath != NULL);

   /* validate pcPath and generate a Path_T for it */
   if(!bIsInitialized)
      return INITIALIZATION_ERROR;

   iStatus = Path_new(pcPath, &oPPath);
   if(iStatus != SUCCESS)
      return iStatus;

   /* find the closest ancestor of oPPath already in the tree */
   iStatus= FT_traverseDirPath(oPPath, &oDNCurr);
   if(iStatus != SUCCESS)
   {
      Path_free(oPPath);
      return iStatus;
   }

   /* no ancestor node found, so if root is not NULL,
   pcPath isn't underneath root. */
   if(oDNCurr == NULL && oDNRoot != NULL) {
      Path_free(oPPath);
      return CONFLICTING_PATH;
   }

   ulDepth = Path_getDepth(oPPath);
   /* no ancestor root found and path has depth 1,
      file cannot be root  */
   if(oDNCurr == NULL && ulDepth == 1) {
      Path_free(oPPath);
      return CONFLICTING_PATH;
   }

   if(oDNCurr == NULL) /* new root! */
      ulIndex = 1;
   else {
      ulIndex = Path_getDepth(DirNode_getPath(oDNCurr))+1;

      /* oDNCurr is the node we're trying to insert */
      if(ulIndex == ulDepth+1 && !Path_comparePath(oPPath,
                                       DirNode_getPath(oDNCurr))) {
         Path_free(oPPath);
         return ALREADY_IN_TREE;
                                       }
   }

   /* starting at oDNCurr, build rest of the path one level at a time */
   while(ulIndex < ulDepth) {
      Path_T oPPrefix = NULL;
      DirNode_T oDNNewNode = NULL;
      boolean isFile;
      size_t ulID;

      /* generate a Path_T for this level */
      iStatus = Path_prefix(oPPath, ulIndex, &oPPrefix);
      if(iStatus != SUCCESS) {
         Path_free(oPPath);
         if(oDNFirstNew != NULL)
            (void) DirNode_free(oDNFirstNew);
         return iStatus;
      }

      /* check if the generated Path_T is a file */
      isFile = DirNode_hasFile(oDNCurr, oPPrefix, &ulID);
      if (isFile) {
         Path_free(oPPath);
         Path_free(oPPrefix);
         if (oDNFirstNew != NULL)
            (void) DirNode_free(oDNFirstNew);
         /* Proper prefix existing means a file exists along path */
         if (ulIndex < ulDepth)
            return NOT_A_DIRECTORY;
         /* However, if full path is file, this is a different error*/
         else
            return ALREADY_IN_TREE;
      }

      /* insert the new node for this level */
      iStatus = DirNode_new(oPPrefix, oDNCurr, &oDNNewNode);
      if(iStatus != SUCCESS) {
         Path_free(oPPath);
         Path_free(oPPrefix);
         if(oDNFirstNew != NULL)
            (void) DirNode_free(oDNFirstNew);
         return iStatus;
      }

      /* set up for next level */
      Path_free(oPPrefix);
      oDNCurr = oDNNewNode;
      ulNewNodes++;
      if(oDNFirstNew == NULL)
         oDNFirstNew = oDNCurr;
      ulIndex++;
   }

   /* at final level, path is simply oPPath */
   /* check if file or directory already exists */
   if (DirNode_hasChild(oDNCurr, oPPath, &ulID) ||
      DirNode_hasFile(oDNCurr, oPPath, &ulID)) {
      Path_free(oPPath);
      if(oDNFirstNew != NULL)
         (void) DirNode_free(oDNFirstNew);
      return ALREADY_IN_TREE;
   }

   /* Insert new file at final level */
   iStatus = FileNode_new(oPPath, oDNCurr, pvContents, ulLength, &oFNResult);
   if(iStatus != SUCCESS) {
      Path_free(oPPath);
      if(oDNFirstNew != NULL)
         (void) DirNode_free(oDNFirstNew);
      return iStatus;
   }

   ulNewNodes++;
   Path_free(oPPath);
   /* update DT state variables to reflect insertion */
   if(oDNRoot == NULL)
      oDNRoot = oDNFirstNew;
   ulCount += ulNewNodes;

   return SUCCESS;
}

/*
  Returns TRUE if the FT contains a file with absolute path
  pcPath and FALSE if not or if there is an error while checking.
*/
boolean FT_containsFile(const char *pcPath) {
   int iStatus;
   FileNode_T oFNFound = NULL;

   assert(pcPath != NULL);

   iStatus = FT_findFileNode(pcPath, &oFNFound);
   return (boolean) (iStatus == SUCCESS);
}

/*
  Removes the FT file with absolute path pcPath.
  Returns SUCCESS if found and removed.
  Otherwise, returns:
  * INITIALIZATION_ERROR if the FT is not in an initialized state
  * BAD_PATH if pcPath does not represent a well-formatted path
  * CONFLICTING_PATH if the root exists but is not a prefix of pcPath
  * NO_SUCH_PATH if absolute path pcPath does not exist in the FT
  * NOT_A_FILE if pcPath is in the FT as a directory not a file
  * MEMORY_ERROR if memory could not be allocated to complete request
*/
int FT_rmFile(const char *pcPath) {
   int iStatus;
   FileNode_T oFNFound = NULL;

   assert(pcPath != NULL);

   iStatus = FT_findFileNode(pcPath, &oFNFound);
   if(iStatus != SUCCESS)
      return iStatus;

   ulCount -= FileNode_free(oFNFound);

   return SUCCESS;
}

/*
  Returns the contents of the file with absolute path pcPath.
  Returns NULL if unable to complete the request for any reason.

  Note: checking for a non-NULL return is not an appropriate
  contains check, because the contents of a file may be NULL.
*/
void *FT_getFileContents(const char *pcPath) {
   int iStatus;
   size_t ulLength;
   FileNode_T oFNFound = NULL;

   assert(pcPath != NULL);

   iStatus = FT_findFileNode(pcPath, &oFNFound);
   if(iStatus != SUCCESS)
      return NULL;

   return FileNode_getContents(oFNFound, &ulLength);
}

/*
  Replaces current contents of the file with absolute path pcPath with
  the parameter pvNewContents of size ulNewLength bytes.
  Returns the old contents if successful. (Note: contents may be NULL.)
  Returns NULL if unable to complete the request for any reason.
*/
void *FT_replaceFileContents(const char *pcPath, void *pvNewContents,
                             size_t ulNewLength) {
   int iStatus;
   FileNode_T oFNFound = NULL;

   assert(pcPath != NULL);

   iStatus = FT_findFileNode(pcPath, &oFNFound);
   if(iStatus != SUCCESS)
      return iStatus;

   return FileNode_replaceContents(oFNFound, pvNewContents, ulNewLength);
}

/*
  Returns SUCCESS if pcPath exists in the hierarchy,
  Otherwise, returns:
  * INITIALIZATION_ERROR if the FT is not in an initialized state
  * BAD_PATH if pcPath does not represent a well-formatted path
  * CONFLICTING_PATH if the root's path is not a prefix of pcPath
  * NO_SUCH_PATH if absolute path pcPath does not exist in the FT
  * MEMORY_ERROR if memory could not be allocated to complete request

  When returning SUCCESS,
  if path is a directory: sets *pbIsFile to FALSE, *pulSize unchanged
  if path is a file: sets *pbIsFile to TRUE, and
                     sets *pulSize to the length of file's contents

  When returning another status, *pbIsFile and *pulSize are unchanged.
*/
int FT_stat(const char *pcPath, boolean *pbIsFile, size_t *pulSize) {
    Path_T oPPath = NULL;
	DirNode_T oDNClosest = NULL;
	int iStatus;
	boolean bFile;
	boolean bDir;
	size_t ulFileID;

    assert(pcPath != NULL);
    assert(poFNResult != NULL);

    if(!bIsInitialized) {
        *poFNResult = NULL;
        return INITIALIZATION_ERROR;
    }

	iStatus = Path_new(pcPath, &oPPath);
	if(iStatus != SUCCESS) {
  		*poFNResult = NULL;
  		return iStatus;
	}

   	iStatus = FT_traverseDirPath(oPPath, &oDNClosest);
   	if(iStatus != SUCCESS)
   	{
		Path_free(oPPath);
		*poFNResult = NULL;
		return iStatus;
   	}

	bDir = Path_comparePath(DirNode_getPath(oDNFound), oPPath) == 0;
	if(bDir) {
		*pbIsFile = FALSE;
		Path_free(oPPath);
        return SUCCESS;
	}

   	bFile = DirNode_hasFile(oDNClosest, oPPath, &ulFileID);
   	if(bFile) {
		*pbIsFile = TRUE;
		FT_getFileContents(pcPath, pulSize);
   		Path_free(oPPath);
        return SUCCESS;
   	}

	Path_free(oPPath);
    return NO_SUCH_PATH;
}

/*
  Sets the FT data structure to an initialized state.
  The data structure is initially empty.
  Returns INITIALIZATION_ERROR if already initialized,
  and SUCCESS otherwise.
*/
int FT_init(void) {
   if(bIsInitialized)
      return INITIALIZATION_ERROR;

   bIsInitialized = TRUE;
   oDNRoot = NULL;
   ulCount = 0;

   return SUCCESS;
}

/*
  Removes all contents of the data structure and
  returns it to an uninitialized state.
  Returns INITIALIZATION_ERROR if not already initialized,
  and SUCCESS otherwise.
*/
int FT_destroy(void) {
   if(!bIsInitialized)
      return INITIALIZATION_ERROR;

   if(oDNRoot) {
      ulCount -= DirNode_free(oDNRoot);
      oDNRoot = NULL;
   }

   bIsInitialized = FALSE;

   return SUCCESS;
}

/*
  Performs a pre-order traversal of the tree rooted at oDNNode,
  inserting each payload to DynArray_T d beginning at index i.
  Returns the next unused index in d after the insertion(s).
*/
static size_t FT_preOrderTraversal(DirNode_T oDNNode, DynArray_T d, size_t i) {
   	size_t c;

   	assert(d != NULL);

   	if(oDNNode != NULL) {
		int iStatus;
		char* str = NULL;
		str = DirNode_toString(oDNNode);
      	(void) DynArray_set(d, i, str);
      	i++;

		for(c = 0; c < DirNode_getNumFiles(oDNNode); c++) {
			FileNode_T oFNFile = NULL;
			str = NULL;
			iStatus = DirNode_getFile(oDNNode, c, &oFNFile);
			assert(iStatus == SUCCESS);
			str = FileNode_toString(oFNFile);
			(void) DynArray_set(d, i, str);
			i++;
		}

      	for(c = 0; c < DirNode_getNumChildren(oDNNode); c++) {
         	DirNode_T oDNChild = NULL;
         	iStatus = DirNode_getChild(oDNNode, c, &oDNChild);
         	assert(iStatus == SUCCESS);
         	i = FT_preOrderTraversal(oDNChild, d, i);
      	}
   	}
   	return i;
}

/*
  Alternate version of strlen that uses pulAcc as an in-out parameter
  to accumulate a string length, rather than returning the length of
  oNNode's path, and also always adds one addition byte to the sum.
*/
static void FT_strlenAccumulate(char* str, size_t *pulAcc) {
   assert(pulAcc != NULL);

   if(str != NULL)
      *pulAcc += (strlen(str) + 1);
}

/*
  Alternate version of strcat that inverts the typical argument
  order, appending oNNode's path onto pcAcc, and also always adds one
  newline at the end of the concatenated string.
*/
static void FT_strcatAccumulate(char* str, char *pcAcc) {
   assert(pcAcc != NULL);

   if(str != NULL) {
      strcat(pcAcc, str);
      strcat(pcAcc, "\n");
   }
}

/*
  Returns a string representation of the
  data structure, or NULL if the structure is
  not initialized or there is an allocation error.

  The representation is depth-first with files
  before directories at any given level, and nodes
  of the same type ordered lexicographically.

  Allocates memory for the returned string,
  which is then owned by client!
*/
char *FT_toString(void) {
	DynArray_T strings;
	size_t ulTotalLength = 1;
	char *result = NULL;

	if(!bIsInitialized)
		return NULL;

	strings = DynArray_new(ulCount);
    (void) FT_preOrderTraversal(oDNRoot, strings, 0);

   	DynArray_map(strings, (void (*)(void *, void*)) FT_strlenAccumulate,
                (void*) &ulTotalLength);

   	result = malloc(ulTotalLength);
   	if(result == NULL) {
      	DynArray_free(strings);
      	return NULL;
   	}
   	*result = '\0';

   	DynArray_map(strings, (void (*)(void *, void*)) FT_strcatAccumulate,
                (void *) result);

   	DynArray_free(strings);

   	return result;
}
