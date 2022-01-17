/*
 * linkedlist.c
 *
 * Based on the implementation approach described in "The Practice 
 * of Programming" by Kernighan and Pike (Addison-Wesley, 1999).
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ics.h"
#include "emalloc.h"
#include "listy.h"


node_t *new_node(event_t *val) {
    assert( val != NULL);

    node_t *temp = (node_t *)emalloc(sizeof(node_t));

    temp->val = val;
    temp->next = NULL;

    return temp;
}


node_t *add_front(node_t *list, node_t *new) {
    new->next = list;
    return new;
}


node_t *add_end(node_t *list, node_t *new) {
    node_t *curr;

    if (list == NULL) {
        new->next = NULL;
        return new;
    }

    for (curr = list; curr->next != NULL; curr = curr->next);
    curr->next = new;
    new->next = NULL;
    return list;
}


node_t *peek_front(node_t *list) {
    return list;
}


node_t *remove_front(node_t *list) {
    if (list == NULL) {
        return NULL;
    }

    return list->next;
}



void apply(node_t *list,
           void (*fn)(node_t *list, void *),
           void *arg)
{
    for ( ; list != NULL; list = list->next) {
        (*fn)(list, arg);
    }
}

// new additions, im assumming the announcment saying we cannot edit these was an error
// either way, here are my files

/*
usage: comapring two events according to when they start, using strcmp

inputs: e1 - event one
		e2 - event two

returned: negative number  if e1 < e2 (earlier start date)
		  positive number  if e1 > e2 (later start date)
		  zero			   if e1 = e2 (start on the same date)
*/
int compare(const event_t *e1, const event_t *e2){
	return strcmp(e1->dtstart, e2->dtstart);
}

/* 
usage:    when i have a sorted list, this will let me insert a node into while still keeping it sorted
inputs:   list  - this is our sourted list	
		  new_node - new node to be added
		   
returned: after running it will give us back our new list
	
from:     geeksforgeeks.org/given-a-linked-list-which-is-sorted-how-will-you-insert-in-sorted-way
*/
node_t *sorted_insert(node_t *list, node_t *new_node){
	
	node_t *cur;
	// list is empty or our instert is smaller than first node of list, we put our new node at the beggining
	if (list == NULL || compare(new_node->val, list->val) < 0){
		new_node->next = list; // sets new_node as first node (and maybe only one) 
		return new_node; 
	}
	
	cur = list;
	// finds the right place to insert the given node
	// once the place is found we move past the while loop
	while (cur->next != NULL && compare(cur->next->val, new_node->val) < 0){
		//moves to the next node
		cur = cur->next;
	}
	new_node->next = cur->next;
	cur->next = new_node;
	return list;
}








