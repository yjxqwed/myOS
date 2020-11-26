#ifndef __LIST_H__
#define __LIST_H__

/**
 * Doubly linked list library
*/

#include <common/types.h>

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


#endif
