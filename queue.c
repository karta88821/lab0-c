#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "harness.h"
#include "list.h"
#include "queue.h"

/* Notice: sometimes, Cppcheck would find the potential NULL pointer bugs,
 * but some of them cannot occur. You can suppress them by adding the
 * following line.
 *   cppcheck-suppress nullPointer
 */

/*
 * Create empty queue.
 * Return NULL if could not allocate space.
 */
struct list_head *q_new()
{
    struct list_head *head = malloc(sizeof(struct list_head));
    if (!head) {
        return NULL;
    }
    INIT_LIST_HEAD(head);
    return head;
}

/* Free all storage used by queue */
void q_free(struct list_head *l)
{
    if (!l) {
        return;
    }

    element_t *node = NULL, *safe = NULL;
    list_for_each_entry_safe (node, safe, l, list) {
        q_release_element(node);
    }
    free(l);
}

element_t *new_element(char *s)
{
    element_t *element = malloc(sizeof(element_t));

    if (!element) {
        return NULL;
    }

    element->list.next = NULL;
    element->list.prev = NULL;

    if (s) {
        // strdup: duplicates strings
        element->value = strdup(s);
        if (!element->value) {
            free(element);
            return NULL;
        }
    } else {
        element->value = NULL;
    }

    return element;
}

/*
 * Attempt to insert element at head of queue.
 * Return true if successful.
 * Return false if q is NULL or could not allocate space.
 * Argument s points to the string to be stored.
 * The function must explicitly allocate space and copy the string into it.
 */
bool q_insert_head(struct list_head *head, char *s)
{
    if (!head) {
        return false;
    }

    element_t *node = new_element(s);

    if (!node) {
        return false;
    }

    list_add(&node->list, head);

    return true;
}

/*
 * Attempt to insert element at tail of queue.
 * Return true if successful.
 * Return false if q is NULL or could not allocate space.
 * Argument s points to the string to be stored.
 * The function must explicitly allocate space and copy the string into it.
 */
bool q_insert_tail(struct list_head *head, char *s)
{
    if (!head) {
        return false;
    }

    element_t *node = new_element(s);

    if (!node) {
        return false;
    }

    list_add_tail(&node->list, head);

    return true;
}

/*
 * Attempt to remove element from head of queue.
 * Return target element.
 * Return NULL if queue is NULL or empty.
 * If sp is non-NULL and an element is removed, copy the removed string to *sp
 * (up to a maximum of bufsize-1 characters, plus a null terminator.)
 *
 * NOTE: "remove" is different from "delete"
 * The space used by the list element and the string should not be freed.
 * The only thing "remove" need to do is unlink it.
 *
 * REF:
 * https://english.stackexchange.com/questions/52508/difference-between-delete-and-remove
 */
element_t *q_remove_head(struct list_head *head, char *sp, size_t bufsize)
{
    if (!head || list_empty(head)) {
        return NULL;
    }

    element_t *element = list_entry(head->next, element_t, list);

    // bufsize -> strlen(sp) + 1
    // strncpy中的count: 最多複製bufsize個char到sp
    if (sp) {
        strncpy(sp, element->value, bufsize - 1);
        strncpy(sp + bufsize - 1, "\0", 1);
    }

    list_del(head->next);

    return element;
}

/*
 * Attempt to remove element from tail of queue.
 * Other attribute is as same as q_remove_head.
 */
element_t *q_remove_tail(struct list_head *head, char *sp, size_t bufsize)
{
    if (!head || list_empty(head)) {
        return NULL;
    }

    element_t *element = list_entry(head->prev, element_t, list);

    if (sp) {
        strncpy(sp, element->value, bufsize - 1);
        strncpy(sp + bufsize - 1, "\0", 1);
    }

    list_del(head->prev);

    return element;
}

/*
 * WARN: This is for external usage, don't modify it
 * Attempt to release element.
 */
void q_release_element(element_t *e)
{
    free(e->value);
    free(e);
}

/*
 * Return number of elements in queue.
 * Return 0 if q is NULL or empty
 */
int q_size(struct list_head *head)
{
    if (!head || list_empty(head)) {
        return 0;
    }
    int size = 0;
    struct list_head *node = NULL;
    list_for_each (node, head) {
        size++;
    }
    return size;
}

/*
 * Delete the middle node in list.
 * The middle node of a linked list of size n is the
 * ⌊n / 2⌋th node from the start using 0-based indexing.
 * If there're six element, the third member should be return.
 * Return NULL if list is NULL or empty.
 */
bool q_delete_mid(struct list_head *head)
{
    if (!head || list_empty(head)) {
        return false;
    }

    struct list_head *slow = head->next;
    for (struct list_head *fast = head->next;
         fast != head && fast->next != head; fast = fast->next->next) {
        slow = slow->next;
    }
    list_del(slow);
    q_release_element(list_entry(slow, element_t, list));
    return true;
}

/*
 * Delete all nodes that have duplicate string,
 * leaving only distinct strings from the original list.
 * Return true if successful.
 * Return false if list is NULL.
 *
 * Note: this function always be called after sorting, in other words,
 * list is guaranteed to be sorted in ascending order.
 */
bool q_delete_dup(struct list_head *head)
{
    if (!head) {
        return false;
    }
    struct list_head *curr = head->next, *next;

    while (curr != head && curr->next != head) {
        next = curr->next;
        element_t *ele_1 = list_entry(curr, element_t, list);
        element_t *ele_2 = list_entry(next, element_t, list);

        if (strcmp(ele_1->value, ele_2->value) == 0) {
            // delete the duplicate elements except the first duplicate element
            while (next != head && strcmp(ele_1->value, ele_2->value) == 0) {
                curr->next = next->next;
                next->next->prev = curr;
                q_release_element(ele_2);
                next = curr->next;
                ele_2 = list_entry(next, element_t, list);
            }
            // delete the first node of the duplicate list
            curr->prev->next = next;
            next->prev = curr->prev;
            q_release_element(list_entry(curr, element_t, list));
            curr = next;
        } else {
            curr = curr->next;
        }
    }
    return true;
}

/*
 * Attempt to swap every two adjacent nodes.
 */
void q_swap(struct list_head *head)
{
    if (!head || list_empty(head) || list_is_singular(head)) {
        return;
    }
    struct list_head *curr = head->next, *tmp = NULL;
    while (curr != head && curr->next != head) {
        tmp = curr->next;
        curr->next = tmp->next;
        tmp->prev = curr->prev;
        curr->prev->next = tmp;
        curr->prev = tmp;
        tmp->next->prev = curr;
        tmp->next = curr;
        curr = curr->next;
    }
}

/*
 * Reverse elements in queue
 * No effect if q is NULL or empty
 * This function should not allocate or free any list elements
 * (e.g., by calling q_insert_head, q_insert_tail, or q_remove_head).
 * It should rearrange the existing ones.
 */
void q_reverse(struct list_head *head)
{
    if (!head || list_empty(head) || list_is_singular(head)) {
        return;
    }

    struct list_head *iter, *next;
    list_for_each_safe (iter, next, head) {
        list_move(iter, head);
    }
}

static struct list_head *merge(struct list_head *l, struct list_head *r)
{
    struct list_head *head = l;
    struct list_head *prev = NULL;
    struct list_head **ptr = &head;
    struct list_head **node;

    for (node = NULL; l && r; *node = (*node)->next) {
        node = strcmp(list_entry(l, element_t, list)->value,
                      list_entry(r, element_t, list)->value) < 0
                   ? &l
                   : &r;
        (*node)->prev = prev;
        prev = *node;
        *ptr = *node;
        ptr = &(*ptr)->next;
    }

    *ptr = (l) ? l : r;
    (*ptr)->prev = prev;

    return head;
}

static struct list_head *mergesort(struct list_head *head)
{
    if (!head->next) {
        return head;
    }

    struct list_head *mid = head;
    for (struct list_head *fast = head->next; fast && fast->next;
         fast = fast->next->next) {
        mid = mid->next;
    }
    struct list_head *r_list = mergesort(mid->next);
    mid->next = NULL;
    struct list_head *l_list = mergesort(head);
    head = merge(l_list, r_list);

    return head;
}

void q_sort(struct list_head *head)
{
    if (!head || list_empty(head) || list_is_singular(head)) {
        return;
    }

    head->prev->next = NULL;
    struct list_head *sorted = mergesort(head->next);
    head->next = sorted;
    sorted->prev = head;

    while (sorted->next) {
        sorted = sorted->next;
    }
    sorted->next = head;
    head->prev = sorted;
}