
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "search_index.h"

// initializarea unei liste de fisiere
File_List* init_file_list() {
    File_List *file_list = (File_List *)malloc(sizeof(File_List));

    file_list->head = NULL;
    file_list->tail = NULL;

    return file_list;
}

// initializarea unui arbore
Tree* init_tree() {
    Tree *tree = (Tree *)malloc(sizeof(Tree));
    tree->is_end = 0;
    tree->nr_files = 0;
    tree->files = NULL;

    for(int i = 0; i < 26; i++) {
        tree->data[i] = NULL;
    }
    return tree;
}

// initializarea unui heap
Heap* initH(int capacitate) {
    Heap *h = (Heap *)malloc(sizeof(Heap));
    h->nr_files = 0;
    h->cap = capacitate;
    h->v = (File_Node **)malloc(sizeof(File_Node *) * capacitate);
    return h;
}

// inserez intr-un arbore un cuvant si ii adaug referinta catre fisier la nodul terminal
void insertTree(Tree *root,char *cuvant, File_Node *new_file) {
    Tree *curent = root;
    int len = strlen(cuvant);

    // parcurg fiecare litera din cuvant si verific daca a fost populata anterior
    for(int i = 0; i < len; i++) {
        int index = cuvant[i] - 'a';

        // daca la un anumit index data este NULL atunci imi aloc un subarbore pornind de la litera respectiva
        // si cobor pe acea litera
        if(curent->data[index] == NULL) 
            curent->data[index] = init_tree();
        curent = curent->data[index];
    }

    // am marcat ca am ajuns la un nod terminal
    curent->is_end = 1;

    // incrementez numarul de fisiere si realloc memorie
    curent->nr_files++;
    curent->files = realloc(curent->files,curent->nr_files * sizeof(File_Node *));

    // adaug fisierul in files in ordine lexicografica 
    int j = curent->nr_files - 2; // pornesc de la penultimul element
    while(j >= 0 && strcmp(curent->files[j]->id, new_file->id) > 0) {
        curent->files[j + 1] = curent->files[j];
        j--;
    }
    curent->files[j + 1] = new_file;
}


void ADD(File_List *file_list, Tree *tree, char *id, int scor,int nr_cuvinte, char **cuvinte, FILE *fout) {
    File_Node *curent = file_list->head;
    
    // verific daca exista deja in lista
    while(curent != NULL) {
        if(strcmp(curent->id,id) == 0) {
            fprintf(fout,"EXISTS\n");
            return;
        }
        curent = curent->urm;
    }

    // imi creez un fisier nou
    File_Node *new_file = (File_Node *)malloc(sizeof(File_Node));

    new_file->id = (char *)malloc(sizeof(char) * (strlen(id) + 1));
    strcpy(new_file->id, id);
    new_file->scor = scor;
    new_file->nr_cuvinte = 0;
    new_file->cuvinte_cheie = NULL;

    // inserez fisierul in lista dublu inlantuita
    new_file->urm = NULL;
    if(file_list->head == NULL) {
        new_file->pre = NULL;
        file_list->head = new_file;
        file_list->tail = new_file;
    } else {
        new_file->pre = file_list->tail;
        file_list->tail->urm = new_file;
        file_list->tail = new_file;
    }

    // procesez fiecare cuvant in parte
    for(int i = 0; i < nr_cuvinte; i++) {
        // imi salvez cuvantul si verific daca a mai aparut
        char *cuvant = cuvinte[i];

        int duplicat = 0;
        for(int j = 0; j < new_file->nr_cuvinte; j++) {
            if( strcmp(new_file->cuvinte_cheie[j], cuvant) == 0) {
                duplicat = 1;
            }
        }

        // daca nu este duplicat, numarul de cuvinte creste, realoc memorie pentru array-ul de pointeri si il adaug
        // apoi apelez functia pentru a insera in arbore cuvantul
        if(duplicat == 0) {
            new_file->nr_cuvinte++;
            new_file->cuvinte_cheie = realloc(new_file->cuvinte_cheie,sizeof(char *) * new_file->nr_cuvinte);
            new_file->cuvinte_cheie[new_file->nr_cuvinte - 1] = (char *)malloc(sizeof(char) * (strlen(cuvant) + 1));
            strcpy(new_file->cuvinte_cheie[new_file->nr_cuvinte - 1], cuvant);

            insertTree(tree, cuvant, new_file);
        }
    }
    
    fprintf(fout, "OK\n");
}

void delete_from_tree(Tree **root, char *cuvant, int level, File_Node *file){
    if(*root == NULL) return;

    // parcurg recursiv arborele pana la capatul cuvantului
    if(level < strlen(cuvant)) {
        int index = cuvant[level] - 'a';
        delete_from_tree(&((*root)->data[index]),cuvant, level + 1, file);
    } else {
        for(int i = 0; i < (*root)->nr_files; i++) {
            if( (*root)->files[i] == file) {
                // shiftez toate referintele la stanga si scad numarul de fisiere
                for(int j = i; j < (*root)->nr_files - 1; j++) 
                    (*root)->files[j] = (*root)->files[j+1];
                
                (*root)->nr_files--;
                break;
            }
        }

        // daca nu mai sunt fisiere atunci nodul nu mai este terminal si trebuie eliminat daca nu mai are fii;
        if((*root)->nr_files == 0) {
            (*root)->is_end = 0;
            free((*root)->files);
            (*root)->files = NULL;
        }
    }

    // cand se intoarce din recursie verifica pentru fiecare nod curent daca este terminal (ca sa pot sa ii dau free)
    
    // daca nu este nod terminal verific daca mai are fii, iar daca nu mai are fii ii dau free;
    if( (*root)->is_end == 0) {
        int has_child = 0;
        for(int i = 0; i < 26; i++)
            if((*root)->data[i] != NULL)
                has_child = 1;
        
        if(has_child == 0) {
            free(*root);
            *root = NULL;
        }
    }
}

void DEL(File_List *file_list, Tree **tree, char *id, FILE *fout) {
    File_Node *curent = file_list->head;
    int gasit = 0;
    // verific daca exista in lista
    while(curent != NULL) {
        if(strcmp(curent->id,id) == 0) {
            gasit = 1;
            break;
        }
        curent = curent->urm;
    }
    if(gasit == 0) {
        fprintf(fout, "NOT FOUND\n");
        return;
    }

    // sterg toate referintele fisierului din toate cuvintele sale
    for(int i = 0; i < curent->nr_cuvinte; i++) {
        delete_from_tree(tree, curent->cuvinte_cheie[i], 0, curent);
    }

    // fac legaturile astfel incat sa elimin nodul din lista de fisiere
    if(curent->pre == NULL)
        file_list->head = curent->urm; // cazul in care acesta este head
    else
        curent->pre->urm = curent->urm;
    
    if(curent->urm == NULL)
        file_list->tail = curent->pre; // cazul in care acesta este tail
    else
        curent->urm->pre = curent->pre;

    //dau free la memoria alocata
    for(int i = 0; i < curent->nr_cuvinte; i++)
        free(curent->cuvinte_cheie[i]);
    free(curent->cuvinte_cheie);
    free(curent->id);
    free(curent);

    fprintf(fout, "OK\n");
}

void ADDKW(File_List *file_list, Tree *tree, char *id, char *cuvant, FILE *fout) {
    File_Node *curent = file_list->head;
    int gasit = 0;
    // verific daca fisierul exista in lista
    while(curent != NULL) {
        if(strcmp(curent->id,id) == 0) {
            gasit = 1;
            break;
        }
        curent = curent->urm;
    }
    // mesaj de eroare
    if(gasit == 0) {
        fprintf(fout, "NOT FOUND\n");
        return;
    }

    // verific daca este duplicat si printez mesajul OK
    for(int i = 0; i < curent->nr_cuvinte; i++) {
        if(strcmp(curent->cuvinte_cheie[i],cuvant) == 0){
            fprintf(fout, "OK\n");
            return;
        }
    }

    // in caz ca nu e duplicat il adaug in lista
    curent->nr_cuvinte++;
    curent->cuvinte_cheie = realloc(curent->cuvinte_cheie, sizeof(char *) * curent->nr_cuvinte);
    curent->cuvinte_cheie[curent->nr_cuvinte - 1] = (char *)malloc(sizeof(char) * (strlen(cuvant) + 1));
    strcpy(curent->cuvinte_cheie[curent->nr_cuvinte - 1], cuvant);

    // adaug cuvantul in arbore folosind functia insertTree
    insertTree(tree, cuvant, curent);
    fprintf(fout, "OK\n");
}

void DELKW(File_List *file_list, Tree **tree, char *id, char *cuvant, FILE *fout) {
    File_Node *curent = file_list->head;
    int gasit = 0;
    // verific daca fisierul exista in lista
    while(curent != NULL) {
        if(strcmp(curent->id,id) == 0) {
            gasit = 1;
            break;
        }
        curent = curent->urm;
    }
    if(gasit == 0) {
        fprintf(fout, "NOT FOUND\n");
        return;
    }

    // caut cuvantul in lista si il elimin din lista de cuvinte cheie
    int found = 0;
    for(int i = 0; i < curent->nr_cuvinte; i++) {
        if(strcmp(curent->cuvinte_cheie[i],cuvant) == 0){
            found = 1;
            // dau free la memorie si shifrez la stanga pointerii
            free(curent->cuvinte_cheie[i]);

            for(int j = i; j < curent->nr_cuvinte - 1; j++) {
                curent->cuvinte_cheie[j] = curent->cuvinte_cheie[j+1];
            }
        
            curent->nr_cuvinte--;
            break;
        }
    }

    // daca nu s-a gasit cuvantul dar fisierul exista comanda este valida
    if(found == 0){
        fprintf(fout, "OK\n");
        return;
    }

    // sterg cuvantul din arbore
    delete_from_tree(tree, cuvant, 0, curent);
    fprintf(fout ,"OK\n");
}

void FIND(Tree *tree, char *cuvant, FILE *fout) {
    Tree *curent = tree;
    int len = strlen(cuvant);

    // parcurg arborele ca sa ajung la nodul terminal al cuvantului
    for(int i = 0 ; i < len; i++) {
        int index = cuvant[i] - 'a';

        // daca valoarea din arbore de la index este null atunci cuvantul nu exista si se afiseaza mesajul aferent
        if(curent->data[index] == NULL) {
            fprintf(fout, "EMPTY\n");
            return;
        }
        curent = curent->data[index];
    }

    // daca am gasit cuvantul, verific daca este terminal.
    //  In caz pozitiv printez numarul de fisiere si fisierele referite acelui cuvant
    if(curent->is_end == 1) {
        fprintf(fout, "%d",curent->nr_files);
        for(int i = 0; i < curent->nr_files; i++)
            fprintf(fout, " %s",curent->files[i]->id);
        fprintf(fout,"\n");
    } else 
        fprintf(fout,"EMPTY\n");
}

// functie de comparare a 2 fisiere pentru maxHeap
int compareFiles(File_Node *a, File_Node *b) {
    // compar scorul celor 2 fisiere si returnez pentru a forma un maxHeap
    if (a->scor > b->scor)
        return 1;
    if (a->scor < b->scor) 
        return 0;

    // daca am ajuns aici atunci scorul este egal si trebuie
    // comparate lexicografic
    if (strcmp(a->id, b->id) < 0) 
        return 1;
    
    return 0;
}

// swap a doua fisiere
void swap(File_Node **a, File_Node **b) {
    File_Node *temp = *a;
    *a = *b;
    *b = temp;
}

// functie de inserare a unui fisier in heap
void insertH(Heap *h, File_Node *file) {
    if(h->nr_files >= h->cap) 
        return;

    // pun fisierul pe ultima pozitie
    int i = h->nr_files;
    h->v[i] = file;
    h->nr_files++;

    // urc elementul cat timp este mai mare decat parintele sau
    while(i > 0 && compareFiles(h->v[i], h->v[(i-1)/2]) == 1) {
        swap( &h->v[i], &h->v[(i-1)/2]);
        i = (i-1)/2;
    }
}

// functie de organizare a heap-ului in maxHeap
void heapify(Heap *h, int curent) {
    int left = 2 * curent + 1; 
    int right = 2 * curent + 2;

    int indexMin = curent;

    // verific daca exista fiul stang si compar
    if(left < h->nr_files && compareFiles(h->v[left],h->v[indexMin]) == 1) {
        indexMin = left;
    } 
    // verific daca exista fiul drept si compar
    if(right < h->nr_files && compareFiles(h->v[right],h->v[indexMin]) == 1) {
        indexMin = right;
    }

    // daca cel mai mare nu este parintele atunci fac swap
    // si se continua in jos
    if(indexMin != curent) {
        swap( &h->v[curent], &h->v[indexMin]);
        heapify( h, indexMin);
    }
}

// functie de extragere a elementului maxim din Heap
File_Node* extragere(Heap *h) {
    if( h == NULL || h->nr_files == 0) {
        return NULL;
    }

    // imi salvez maximul (care este radacina)
    File_Node *rez = h->v[0];

    // inlocuiesc radacina cu ultimul element
    h->v[0] = h->v[h->nr_files - 1];
    h->nr_files--;

    // reorganizez heapul folosind functia heapify
    if(h->nr_files > 0) {
        heapify(h, 0);
    }

    return rez;
}

void TOPK(Tree *tree, char *cuvant, int k, FILE *fout) {
    Tree *curent = tree;
    int len = strlen(cuvant);
    
    // parcurg arborele pentru a ajunge in nodul terminal
    // al cuvantului pe care il caut
    for(int i = 0 ; i < len; i++) {
        int index = cuvant[i] - 'a';
        if(curent->data[index] == NULL) {
            fprintf(fout, "EMPTY\n"); // daca nu am cuvant returnez empty
            return;
        }
        curent = curent->data[index];
    }

    if (curent->is_end == 0 || curent->nr_files == 0) {
        fprintf(fout, "EMPTY\n");
        return;
    }
    // imi initializez un heap cu o capacitate suficienta
    Heap *h = initH( curent->nr_files + 1);

    // imi inserez toate fisierele din cuvant in heap
    for(int i = 0; i < curent->nr_files; i++) {
        insertH(h,curent->files[i]);
    }
    
    // gasesc maximul de cuvinte pe care le pot printa
    int max;
    if(curent->nr_files < k) {
        fprintf(fout, "%d", curent->nr_files);
        max = curent->nr_files;
    }else{
        fprintf(fout, "%d", k);
        max = k;
    }

    // imi extrag cate un fisier din heap si ii printez id-ul
    for(int i = 0; i < max; i++) {
        File_Node *file = extragere(h);
        fprintf(fout, " %s", file->id);
    }
    fprintf(fout,"\n");
    free(h->v);
    free(h);
}

// parcurgere in adancime a arborelui
void PRINT(Tree *tree, char *cuvant, int level, FILE *fout) {
    if(tree == NULL) {
        return;
    }
    // daca am ajuns la capatul unui cuvant ii pun terminatorul de sir si afisez informatiile cerute
    if(tree->is_end == 1) {
        cuvant[level] = '\0';

        fprintf(fout, "%s %d", cuvant,tree->nr_files);
        for(int i = 0; i < tree->nr_files; i++) {
            fprintf(fout, " %s",tree->files[i]->id);
        }
        fprintf(fout,"\n");
    }

    // daca nu este capatul unui cuvant atunci ma duc in toti fii nodului pentru a forma cuvintele urmatoare in bufferul 'cuvant'
    for(int i = 0 ; i < 26; i++) {
        if(tree->data[i] != NULL) {
            cuvant[level] = 'a' + i; // adaug litera in cuvant si apelez functia mergand pe urmatorul nivel
            PRINT(tree->data[i], cuvant, level + 1, fout);
        }
    }
    
}

// parcurgere in adancime a arborelui
void find_prefix_files(Tree *tree, int *len,File_Node ***result,int *cap) {
    if(tree == NULL)
        return;

    // daca am ajuns la capatul unui cuvant il adaug in pointerul de fisiere
    if(tree->is_end == 1) {
        for(int i = 0; i < tree->nr_files; i++){
            // verific fiecare fisier daca este duplicat
            int dup = 0;
            for(int j = 0; j < *len; j++) {
                if((*result)[j] == tree->files[i]){
                    dup = 1;
                    break;
                }
            }
            // daca am dat de un fisier nou, verific cata memorie este alocata si realoc
            // in caz de nevoie, apoi adaug fisierul in pointer
            if(dup == 0) {
                if(*len >= *cap) {
                    *cap = (*cap) * 2;
                    *result = realloc(*result, sizeof(File_Node *) * (*cap));
                }

                // folosesc insertion sort pentru a adauga fisierul in pointer in functie de id
                // la printare acestea trebuie sa fie sortate in ordine lexicografica
                int j = (*len) - 1;
                while(j >= 0 && strcmp((*result)[j]->id, tree->files[i]->id) > 0) {
                    (*result)[j + 1] = (*result)[j];
                    j--;
                }
                (*result)[j+1] = tree->files[i];
                (*len)++;
            }
        }
    }

    // daca nu este capatul unui cuvant atunci ma duc in toti fii nodului 
    // pentru a adauga fisierele urmatoare
    for(int i = 0 ; i < 26; i++) {
        if(tree->data[i] != NULL) {
            find_prefix_files(tree->data[i], len, result, cap);
        }
    }
    
}

void PREFIX(Tree *tree, char *prefix, FILE *fout) {
    Tree *curent = tree;
    int len = strlen(prefix);

    // parcurg arborele pentru a ajunge in nodul
    // unde se afla ultimul caracter al prefixului
    for(int i = 0; i < len; i++) {
        int index = prefix[i] - 'a';

        if(curent->data[index] == NULL) {
            fprintf(fout, "EMPTY\n"); // daca nu am cuvant returnez empty
            return;
        }
        curent = curent->data[index];
    }
    
    // imi aloc dinamic un pointer de pointeri la structura de File_Node
    // pentru a stoca fisierele care contin prefixul
    int cap = 10; // capacitate maxima momentana
    File_Node **rez = (File_Node **)malloc(sizeof(File_Node *) * cap);
    int len_rez = 0;

    // in level voi avea si lungimea
    find_prefix_files(curent, &len_rez,&rez, &cap);

    // daca lungimea pointerului este 0 atunci nu am fisiere
    if (len_rez == 0) {
        fprintf(fout, "EMPTY\n");
        free(rez);
        return;
    }

    // printez lungimea si id-ul fisierelor ce contin prefix-ul
    fprintf(fout, "%d", len_rez);
    
    for (int i = 0; i < len_rez; i++)
        fprintf(fout, " %s", rez[i]->id);
    fprintf(fout, "\n");
    free(rez);
}

//functii de eliberare de memorie
void free_tree(Tree *tree) {
    if(tree == NULL)
        return;
    
    for(int i = 0; i < 26; i++)
        free_tree(tree->data[i]);
    
    // elibereaza doar array-ul files, File_node - urile vor fi eliberate
    // de functia free_file_list
    if(tree->files != NULL)
        free(tree->files);
    
    free(tree);
}

void free_file_list(File_List *file_list) {
    File_Node *curent = file_list->head;
    while(curent != NULL) {
        File_Node *temp = curent;
        curent = curent->urm;
        for(int i = 0 ; i < temp->nr_cuvinte; i++)
            free(temp->cuvinte_cheie[i]);
        free(temp->cuvinte_cheie);
        free(temp->id);
        free(temp);
    }
    free(file_list);
}

int main() {
    FILE *fin = fopen("indexare.in", "r");
    FILE *fout = fopen("indexare.out", "w");

    File_List *file_list = init_file_list();
    Tree *word_tree = init_tree();

    int nr_operatii;
    fscanf(fin, "%d",&nr_operatii);

    char command[30];
    for(int i = 0; i < nr_operatii; i++) {
        fscanf(fin, "%s", command);

        if(strcmp(command, "ADD") == 0) {
            char id[200];
            fscanf(fin, "%s", id);

            int scor;
            fscanf(fin, "%d", &scor);

            int nr_cuvinte;
            fscanf(fin, "%d", &nr_cuvinte);

            char cuvinte[1000][1001];
            char *pointer_cuvinte[1000];
            for(int j = 0; j < nr_cuvinte; j++){
                fscanf(fin,"%s",cuvinte[j]);
                pointer_cuvinte[j] = cuvinte[j];
            }
            ADD( file_list,word_tree,id,scor,nr_cuvinte,pointer_cuvinte, fout);
        } else if(strcmp(command, "DEL") == 0) {
            char id[100];
            fscanf(fin, "%s", id);

            // daca sterg toate fisierele dintr-un arbore atunci word_tree == NULL
            // asa ca trebuie sa ii dau init din nou
            DEL( file_list, &word_tree, id, fout);
            if (word_tree == NULL) 
                word_tree = init_tree();

        } else if(strcmp(command, "ADDKW") == 0) {
            char id[100];
            fscanf(fin,"%s", id);

            char cuvant[100];
            fscanf(fin,"%s", cuvant);
            
            ADDKW( file_list, word_tree, id, cuvant, fout);
        } else if(strcmp(command, "DELKW") == 0) {
            char id[100];
            fscanf(fin,"%s", id);

            char cuvant[100];
            fscanf(fin,"%s", cuvant);
            
            DELKW( file_list, &word_tree, id, cuvant, fout);

            // daca sterg toate fisierele dintr-un arbore atunci word_tree == NULL
            // asa ca trebuie sa ii dau init din nouxs
            if (word_tree == NULL) 
                word_tree = init_tree();

        } else if(strcmp(command, "FIND") == 0) {
            char cuvant[100];
            fscanf(fin,"%s", cuvant);
            
            FIND( word_tree, cuvant, fout);
        } else if(strcmp(command, "TOPK") == 0) {
            char cuvant[100];
            fscanf(fin,"%s", cuvant);

            int k;
            fscanf(fin,"%d", &k);

            TOPK(word_tree, cuvant, k, fout);
        } else if(strcmp(command, "PRINT") == 0) {
            char cuvant[200];
            
            // verific daca arborele este gol
            int not_empty = 0;
            for(int i = 0 ; i < 26; i++){
                if(word_tree->data[i] != NULL)
                    not_empty = 1;
            }

            if(not_empty == 1)
                PRINT(word_tree, cuvant, 0, fout);
            else
                fprintf(fout,"EMPTY\n");
        } else if(strcmp(command, "PREFIX") == 0) {
            char prefix[200];
            fscanf(fin, "%s", prefix);

            PREFIX( word_tree, prefix, fout);
        }
    }

    free_tree(word_tree);
    free_file_list(file_list);

    fclose(fin);
    fclose(fout);
}