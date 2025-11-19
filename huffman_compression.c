#include "./huffman_compression.h"
#include <string.h>

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
    /* scriviamo l'header delle frequenze (in modo che la decompressione possa ricostruire l'albero)
       Nota: usa lo stesso tipo e ordine di byte che leggerà decompress */
    if (fwrite(freq, sizeof(unsigned int), NUM_CHARS, out) != NUM_CHARS) {
        fprintf(stderr, "Errore nella scrittura dell'header delle frequenze su %s\n", newfile);
        fclose(out);
        return;
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
    fclose(out);
}

/*il file bin già si aspetta sequenze di bit (byte), quindi a noi basta scrivere i bit in
maniera corretta, bufferizzandoli come abbiamo visto, in modo da scrivere un byte alla volta*/

void freeTree(node* root) {
    if (root == NULL) return;
    freeTree(root->left);
    freeTree(root->right);
    free(root);
}

void decompress(const char* infile, const char* outfile) {
    FILE* in = fopen(infile, "rb");
    if (in == NULL) {
        fprintf(stderr, "Errore nell'apertura del file %s\n", infile);
        return;
    }
    FILE* out = fopen(outfile, "w");
    if (out == NULL) {
        fprintf(stderr, "Errore nell'apertura del file %s\n", outfile);
        fclose(in);
        return;
    }
    unsigned int freq[NUM_CHARS]; //array che conterrà le frequenze dei caratteri
    if (fread(freq, sizeof(unsigned int), NUM_CHARS, in) != NUM_CHARS) {
        fprintf(stderr, "Errore nella lettura dell'header delle frequenze da %s\n", infile);
        fclose(in);
        fclose(out);
        return;
    }
    node* root=buildHuffmanTree(freq);
    if(root==NULL){
        perror("impossibile creare l'albero delle codifiche nella decompressione.");
        exit(EXIT_FAILURE);
        fclose(in);
        fclose(out);
        return;
    }
    node* current=root;  //imposta il nodo corrente sulla radice
    unsigned char byte;
    int position=0;
    while(fread(&byte, sizeof(unsigned char),1,in)==1){
        for(int k=0; k<8; k++){
            int bit = (byte >> k) & 1;  
            /*spostando in primma posizione il bit e mettendolo in and logico con 11111111 otteniamo
            il valore del bit*/
            if(bit==0){
                current=current->left;
            }
            else{
                current=current->right;
            } //discende l'albero fino ad una foglia
            if(current->left==NULL && current->right==NULL){
                fputc(current->c,out); //raggiunta la foglia usiamo fputc per inserire un carattere nel file alla posizione puntata
                current=root; //reimpostiamo il nodo corrente sulla radice
            }
        }
    }
    fclose(in);
    fclose(out);
    freeTree(root);
}


int main(int argc, char *argv[]){
    if (argc < 3) {
        fprintf(stderr, "Usage: %s -c <input_file> <output_file> or %s -d <compressed_file> <output_file>\n", argv[0], argv[0]);
        return 1;
    }

    if (strcmp(argv[1], "-c") == 0) {
        // Compressione
        if (argc != 4) {
            fprintf(stderr, "Usage for compression: %s -c <input_file> <output_file>\n", argv[0]);
            return 1;
        }
        char *input_file = argv[2];
        char *output_file = argv[3];

        FILE *file = fopen(input_file, "rb");
        if (file == NULL) {
            fprintf(stderr, "Errore nell'apertura del file %s\n", input_file);
            return 1;
        }
        fseek(file, 0, SEEK_END);
        long file_size = ftell(file);
        fseek(file, 0, SEEK_SET);
        char *input = (char*)malloc(file_size + 1);
        if (input == NULL) {
            fprintf(stderr, "Memory allocation failed\n");
            fclose(file);
            return 1;
        }
        size_t read_size = fread(input, 1, file_size, file);
        input[read_size] = '\0';
        fclose(file);

        unsigned int freq[NUM_CHARS] = {0};
        for (int i = 0; input[i] != '\0'; i++) {
            freq[(unsigned char)input[i]]++;
        }
        node* root = buildHuffmanTree(freq);
        unsigned int code_arr[NUM_CHARS];
        char** codes = (char**)malloc(NUM_CHARS * sizeof(char*));
        for (int i = 0; i < NUM_CHARS; i++) {
            codes[i] = NULL;
        }
        generateCodes(root, code_arr, 0, codes);
        compress(output_file, codes, freq, input);
        for (int i = 0; i < NUM_CHARS; i++) {
            if (codes[i] != NULL) {
                free(codes[i]);
            }
        }
        free(codes);
        freeTree(root);
        free(input);
    } else if (strcmp(argv[1], "-d") == 0) {
        // Decompressione
        if (argc != 4) {
            fprintf(stderr, "Usage for decompression: %s -d <compressed_file> <output_file>\n", argv[0]);
            return 1;
        }
        char *compressed_file = argv[2];
        char *output_file = argv[3];
        decompress(compressed_file, output_file);
    } else {
        fprintf(stderr, "Invalid option. Use -c for compression or -d for decompression.\n");
        return 1;
    }

    return 0;
}
