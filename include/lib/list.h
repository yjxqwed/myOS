#ifndef __LIST_H__
#define __LIST_H__

/**
 * Doubly linked list library
*/

#include <common/types.h>
#include <common/debug.h>

struct ListNode;
typedef struct ListNode list_node_t;

typedef struct ListNode {
    list_node_t *prev;
    list_node_t *next;
} list_node_t;

typedef struct List {
    // the actual first node is head.next
    list_node_t head;
    // the actual last node is tail.prev
    list_node_t tail;
} list_t;

typedef bool_t (*list_node_pred)(list_node_t*, void*);

void         list_init(list_t *l);

void         list_push_back(list_t *l, list_node_t *node);
void         list_push_front(list_t *l, list_node_t *node);
list_node_t *list_pop_back(list_t *l);
list_node_t *list_pop_front(list_t *l);
list_node_t *list_back(list_t *l);
list_node_t *list_front(list_t *l);
bool_t       list_empty(list_t *l);
size_t       list_length(list_t *l);
bool_t       list_find(list_t *l, list_node_t *node);
void         list_erase(list_node_t *node);

// insert node before b
void         list_insert_before(list_node_t *b, list_node_t *node);
// insert node after a
void         list_insert_after(list_node_t *a, list_node_t *node);


// return the first node that pred(node, args) is True
list_node_t *list_pred(list_t *l, list_node_pred pred, void *args);

typedef void (*list_traversal_func_t)(list_node_t *p);

// @brief traverse list and perform tfunc for every node,
//        tfunc should not modify the list l itself
// @param l list pointer
// @param tfunc traversal function
void list_traverse(list_t *l, list_traversal_func_t tfunc);


#define __member_offset(struct_type, member) \
    (uintptr_t)(&((struct_type*)0)->member)

// get the container struct pointer from a list_node pointer
// @param struct_type the type name of the target struct
// @param member the field name of node_ptr
// @param node_ptr the pointer to the list_node_t type
#define __container_of(struct_type, member, node_ptr) \
    (struct_type*)( \
        (uintptr_t)node_ptr - __member_offset(struct_type, member) \
    )

/**
 * @brief list iteration
 * @param list pointer to list
 * @param node_ptr the list node pointer for interation
 */
#define __list_for_each(list, node_ptr) \
    for ( \
        node_ptr = list->head.next; \
        node_ptr != &(list->tail); \
        node_ptr = node_ptr->next \
    )

/**
 * @brief push front
 * @param list pointer to list
 * @param container_ptr pointer to container
 * @param member field name of the list_node_t of the container type
 */
#define __list_push_front(list, container_ptr, member) { \
    ASSERT(list != NULL && container_ptr != NULL); \
    list_node_t *p = &(container_ptr->member); \
    ASSERT(p->next == NULL && p->prev == NULL); \
    ASSERT(!list_find(list, p)); \
    list_push_front(list, p); \
}

/**
 * @brief push back
 * @param list pointer to list
 * @param container_ptr pointer to container
 * @param member field name of the list_node_t of the container type
 */
#define __list_push_back(list, container_ptr, member) { \
    ASSERT(list != NULL && container_ptr != NULL); \
    list_node_t *p = &(container_ptr->member); \
    ASSERT(p->next == NULL && p->prev == NULL); \
    ASSERT(!list_find(list, p)); \
    list_push_back(list, p); \
}

/**
 * @brief pop front
 * @param list pointer to list
 * @param struct_type type of container
 * @param member field name of the list_node_t of the container type
 */
#define __list_pop_front(list, struct_type, member) ({ \
    struct_type *front = NULL; \
    list_node_t *p = list_pop_front(list); \
    if (p != NULL) { \
        front = __container_of(struct_type, member, p); \
    } \
    front; \
})

/**
 * @brief pop back
 * @param list pointer to list
 * @param struct_type type of container
 * @param member field name of the list_node_t of the container type
 */
#define __list_pop_back(list, struct_type, member) ({ \
    struct_type *back = NULL; \
    list_node_t *p = list_pop_back(list); \
    if (p != NULL) { \
        back = __container_of(struct_type, member, p); \
    } \
    back; \
})

#endif
