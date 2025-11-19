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

void generateCodes(node* root, unsigned int code[], unsigned int top, char** codes){
    if(root == NULL) return;
    //se esiste un figlio sinistro aggiungiamo uno zero
    if(root->left){
        code[top]=0;
        generateCodes(root->left, code, top+1, codes); //se esiste un figlio sinistro chiamiamo ricorsivamente la funzione su di esso
    }
    //se esiste il figlio destro aggiungismo un 1 al codice
    if(root->right){
        code[top]=1;
        generateCodes(root->right, code, top+1, codes); //stessa cosa per il figlio destro
    }
    //se non esistono figli significa che siamo arrivati ad un carattere
    if(!(root->left) && !(root->right)){
        codes[root->c] = (char*)malloc((top + 1) * sizeof(char)); //alloca nell'array di stringhe codes una nuova stringa
        if(codes[root->c] == NULL){
            fprintf(stderr, "Allocazione fallita per codice.\n");
            exit(EXIT_FAILURE);
        }
        for(unsigned int i=0; i<top; i++){  //carica il codice in codes[nodo] carattere per carattere
            codes[root->c][i] = '0' + code[i];
        }
        codes[root->c][top] = '\0'; //seguito dal terminatore
    }
}

void compress(const char* newfile, char** codes, unsigned int freq[], char* input){
    FILE* out = fopen(newfile, "wb");
    if (out == NULL) {
        fprintf(stderr, "Errore nell'apertura del file di output %s\n", newfile);
        return; //error handling nell'apertura del file
    }
    int bitCount=0;
    unsigned char buffer=0;
    for(int i=0; input[i]!='\0'; i++){
        unsigned char c = (unsigned char)input[i]; //preleva dall'input un carattere
        char* code = codes[c]; //recupera il codice di quel carattere
    for(int j=0; code[j]!='\0';j++){
            int bit = code[j]-'0';
            if(bit==1){
                buffer |= (1<<(7-bitCount)); //shiftiamo il bit nella sua posizione
                /*7-0 che sarà il primo bit andrà in posizione 7, quindi darà il primo, ogni volta lo inseriremo nella posizione successiva del buffer*/
            }
            //per bit nullo è già inizializzato in automatico a 0;
            bitCount++; //incrementiamo il bitcount per la lettura del prossimo valore
            if(bitCount==8){ //arrivati ad 8 il byte è completo e possiamo scriverlo sul file
                fwrite(&buffer, sizeof(unsigned char), 1, out);
                buffer=0; //svuotiamo poi il buffer e azzeriamo il bit count per il prossimo carattere
                bitCount=0; 
            }
        }
    }
    if(bitCount>0){ //gestione di eventuali bit rimasti alla fine
        fwrite(&buffer, sizeof(unsigned char), 1, out);
    }
}

/*il file bin già si aspetta sequenze di bit (byte), quindi a noi basta scrivere i bit in
maniera corretta, bufferizzandoli come abbiamo visto, in modo da scrivere un byte alla volta*/

int main(int argc, char *argv[]){
    if(argc < 2){
        fprintf(stderr, "Usage: %s <filename>\n", argv[0]);
        return 1;
    }
    char *filename = argv[1];
    FILE *file = fopen(filename, "r"); //apre il file col nome specificato come parametro
    if(file == NULL){
        fprintf(stderr, "Errore nell'apertura del file %s\n", filename);
        return 1;
    }
    fseek(file, 0, SEEK_END); //consente di spostare il puntatore per la lettura
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET); //il puntatore viene spostato all'inizio
    // crea un buffer per il contenuto del file
    char *input = (char*)malloc(file_size + 1); //+1 lo usiamo per allocare un byte aggiuntivo per il terminatore di stringa 
    if(input == NULL){  //se il puntatore resta NULL non è avvenuta l'allocazione
        fprintf(stderr, "Memory allocation failed\n");
        fclose(file);
        return 1;
    }
    // Read file
    size_t read_size = fread(input, 1, file_size, file); //inserisce il contenuto de file nel buffer e restituisce la dimensione
    input[read_size] = '\0'; // inserisce il terminatore in ultima posizione
    fclose(file);   //chiusura del file
    unsigned int freq[NUM_CHARS] = {0}; //assegna il valore di frequenza a 0 per tutti i caratteri
    for(int i=0; input[i]!='\0'; i++){
        freq[(unsigned char)input[i]]++; //incrementiamo la frequenza di un valore ogni volta che viene individuato
    }
    node* root = buildHuffmanTree(freq); //costruiamo l'albero di Huffman
    unsigned int code[NUM_CHARS];  //array per memorizzare il codice
    char** codes = (char**)malloc(NUM_CHARS * sizeof(char*));
    if (codes == NULL) {
        fprintf(stderr, "Allocazione fallita per codes.\n");
        free(input);
        return 1;
    }
    for (int i = 0; i < NUM_CHARS; i++) {
        codes[i] = NULL;
    }
    generateCodes(root, code, 0, codes); //all'interno della funzione avviene il salvataggio oltre che la stampa
    const char* newfilename = "compressed.bin";
    compress(newfilename, codes, freq, input);
    for (int i = 0; i < NUM_CHARS; i++) {
        if (codes[i] != NULL) {
            free(codes[i]);
        }
    }
    free(codes); //libera l'array con le stringhe ottenute 
    free(input);    //libera il buffer
    return 0;
}
