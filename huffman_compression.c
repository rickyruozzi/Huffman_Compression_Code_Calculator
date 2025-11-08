#include "./huffman_compression.h"

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

//le seguenti funzioni ci serviranno per trovare il valore minimo delle frequenze dei caratteri gestendole tramite un minHeap

void heapify(minHeap *heap, int i){
    int smallest = i;   //inizilamente il più piccolo è l'elemento passato come parametro
    int left = leftson(i);
    int right = rightson(i);
    if(left<heap->size && heap->data[left]->f < heap->data[smallest]->f){
        smallest=left;
    }
    if(right<heap->size && heap->data[right]->f < heap->data[smallest]->f){
        smallest=right; 
    }
    //controlliamo che non abbia figli più piccoli
    if(smallest!=i){
        swap(heap->data[i], heap->data[smallest]); //in caso scambiamo i nodi
        heapify(heap, smallest); //chiamiamo nuovamente heapify sulla nuova terna con padre smallest
    }
}

node* extractMin(minHeap *heap){
    if(heap->size==0){
        return NULL; //heap vuoto
    }
    node* min = heap->data[0]; //il minimo è sempre la radice se la proprietà è rispettata
    heap->data[0]=heap->data[heap->size-1]; //sostituiamo la radice con l'ultimo elemento
    heap->size--; //decrementiamo la dimensione dell'heap
    heapify(heap,0); //ripristiniamo la proprietà dell'heap
    return min;
}

void insertHeap(minHeap *heap, node *nodeTI){
    if(heap->size==NUM_CHARS){
        printf("Heap pieno, impossibile inserire il nodo\n");
        return;
    }
    heap->data[heap->size]=nodeTI;
    heap->size++;
    int i = heap->size - 1;

    while(i!=0 && heap->data[i]->f < heap->data[parent(i)]->f){
        swap(heap->data[i], heap -> data[parent(i)]);
        i=parent(i);
    }
}

//ora abbiamo finito le funzioni per gestire il minHeap

//vediamo ora come costruire l'albero per la codifica di Huffman

node* buildHuffmanTree(unsigned int freq[NUM_CHARS]){
    minHeap *heap = (minHeap*)malloc(sizeof(minHeap));
    if(heap == NULL){
        fprintf(stderr, "Allocazione della memoria fallita.\n");
        exit(EXIT_FAILURE);
    }
    heap->size=0;

    for(int i=0; i<NUM_CHARS; i++){
        if(freq[i]>0){
            node *newNode = create_node((unsigned char)i, freq[i]);
            insertHeap(heap, newNode);
        }
    }
    //ora costruiamo l'albero
    while(heap->size>1){
        node* left = extractMin(heap);
        node* right = extractMin(heap);
        node *useNode = create_node('\0', left->f + right->f);
        useNode->left = left;
        useNode->right = right;
        insertHeap(heap, useNode);
    }
    node * root = extractMin(heap); //la radice dell'albero
    free(heap);
    return root;
}

void printCodes(node* root, unsigned int code[], unsigned int top){
    if(root == NULL) return;
    //se esiste un figlio sinistro aggiungiamo uno zero
    if(root->left){
        code[top]=0;
        printCodes(root->left, code, top+1);
    }
    //se esiste il figlio destro aggiungismo un 1 al codice
    if(root->right){
        code[top]=1;
        printCodes(root->right, code, top+1);
    }
    //se non esistono figli significa che siam arrivati ad un carattere
    if(!(root->left) && !(root->right)){
        printf("Carattere: %c frequenza: %u codice: ", root->c, root->f);
        for(unsigned int i=0; i<top; i++){ //stampiamo il codice fino al punto top, questo perchè nel worst case potremmo avere NUM_CHARS-1 valori
            printf("%u", code[i]);
        }
        printf("\n");
    }
}

int main(int argc, char *argv[]){
    if(argc < 2){
        fprintf(stderr, "Usage: %s <string to encode>\n", argv[0]);
        return 1;
    }
    char *input = argv[1];
    unsigned int freq[NUM_CHARS] = {0}; //assegna il valore di frequenza a 0 per tutti i caratteri
    for(int i=0; input[i]!='\0'; i++){
        freq[(unsigned char)input[i]]++; //incrementiamo la frequenza di un valore ogni volta che viene individuato
    }
    node* root = buildHuffmanTree(freq); //costruiamo l'albero di Huffman
    unsigned int code[NUM_CHARS];  //array per memorizzare il codice
    printCodes(root, code, 0); //all'interno della funzione avviene il salvataggio oltre che la stampa
    return 0;
}
