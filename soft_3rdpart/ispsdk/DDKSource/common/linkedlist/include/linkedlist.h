/**
 ************************************************************************
  @file linkedlist.h

  @brief Template double linked list

  @copyright StarFive Technology Co., Ltd. All Rights Reserved.

  @license Strictly Confidential.
    No part of this software, either material or conceptual may be copied or
    distributed, transmitted, transcribed, stored in a retrieval system or
    translated into any human or computer language in any form by any means,
    electronic, mechanical, manual or other-wise, or disclosed to third
    parties without the express written permission of
    Shanghai StarFive Technology Co., Ltd.

 ******************************************************************************/

#ifndef __LINKEDLIST_H__
#define __LINKEDLIST_H__


#include "stf_types.h"


#ifdef __cplusplus
extern "C" {
#endif


/**
 * @defgroup linkedlist_t Template Double Linked List
 *
 * @{
 */

/**
 * @brief A double linked list cell
 *
 * This structure can be hidden to the user, when using the "Object" functions.
 * But the user can also use the other functions to directly control the cell
 */
typedef struct Cell_T {
	/**
	 * @brief Pointer to the held object. 
	 * @Description void* is used to enable a "template" behaviour - it actually stores the start address of the object.
	 * Manual cast is therefore needed to retrieve the original object.
	 */
  void* object;

  struct Cell_T *pNext; /**< @brief Next Cell */
  struct Cell_T *pPrev; /**< @brief Previous Cell */

  struct LinkedList_T *pContainer; ///< @brief Owner of the Cell - NULL if none
} sCell_T;

/**
 * @brief Double linked list
 * @warning If the list is searched manually be aware that the pNext or pPrev of the cells is NEVER NULL (see the anchor explanation).
 *
 * The list elements are contained into sCell_T structures that can be hidden from the user (using the Object functions) or directly manipulated.
 * The sCell_T does not have to be allocated dynamically, it can be imbeded into the user's code, but one should be aware that the cell's object pointer should nonetheless be filled (even thought the container_of() macro could be used to retrieve the container struct).
 *
 * The list is created with List_create().
 * It is destroyed calling List_clear() (or List_clearObjects()) and then freed.
 *
 * @code
	LinkedList_T *pList = NULL;
	if ( List_create(&pList) != STF_SUCCESS ) return EXIT_FAILURE;
	// add things to the list
	List_clear(pList);
	STF_FREE(pList); @endcode
 */
typedef struct LinkedList_T {
	/**
	 * @brief Anchor of the list - not a legit element for the 'user'
	 * @li pNext is the HEAD
	 * @li pPrev is the TAIL
	 *
	 * The anchor is used so that the previous and next elements of every cell is NEVER NULL.
	 * This allows insertion and removal of cells with a lot less branches.
	 *
	 * If the list is searched you should use List_getPrev() and List_getNext() to avoid checking against the anchor
	 */
	sCell_T sAnchor;

    STF_U32 u32Elements; /**< @brief Number of elements currently in the list */
    
} sLinkedList_T;

/**
 * @brief Creates an empty linked list.
 *
 * @param pList pointer to the list to initialise
 *
 * @return STF_SUCCESS
 * @return STF_ERROR_INVALID_PARAMETERS if ppList is NULL
 * @return STF_ERROR_MEMORY_IN_USE if *ppList is NOT NULL
 * @return STF_ERROR_MALLOC_FAILED if malloc of the list failed
 */
STF_RESULT List_init(sLinkedList_T *pList);

/**
 * @brief Create a pool of N cells using pfnConstructor to create each objects
 *
 * @warning If the population fails the list HAS to be cleaned by the caller (the destructor operation for the objects is not known)
 *
 * @param pList pointer to the list to initialise
 * @param u32nelem number of elements in the pool
 * @param pfnConstructor pointer to a constructor function for each object in the pool - if NULL no objects are put in the cells
 * @param param parameter to give to the constructor function
 
 *
 * @return STF_SUCCESS
 * @return Or any return code from its delegate functions
 *
 * @see Delegates to List_create() and List_pushBackObject()
 */
STF_RESULT List_initPool(sLinkedList_T *pList, STF_U32 u32nelem, void *param, void* pfnConstructor(void *param));

/**
 * @brief Free all the cells, but NOT their contained object - does not free the list itself
 *
 * @warning User is responsible to free the object of each cell first!
 *
 * @return STF_SUCCESS
 * @return STF_ERROR_INVALID_PARAMETERS if the pList is NULL
 *
 * @see Use List_clearObjects() if you need to perform an action on the objects prior to destruction.
 */
STF_RESULT List_clear(sLinkedList_T *pList);

/**
 * @brief Free all the cells AND their contained object (using pfnDestructor for the objects) - does not free the list itself
 *
 * @param pList
 * @param pfnDestructor destructor function to be used on the cell's objects
 *
 * @return STF_SUCCESS
 * @return STF_ERROR_INVALID_PARAMETERS if the pList or pfnDestructor are NULL
 *
 * @see List_free()
 */
STF_RESULT List_clearObjects(sLinkedList_T *pList, void pfnDestructor(void*));

/**
 * @brief Add an element after the list's tail - allocates the cell
 *
 * @param pList
 * @param elem the element to push in the list - NULL is allowed.
 *
 * @return STF_SUCCESS
 * @return STF_ERROR_MALLOC_FAILED if the allocation of the cell failed
 * @return STF_ERROR_INVALID_PARAMETER if pList is NULL
 * @return Or any return code from its delegate functions
 *
 * @see List_pushFrontObject()
 * @see Delegates to List_pushBack()
 */
STF_RESULT List_pushBackObject(sLinkedList_T *pList, void *elem);

/**
 * @brief Add an element before the list's head - allocates the cell
 *
 * @param pList
 * @param elem the element to push in the list - NULL is allowed.
 *
 * @return STF_SUCCESS if the element is added
 * @return STF_ERROR_MALLOC_FAILED if the allocation of the cell failed
 * @return STF_ERROR_INVALID_PARAMETER if pList is NULL
 * @return Or any return code from its delegate functions
 *
 * @see List_pushBackObject()
 * @see Delegates to List_pushFront()
 */
STF_RESULT List_pushFrontObject(sLinkedList_T *pList, void *elem);

/**
 * @brief Add a cell after the list's tail
 *
 * @note Call the updater functions when all linking are dones in that order:
 @li on the old tail if the list was not empty
 @li on the new cell
 *
 * @param pList
 * @param pCell orphan cell
 *
 * @return STF_SUCCESS
 * @return STF_ERROR_UNEXPECTED_STATE if the element is attached to another list - detach it first.
 *
 * @see List_pushFront()
 */
STF_RESULT List_pushBack(sLinkedList_T *pList, sCell_T *pCell);

/**
 * @brief Add a cell before the list's head 
 *
 * @note Call the updater functions when all linking are dones in that order:
 @li on the new cell
 @li on the old head if the list was not empty
 * 
 * @param pList
 * @param pCell orphan cell
 *
 * @return STF_SUCCESS if the element is not yet attached to a list
 * @return STF_ERROR_UNEXPECTED_STATE if the element is attached to another list - detach it first.
 *
 * @see List_pushBack
 */
STF_RESULT List_pushFront(sLinkedList_T *pList, sCell_T *pCell);

/**
 * @brief Detach one cell from the list - does not delete the cell or its object.
 *
 * The cell's container, next and previous pointers are set to NULL.
 *
 * @note Call the updater function after all the linking is done in that order:
 @li on the previous cell (if not NULL)
 @li on the detached cell
 @li on the next cell (if not NULL)
 *
 * @return STF_SUCCESS
 * @return STF_ERROR_INVALID_PARAMETER if the element is attached to another list - detach it first.
 */
STF_RESULT List_detach(sCell_T *pCell);

/**
 * @brief Delete one cell from its list - its object is not deleted.
 *
 * @warning User is responsible to free the cell's object
 *
 * The cell is detached and then freed.
 *
 * @note Does NOT call the updater functions.
 *
 * @return STF_SUCCESS
 * @return Or return code for its delegate functions.
 *
 * @see Delegates to List_detach()
 * @see List_removeObject()
 */
STF_RESULT List_remove(sCell_T *pCell);

/**
 * @brief Delete the cel from its list and its object (using pfnDestructor)
 *
 * The object is deleted only if the cell was removed successfully.
 *
 * @note Does NOT call the updater functions.
 *
 * @return STF_SUCCESS
 * @return STF_ERROR_INVALID_PARAMETER if pCell or pfnDestructor are NULL
 * @return Or return code for its delegate functions.
 *
 * @see Delegates to List_remove()
 */
STF_RESULT List_removeObject(sCell_T *pCell, void pfnDestructor(void* param));

/**
 * @brief Detach the tail cell
 *
 * @note Calls the updater in List_detach()
 *
 * @return a pointer to the tail cell (now an orphan cell)
 * @return NULL if the list is empty or pList is NULL
 *
 * @see Delegates to List_detach()
 * @see List_popHead()
 */
sCell_T* List_popBack(sLinkedList_T *pList);

/**
 * @brief Detach the front cell
 *
 * @note Calls the updater in List_detach()
 *
 * @return a pointer to the head cell (now an orphan cell)
 * @return NULL if the list is empty or pList is NULL
 *
 * @see Delegates to List_detach()
 * @see List_popBack()
 */
sCell_T* List_popFront(sLinkedList_T *pList);

/**
 * @brief Access the head
 *
 * @return a pointer to the head cell
 * @return NULL if the list is empty or pList is NULL
 *
 * @see List_getTail()
 */
sCell_T* List_getHead(sLinkedList_T *pList);

/**
 * @brief Access the tail
 *
 * @return a pointer to the tail cell
 * @return NULL if the list is empty or pList is NULL
 *
 * @see List_getHead()
 */
sCell_T* List_getTail(sLinkedList_T *pList);

/**
 * @brief Get the next cell pointer - handle the anchor
 *
 * @return NULL if the cell is orphan or if it is the Tail
 */
sCell_T* List_getNext(sCell_T *pCell);

/**
 * @brief Get the previous cell pointer - handle the anchor
 *
 * @return NULL if the cell is orphan or if it is the Head
 */
sCell_T* List_getPrev(sCell_T *pCell);

/**
 * @brief To know if the list is empty.
 *
 * @return STF_TRUE if pList is NULL
 */
STF_BOOL8 List_isEmpty(sLinkedList_T *pList);

/**
 * @brief The number of elements in the list.
 *
 * @return 0 if the pList is NULL
 */
STF_U32 List_nElem(sLinkedList_T *pList);

/**
  * @brief Apply a visitor on the list.
  *
  * Searches from head to tail using the given pfnVisitor function.
  * The choice of going to the next element is made by the visitor function but the next cell is chosen before the call
  *
  * @param pList
  * @param param to give to the visitor
  * @param pfnVisitor gives the element from the list as 1st parameter and element as 2nd. It returns STF_TRUE if the visitor can continue its run through the list.
  *
  * @return the pointer to the last cell visited
  * @return NULL if all the list was visited or if pList or pfnVisitor are NULL
 */
sCell_T* List_visitor(sLinkedList_T *pList, void* param, STF_BOOL8 pfnVisitor(void* elem, void *param));

/**
 * @brief Example of visitor: finding an element
 *
 * @param listElem cell's object
 * @param lookingFor given param of List_visitor()
 */
STF_BOOL8 ListVisitor_findPointer(void* listElem, void* lookingFor);

/**
  *@}
 */


#ifdef __cplusplus
}
#endif


#endif // __LINKEDLIST_H__
