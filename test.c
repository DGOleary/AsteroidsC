#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include "drawfunctions.h"
#include "SDL2/SDL.h"
#include "SDL2/SDL_image.h"
#include "defaultfunctions.h"

int intHash(HashSet *set, void *add){
    //array of primes for each value in the Boundary struct
    int prime = 23;
    //initial prime to seed function 
    int seed = 11;
    //combines all the values into one hash mod the size 
    int hash = (seed * (prime + *(int*)add)) % set->size;
    return hash;
}

bool intEquality(void *num1, void *num2){
    if(num1==NULL||num2==NULL){
        return false;
    }
    return *(int*)num1==*(int*)num2;
}

int intCompare(void *num1, void *num2){
    return *(int*)num1 - *(int*)num2;
}

int main(int argc, char *argv[]){
    // LinkedList *test=createLinkedList();
    // int num=5;
    // printf("%d len\n", test->length);
    // int *nump=&(int){5};
    // test=LinkedListAdd(test, nump);
    // nump=&(int){6};
    // test=LinkedListAdd(test, nump);
    // nump=&(int){10};
    // test=LinkedListAdd(test, nump);
    // nump=&(int){50};
    // test=LinkedListAdd(test, nump);
    // printf("length %d\n", test->length);
    // while(test!=NULL){
    //     printf("pop: %d\n",*(int*)LinkedListPop(&test));
    // }
    // int *temp=(int*)LinkedListPop(&test);
    // printf("hi1\n");
    // printf("%d\n", *temp);
    // temp=(int*)LinkedListPop(&test);
    // printf("hi2\n");
    // printf("%d\n", *temp);
    // temp=(int*)LinkedListPop(&test);
    // printf("hi3\n");
    // printf("%d\n", *temp);
    // temp=(int*)LinkedListPop(&test);
    // printf("hi4\n");
    // printf("%d\n", *temp);
    // temp=(int*)LinkedListPop(&test);
    // printf("hi5\n");
    // printf("%d\n", *temp==NULL);

    // HashSet *hash = createHashSet();
    // HashSetAdd(hash, &(int){5}, intHash);
    // HashSetAdd(hash, &(int){15}, intHash);
    // HashSetAdd(hash, &(int){556}, intHash);
    // HashSetAdd(hash, &(int){84865}, intHash);
    // HashSetAdd(hash, &(int){546546}, intHash);
    // HashSetAdd(hash, &(int){51646068}, intHash);
    // printf("TESTING 11111 !!!!!!!!! \n");
    // printf("contain %d\n",HashSetContains(hash, &(int){5}, intHash, intEquality));
    // printf("contain %d\n",HashSetContains(hash, &(int){15}, intHash, intEquality));
    // printf("contain %d\n",HashSetContains(hash, &(int){556}, intHash, intEquality));
    // printf("contain %d\n",HashSetContains(hash, &(int){84865}, intHash, intEquality));
    // printf("contain %d\n",HashSetContains(hash, &(int){546546}, intHash, intEquality));
    // printf("contain %d\n",HashSetContains(hash, &(int){51646068}, intHash, intEquality));
    // resizeHash(hash, intHash);
    // printf("TESTING !!!!!!!!! \n");
    // printf("contain %d\n",HashSetContains(hash, &(int){5}, intHash, intEquality));
    // printf("contain %d\n",HashSetContains(hash, &(int){15}, intHash, intEquality));
    // printf("contain %d\n",HashSetContains(hash, &(int){556}, intHash, intEquality));
    // printf("contain %d\n",HashSetContains(hash, &(int){84865}, intHash, intEquality));
    // printf("contain %d\n",HashSetContains(hash, &(int){546546}, intHash, intEquality));
    // printf("contain %d\n",HashSetContains(hash, &(int){51646068}, intHash, intEquality));

    // BinaryTreeNode *bst = CreateBinaryTree(&(int){10});
    // printf("%d\n", *(int*)bst->value);
    // AddBST(bst, &(int){1}, intCompare);
    // AddBST(bst, &(int){15}, intCompare);
    // AddBST(bst, &(int){456}, intCompare);
    // AddBST(bst, &(int){58}, intCompare);
    // AddBST(bst, &(int){0}, intCompare);
    // AddBST(bst, &(int){-1}, intCompare);
    // AddBST(bst, &(int){100}, intCompare);
    // AddBST(bst, &(int){25}, intCompare);
    // printf("pass add\n");
    // printf("more %d\n", *(int*)bst->more->value);
    // printf("less %d\n", *(int*)bst->less->value);
    // printf("%d\n",BSTContains(bst, &(int){15}, intCompare));
    // printf("%d\n",BSTContains(bst, &(int){500000}, intCompare));

    Queue *queue = createQueue();
    QueueAdd(queue, &(int){1});
    QueueAdd(queue, &(int){2});
    QueueAdd(queue, &(int){3});
    Queue *temp = queue;
    while(temp != NULL){
        printf("%d\n", *(int*)temp->value);
        temp=temp->next;
    }
    printf("%d\n", *(int*)QueuePoll(&queue));
    printf("%d\n", *(int*)queue->value);
}

