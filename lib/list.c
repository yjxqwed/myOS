#include <lib/list.h>
#include <common/debug.h>
#include <kprintf.h>

void list_init(list_t *l) {
    ASSERT(l != NULL);
    l->head.prev = NULL;
    l->tail.next = NULL;
    l->head.next = &(l->tail);
    l->tail.prev = &(l->head);
}

void list_push_back(list_t *l, list_node_t *node) {
    ASSERT(l != NULL);
    ASSERT(node != NULL);
    ASSERT(node->prev == NULL && node->next == NULL);
    list_node_t *last = l->tail.prev;
    last->next = node;
    node->prev = last;
    node->next = &(l->tail);
    l->tail.prev = node;
}

void list_push_front(list_t *l, list_node_t *node) {
    ASSERT(l != NULL);
    ASSERT(node != NULL);
    ASSERT(node->prev == NULL && node->next == NULL);
    list_node_t *first = l->head.next;
    l->head.next = node;
    node->prev = &(l->head);
    node->next = first;
    first->prev = node;
}

list_node_t *list_pop_back(list_t *l) {
    ASSERT(l != NULL);
    list_node_t *back = l->tail.prev;
    if (back == &(l->head)) {
        return NULL;
    }
    back->prev->next = &(l->tail);
    l->tail.prev = back->prev;
    back->next = back->prev = NULL;
    return back;
}

list_node_t *list_pop_front(list_t *l) {
    ASSERT(l != NULL);
    list_node_t *front = l->head.next;
    if (front == &(l->tail)) {
        return NULL;
    }
    l->head.next = front->next;
    front->next->prev = &(l->head);
    front->next = front->prev = NULL;
    return front;
}

list_node_t *list_back(list_t *l) {
    ASSERT(l != NULL);
    if (l->tail.prev == &(l->head)) {
        return NULL;
    } else {
        return l->tail.prev;
    }
}

list_node_t *list_front(list_t *l) {
    ASSERT(l != NULL);
    if (l->head.next == &(l->tail)) {
        return NULL;
    } else {
        return l->head.next;
    }
}

bool_t list_empty(list_t *l) {
    ASSERT(l != NULL);
    if (l->head.next == &(l->tail)) {
        return True;
    } else {
        return False;
    }
}

size_t list_length(list_t *l) {
    ASSERT(l != NULL);
    size_t len = 0;
    list_node_t *p;
    for (p = l->head.next; p != &(l->tail); p = p->next) {
        len++;
    }
    return len;
}

bool_t list_find(list_t *l, list_node_t *node) {
    ASSERT(l != NULL);
    if (node == NULL || node->next == NULL || node->prev == NULL) {
        return False;
    }
    list_node_t *p;
    for (p = l->head.next; p != &(l->tail); p = p->next) {
        if (p == node) {
            return True;
        }
    }
    return False;
}

void list_insert_before(list_node_t *b, list_node_t *node) {
    ASSERT(b != NULL && node != NULL);
    ASSERT(node->next == NULL && node->prev == NULL);
    list_node_t *prev = b->prev;
    prev->next = node;
    node->prev = prev;
    node->next = b;
    b->prev = node;
}

void list_insert_after(list_node_t *a, list_node_t *node) {
    ASSERT(a != NULL && node != NULL);
    ASSERT(node->next == NULL && node->prev == NULL);
    list_node_t *next = a->next;
    a->next = node;
    node->prev = a;
    node->next = next;
    next->prev = node;
}

list_node_t *list_pred(list_t *l, list_node_pred pred, void *args) {
    ASSERT(l != NULL);
    list_node_t *p;
    for (p = l->head.next; p != &(l->tail); p = p->next) {
        if (pred(p, args)) {
            return p;
        }
    }
    return NULL;
}

void list_erase(list_node_t *node) {
    ASSERT(node->prev != NULL && node->next != NULL);
    node->prev->next = node->next;
    node->next->prev = node->prev;
    node->next = node->prev = NULL;
}
