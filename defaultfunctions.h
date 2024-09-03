#ifndef DEFAULT_FUNCTIONS_H
#define DEFAULT_FUNCTIONS_H
#include <stdlib.h>
#include <stdio.h>

//LinkedList functions and data

//struct for a linked list object
typedef struct LinkedList LinkedList;

struct LinkedList{
    //pointer to next object in list
    LinkedList *next;
    //generic holder for the value
    void *value;
    //size of the list
    int length;
};

//creates a new LinkedList
LinkedList *createLinkedList();

//function to add to the front of the LinkedList
LinkedList *LinkedListAdd(LinkedList *list, void *value);

//function to pop the head of the list off
void *LinkedListPop(LinkedList **listptr);

//HashSet functions and data

typedef struct {
    //array that contains all the objects in LinkedLists, to chain hashes
    LinkedList **set;
    //size of the array
    int size;
    //amount of items in the array 
    int length;
} HashSet;

//function that creates and initializes a HashSet
HashSet *createHashSet();

//function to add to a HashSet, takes in a user defined hashing function
void HashSetAdd(HashSet *set, void *add, int (*hashFunc)(HashSet*, void*));

//function to get item from a HashSet, returns NULL if not present, uses a user defined equality function for the specific type
bool HashSetContains(HashSet *set, void *item, int (*hashFunc)(HashSet*, void*), bool (*equalsFunc)(void*, void*));

//function that resizes a HashSet, takes in the HashSet to add to and a function that defines how to hash an object
void resizeHash(HashSet *set, int (*hashFunc)(HashSet*, void*));

//Tree functions and data

typedef struct BinaryTreeNode BinaryTreeNode;

struct BinaryTreeNode{
    void *value;
    //pointers to children
    BinaryTreeNode *less;
    BinaryTreeNode *more;
};

//function to create a BinaryTree
BinaryTreeNode *CreateBinaryTree(void *val);

//adds to a BinaryTreeNode in a BST manner
void AddBST(BinaryTreeNode *node, void *value, int (*compareFunc)(void*, void*));

//checks if an item is in a BST
bool BSTContains(BinaryTreeNode *node, void *value, int (*compareFunc)(void*, void*));

#endif // DEFAULT_FUNCTIONS_H