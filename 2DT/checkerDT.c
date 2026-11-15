/*--------------------------------------------------------------------*/
/* checkerDT.c                                                        */
/* Author:                                                            */
/*--------------------------------------------------------------------*/

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "checkerDT.h"
#include "dynarray.h"
#include "path.h"



/* see checkerDT.h for specification */
boolean CheckerDT_Node_isValid(Node_T oNNode) {
   Node_T oNParent;
   Path_T oPNPath;
   Path_T oPPPath;

   /* Sample check: a NULL pointer is not a valid node */
   if(oNNode == NULL) {
      fprintf(stderr, "A node is a NULL pointer\n");
      return FALSE;
   }

   oNParent = Node_getParent(oNNode);
   if(oNParent != NULL) {
      size_t ulChildID;
      Node_T oNResult;
      oPNPath = Node_getPath(oNNode);
      oPPPath = Node_getPath(oNParent);

      /*node cannot have null path*/
      if (oPNPath == NULL) {
         fprintf(stderr, "Path is NULL\n");
         return FALSE;
      }
      /*a parent node, if exists cannot have null path*/
      if (oPPPath == NULL) {
         fprintf(stderr, "Parent Path is NULL\n");
         return FALSE;
      }

      /* Sample check: parent's path must be the longest possible
         proper prefix of the node's path */
      if(Path_getSharedPrefixDepth(oPNPath, oPPPath) !=
         Path_getDepth(oPNPath) - 1) {
         fprintf(stderr, "P-C nodes don't have P-C paths: (%s) (%s)\n",
                 Path_getPathname(oPPPath), Path_getPathname(oPNPath));
         return FALSE;
      }

      /*parent must be linked to child with given path*/
      if (!Node_hasChild(oNParent, oPNPath, &ulChildID)) {
         fprintf(stderr, "Parent does not have child linked with path %s\n",
            Path_getPathname(oPNPath));
         return FALSE;
      }
      /*parent must be linked to child with given ulChildID*/
      if (Node_getChild(oNParent, ulChildID, &oNResult) != SUCCESS) {
         fprintf(stderr, "Parent does not have child linked with ID %lu\n", ulChildID);
         return FALSE;
      }
      /*parent must have one child with exact path*/
      if (oNResult != oNNode) {
         fprintf(stderr, "Parent has more than one child with same path %s\n",
            Path_getPathname(oPNPath));
         return FALSE;
      }
   }



   return TRUE;
}

/*
   Performs a pre-order traversal of the tree rooted at oNNode.
   Returns FALSE if a broken invariant is found and
   returns TRUE otherwise.

   You may want to change this function's return type or
   parameter list to facilitate constructing your checks.
   If you do, you should update this function comment.
*/
static boolean CheckerDT_treeCheck(Node_T oNNode) {
   size_t ulIndex;
   Node_T oNPrev = NULL;

   if(oNNode!= NULL) {

      /* Sample check on each node: node must be valid */
      /* If not, pass that failure back up immediately */
      if(!CheckerDT_Node_isValid(oNNode))
         return FALSE;

      /* Recur on every child of oNNode */
      for(ulIndex = 0; ulIndex < Node_getNumChildren(oNNode); ulIndex++)
      {
         Node_T oNChild = NULL;
         int iStatus = Node_getChild(oNNode, ulIndex, &oNChild);

         if(iStatus != SUCCESS) {
            fprintf(stderr, "getNumChildren claims more children than getChild returns\n");
            return FALSE;
         }

         /* children must be in strictly increasing lexicographic order */
         if(oNPrev != NULL &&
            Path_comparePath(Node_getPath(oNPrev),
                             Node_getPath(oNChild)) >= 0) {
            fprintf(stderr, "Children of %s out of order: (%s) before (%s)\n",
                    Path_getPathname(Node_getPath(oNNode)),
                    Path_getPathname(Node_getPath(oNPrev)),
                    Path_getPathname(Node_getPath(oNChild)));
            return FALSE;
         }
         oNPrev = oNChild;

         /* if recurring down one subtree results in a failed check
            farther down, passes the failure back up immediately */
         if(!CheckerDT_treeCheck(oNChild))
            return FALSE;
      }
   }
   return TRUE;
}

/* see checkerDT.h for specification */
boolean CheckerDT_isValid(boolean bIsInitialized, Node_T oNRoot,
                          size_t ulCount) {

   /* Sample check on a top-level data structure invariant:
      if the DT is not initialized, its count should be 0. */
   if(!bIsInitialized)
      if(ulCount != 0) {
         fprintf(stderr, "Not initialized, but count is not 0\n");
         return FALSE;
      }

   /* Now checks invariants recursively at each node from the root. */
   return CheckerDT_treeCheck(oNRoot);
}
