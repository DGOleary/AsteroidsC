#include <stdbool.h>
#include "defaultfunctions.h"

//LinkedList function implementations

LinkedList *createLinkedList(){
    LinkedList *ret=(LinkedList*)malloc(sizeof(LinkedList));
    ret->length=0;
    ret->next=NULL;
    ret->value=NULL;
    return ret;
}

LinkedList *LinkedListAdd(LinkedList *list, void *value){
    if(list == NULL){
        return createLinkedList();
    }
    if(list->length==0){
        list->length=1;
        list->next=NULL;
        list->value=value;
        return list;
    }
    LinkedList *ret=(LinkedList*)malloc(sizeof(LinkedList));
    ret->next=list;
    ret->value=value;
    ret->length=list->length+1;
    return ret;
}

void *LinkedListPop(LinkedList **listptr){
    if(*listptr==NULL){
        return NULL;
    }
    LinkedList *list=*listptr;
    //stores the head's value
    void* val=list->value;
    //updates the head of the list to the next value
    *listptr=list->next;
    //frees the head of the list
    free(list); 
    list = NULL;
    return val;
}

Queue *createQueue(){
    Queue *ret=(Queue*)malloc(sizeof(Queue));
    ret->length=0;
    ret->next=NULL;
    ret->last=NULL;
    ret->value=NULL;
    return ret;
}

void QueueAdd(Queue *queue, void *value){
    if(queue->length==0){
        queue->length=1;
        queue->next=NULL;
        queue->last=queue;
        queue->value=value;
        return;
    }
    Queue *next=(Queue*)malloc(sizeof(Queue));
    next->length=0;
    next->next=NULL;
    next->last=next;
    next->value=value;
    queue->last->next=next;
    queue->last=next;
    queue->length++;
}

void *QueuePoll(Queue **queueptr){
    if(*queueptr==NULL){
        return NULL;
    }
    Queue *queue=*queueptr;
    //stores the head's value
    void* val=queue->value;
    if(queue->next!=NULL){
        queue->next->length=queue->length-1;
    }
    //updates the head of the list to the next value
    *queueptr=queue->next;
    //frees the head of the list
    free(queue); 
    queue = NULL;
    return val;
}

HashSet *createHashSet(){
    HashSet *ret = (HashSet*)malloc(sizeof(HashSet));
    //defaults value to 50
    ret->set = (LinkedList**)malloc(sizeof(LinkedList*)*25);
    ret->size = 25;
    ret->length = 0;
    //sets all elements in the array to a default value
    for(int i=0;i<25;i++){
        ret->set[i]=createLinkedList();
    }
    return ret;
}

//HashSet function implementations

void HashSetAdd(HashSet *set, void *add, int (*hashFunc)(HashSet*, void*)){
    if((set->length>=(set->size*7)/10)&&set->length>0){
        resizeHash(set, hashFunc);
    }
    //calls the user defines hashing function
    int hash=hashFunc(set, add);
    //insert into the linked list at that spot
    set->length=set->length+1;
    set->set[hash]=LinkedListAdd(set->set[hash], add);
}

bool HashSetContains(HashSet *set, void *item, int (*hashFunc)(HashSet*, void*), bool (*equalsFunc)(void*, void*)){
    //checks for null references
    if(set==NULL||set->set==NULL){
        return false;
    }
    //the LinkedList contained inside the HashSet
    LinkedList *temp = set->set[hashFunc(set, item)];
    //goes through the LinkedList to check for a match in the case multiple values have matched to it
    while(temp!=NULL){
        if(equalsFunc(temp->value, item)){
            return true;
        }
        temp=temp->next;
    }
    return false;
}

void resizeHash(HashSet *set, int (*hashFunc)(HashSet*, void*)){
    //puts size to an int for convenience
    int size=set->size;
    //Old array of items
    LinkedList **old=set->set;
    //new larger array
    LinkedList **ret = (LinkedList**)malloc(sizeof(LinkedList*)*(size*size));
    //initializes array
    for(int i=0;i<size*size;i++){
        ret[i]=createLinkedList();
    }
    //new HashSet of new size
    HashSet *newSize=createHashSet();
    newSize->size=size*size;
    //sets new HashSet
    newSize->set=ret;
    //goes through the original array and reinserts all values
    for(int i=0;i<size;i++){
        //if the value is null then its an empty space, should only reallocate when full but just incase
        if(old[i]->value==NULL){
            continue;
        }
        //loop through all items in the linkedlist 
        LinkedList *temp = old[i];
        while(temp!=NULL&&temp->length!=0){
            HashSetAdd(newSize, temp->value, hashFunc);
            //goes to the next spot and frees entry
            LinkedListPop(&temp);
        }

    }
    //frees the old array
    free(*set->set);
    *set=*newSize;
}

BinaryTreeNode *CreateBinaryTree(void *val){
    BinaryTreeNode *temp = (BinaryTreeNode*)malloc(sizeof(BinaryTreeNode));
    temp->value=val;
    temp->less=NULL;
    temp->more=NULL;
    return temp;
}

void AddBST(BinaryTreeNode *node, void *value, int (*compareFunc)(void*, void*)){
    //stores the comparison
    int comp = compareFunc(node->value, value);
    //if the value already exists don't add it, so only add new values
    if(comp<1){
        if(node->more==NULL){
            BinaryTreeNode *temp = CreateBinaryTree(value);
            node->more=temp;
        }else{
            AddBST(node->more, value, compareFunc);
        }
    }else if(comp>1){
        if(node->less==NULL){
            BinaryTreeNode *temp = CreateBinaryTree(value);
            node->less=temp;
        }else{
            AddBST(node->less, value, compareFunc);
        }
    }
}

bool BSTContains(BinaryTreeNode *node, void *value, int (*compareFunc)(void*, void*)){
    //if it makes it all the way through to a null end, then the value is no there and it's false
    if(node==NULL){
        return false;
    }
    //if the value isn't equal keep trying to go through
    int compVal = compareFunc(node->value, value);
    if(compVal != 0){
        if(compVal < 0){
            return BSTContains(node->more, value, compareFunc);
        }else{
            return BSTContains(node->less, value, compareFunc);
        }
       
    }else{
        //when the value is the same, it's true
        return true;
    }
}
