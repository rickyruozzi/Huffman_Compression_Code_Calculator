#ifndef HUFFMAN_COMPRESSION_H
#define HUFFMAN_COMPRESSION_H
#define NUM_CHARS 256 // codificheremo tutti i caratteri ASCII
#endif

#include <stdio.h>
#include <stdlib.h>

typedef struct node{
    unsigned char c; // carattere codificato
    unsigned int f; // frequenza del carattere
    struct node* left; // puntatore al figlio sinistro
    struct node* right; // puntatore al figlio destro
}node;

typedef struct minHeap{
    unsigned int size;
    node *data[NUM_CHARS]; // array di puntatori ai nodi per l'albero
}minHeap;

//creazione di un nuovo nodo
node* create_node(unsigned char C, unsigned int F){
    node* newNode = (node*)malloc(sizeof(node));
    if (newNode == NULL) {
        fprintf(stderr, "Allocazione della memoria fallita.\n");
        exit(EXIT_FAILURE);
    }
    newNode->c = C;
    newNode->f = F;
    newNode->left = NULL;
    newNode->right = NULL;
    return newNode;
}

//scambio di due nodi
void swap(node* a, node* b){
    node temp = *a;
    *a = *b;
    *b = temp;
}

int parent(int i) {
    if (i == 0)
        return -1; // la radice non ha padre
    return (i - 1) / 2;
}

int leftson(int i) {
    return 2 * i + 1;
}

int rightson(int i) {
    return 2 * i + 2;
}

void heapify(minHeap* heap, int i);
node* extractMin(minHeap* heap);
void insertHeap(minHeap* heap, node* node);

node* buildHuffmanTree(char chars[], int freqs[], int n);