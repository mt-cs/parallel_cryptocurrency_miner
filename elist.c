/**
 * @file elist.c
 */


#include <errno.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#include "elist.h"
#include "logger.h"

#define DEFAULT_INIT_SZ 10 //preprocesser macro
#define RESIZE_MULTIPLIER 2 //double the element

/**
* checks if index is valid.
*/
bool idx_is_valid(struct elist *list, size_t idx); // private function in c

/**
 * Creates a new elist data structure. If the initial size is set to 0, the
 * default capacity will be used. 
 *
 * @param init_capacity The initial capacity of the list
 * @param item_sz Size of the elements that will be stored in the list (in bytes)
 *
 * @return A pointer to the newly-created list, or NULL on failure.
 */
struct elist *elist_create(size_t list_sz, size_t item_sz)
{
	/* Create space for the container structL */
	struct elist *list = malloc(sizeof(struct elist));
	if (list == NULL) { // check if malloc returns null
		return NULL;
	}

	/* if list_sz is 0, then we'll just use the default */
	if (list_sz == 0) {
		list_sz = DEFAULT_INIT_SZ;
	}
	
	list->capacity = list_sz; // the capacity
	list->item_sz = item_sz;
	list->size = 0; // the actual number of element. Here empty list we haven't put anything in the "array list"

	size_t storage_bytes = list->capacity * list->item_sz;
	// LOG("Initializing new elist: capacity=[%zu], item_sz=[%zu], bytes=[%zu]\n",
	// 		list->capacity,
	// 		list->item_sz,
	// 		storage_bytes);
			
	list->element_storage = malloc(storage_bytes);

	/*Check if returns NULL */
	if (list-> element_storage == NULL) {
		perror("malloc");
		free(list);
		return NULL;
	}
    return list;
}

/**
 * Destroys the specified list and frees any memory that was allocated to it.
 *
 * @param list The list to destroy
 */
void elist_destroy(struct elist *list)
{
	free(list->element_storage); // element_storage first before we free the list itself
	free(list);
}


/**
 * Increases or decreases the storage capacity of a list. When decreasing the
 * capacity, any elements beyond the new capacity are freed.
 *
 * @param list The list to change the capacity of
 * @param capacity The list's new capacity
 *
 * @return zero on success, nonzero on failure
 */
int elist_set_capacity(struct elist *list, size_t capacity)
{
	if (capacity == 0) {
		capacity = 1;
		elist_clear(list);
	}

	list->element_storage = realloc(list->element_storage, capacity * list->item_sz);
	if (list->element_storage == NULL) {
		perror("realloc");
		return -1;
	}	
	
	list->capacity = capacity;
	if (list->size > capacity) {
		list->size = capacity;
	}
    return 0;
}

/**
 * Retrieves the current capacity of the list.
 *
 * @param list The list to retrieve the capacity of
 *
 * @return capacity of  the list
 */
size_t elist_capacity(struct elist *list)
{
	return list->capacity;
}

/**
 * Adds a new element to the list by copying its in-memory contents into the
 * list's element storage.
 *
 * @param list The list to copy the element into
 * @param item The element to copy
 *
 * @return Index of the element, or -1 on failure
 */
ssize_t elist_add(struct elist *list, void *item)
{
	/*Check if we need to resize*/
	if (list->size >= list->capacity) {
		size_t capacity = list->capacity * RESIZE_MULTIPLIER;
		LOG("Resizing the list. New capacity: %zu\n", list->capacity);
		if(elist_set_capacity(list, capacity) == -1) {
			return -1;
		}
	}
	size_t idx = list->size++;
	void *item_ptr = (int *)list->element_storage + idx * list->item_sz;
	memcpy(item_ptr, item, list->item_sz);
    return idx;
}

/**
 * Creates storage space for a new list element and returns a pointer to it.
 * Unlike elist_add, copying memory is not required so this function may be more
 * efficient when performance is critical.
 *
 * @param list The list to add a new element to
 *
 * @return Pointer to the new element (NOTE: it will not be initialized) or NULL
 * on failure.
 */
void *elist_add_new(struct elist *list)
{	
	/*Check if we need to resize*/
	size_t list_sz = elist_size(list);
	if (list_sz >= list->capacity) {
		size_t capacity = list->capacity * RESIZE_MULTIPLIER;
		elist_set_capacity(list, capacity);
	}
	size_t idx = list->size++;
	size_t pos = idx * list->item_sz;
	void *item_ptr = (int *)list->element_storage + pos; // pointer to the start of the storage
    return item_ptr;
}

/**
 * Replaces an element at a particular index.
 *
 * @param list The list to modify
 * @param idx Index of the element to replace
 * @param item Element to place at 'idx' in the list
 *
 * @return zero on success, nonzero on failure
 */
int elist_set(struct elist *list, size_t idx, void *item)
{
	if(!idx_is_valid(list, idx)){
		return -1;
	}
	memcpy((int *)list->element_storage + idx * list->item_sz, item, list->item_sz);
    return 0;
}

/**
 * Retrieves the element at the given index.
 *
 * @param list The list to retrieve the element from
 * @param idx Index of the element to retrieve
 *
 * @return A pointer to the element, or NULL if the index is invalid.
 */
void *elist_get(struct elist *list, size_t idx)
{
	if (idx >= list->size) {
		return NULL;
	}
    return (int *)list->element_storage + idx * list->item_sz;
}

/**
 * Retrieves the size of the elist (the actual number of elements stored).
 *
 * @param list The list to retrieve the size of
 *
 * @return The list size
 */
size_t elist_size(struct elist *list)
{
    return list->size;
}

/**
 * Removes the element at the given index and shifts any subsequent element to
 * the left (or in other words, subtracts one from their indices).
 *
 * @param list The list to modify
 * @param idx Index of the element to remove
 * 
 * @return zero on success, nonzero on failure
 */
int elist_remove(struct elist *list, size_t idx)
{
	if (!idx_is_valid(list, idx)) {
		fprintf(stderr, "Out of index!");
		return -1;
	}
	memmove((int *)list->element_storage + idx * list->item_sz,
			(int *)list->element_storage + (idx + 1) * list->item_sz,
			(list->size - idx - 1) * list->item_sz);
	// for(size_t j = idx + 1; j < list->size; j++) {
	// 	void* item = elist_get(list, j);
	// 	elist_set(list, j - 1, item);
	// }
	list->size--;
	
    return 0;
}

/**
 * Empties a list (but does not change its capacity).
 *
 * @param list The list to clear
 */
void elist_clear(struct elist *list)
{
	list->size = 0;
}

/**
 * Empties a list (but does not change its capacity) and zeroes out all its
 * elements.
 * 
 * @param list The list to clear
 */
void elist_clear_mem(struct elist *list)
{
	elist_clear(list);
	memset(list->element_storage, 0, list->capacity * list->item_sz);
}

/**
 * Retrieves the index of the first occurrence of a given element in the list.
 * This function uses memcmp() to determine element equality, so the supplied
 * element must be an exact copy (in terms of memory representation).
 *
 * @param list The list to search for the element
 * @param item An exact memory copy of the element to retrieve the index of
 *
 * @return The index, or -1 if the element was not found
 */
ssize_t elist_index_of(struct elist *list, void *item)
{
	size_t index = 0;
	while(index < list->size) {
		void *temp = (int *)list->element_storage + index*list->item_sz;
		if (memcmp(item, temp, list->item_sz)==0) {
			return index;
		}
		index++;
	}
    return -1;
}

/**
 * Sorts an elist using the provided comparator function pointer.
 *
 * The comparator signature follows the same form as the comparators of
 * qsort(3):
 *
 * int (*compar)(const void *, const void *)
 *
 * The comparison function must return an integer less than, equal to, or
 * greater  than zero  if the first argument is considered to be respectively
 * less than, equal to, or greater than the second.  If two members compare as
 * equal, their order in the sorted array is undefined.
 *
 * @param list The list to sort
 * @param comparator Comparison function to use for the sort
 */
void elist_sort(struct elist *list, int (*comparator)(const void *, const void *))
{
	qsort (list->element_storage, list->size, list->item_sz, comparator);
	return;
}

/*
* Checks if index is valid
*/
bool idx_is_valid(struct elist *list, size_t idx)
{
	if (idx >= list->size) {
		return false;
	}
	return true;
}
