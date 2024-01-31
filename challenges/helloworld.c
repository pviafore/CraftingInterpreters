#include <assert.h>
#include <stdlib.h>
#include <stdio.h>


typedef struct _DblLinkedList {
    struct _DblLinkedList* left;
    struct _DblLinkedList* right;
    int value;
} DblLinkedList;


DblLinkedList* DblCreate(int initialValue){
    DblLinkedList* mem = (DblLinkedList *)malloc(sizeof(DblLinkedList));
    assert(mem);
    mem->value = initialValue;
    mem->left = mem;
    mem->right = mem;
    return mem;
}


DblLinkedList* DblDelete(DblLinkedList* ll){
    if(!ll) {
        return ll;
    }
    DblLinkedList* returnValue = NULL;
    if(ll->left != ll){
        returnValue = ll->left;
        ll->left->right = ll->right;
        ll->right->left = ll->left;
    }
    free(ll);
    return returnValue;
}

void InsertToRight(DblLinkedList* ll, int newValue) {
    assert(ll);
    DblLinkedList* node = DblCreate(newValue);
    DblLinkedList* right = ll->right;
    ll->right = node;
    node->left = ll;
    right->left = node;
    node->right = right;

}

void DblFree(DblLinkedList* ll) {
    while(ll){
        ll = DblDelete(ll);
    }
}

DblLinkedList* DblFind(DblLinkedList* ll, int haystack) {
    assert(ll);
    DblLinkedList* node = ll;
    do {
        if(node->value == haystack) {
            return node;
        }
        node = node->right;
    } while(node != ll);
    return NULL;
}

int main() {
    printf("Hello, World\n");

    DblLinkedList* ll = DblCreate(5);
    InsertToRight(ll, 8);
    InsertToRight(ll, 3);
    InsertToRight(ll->left, 1);

    DblLinkedList* found = DblFind(ll, 8);
    printf("-> %d %d %d %d %d\n", found->value, found->right->value, found->right->right->value, found->right->right->right->value, found->right->right->right->right->value);
    printf("<- %d %d %d %d %d\n", found->value, found->left->value, found->left->left->value, found->left->left->left->value, found->left->left->left->left->value);

    DblFree(ll);

    return 0;
}