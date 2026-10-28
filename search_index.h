#ifndef SEARCH_INDEX_H
#define SEARCH_INDEX_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// --- STRUCTURI DE DATE ---

// Structura pentru un nod din lista de fisiere
typedef struct node_list {
    char *id;
    int scor;
    char **cuvinte_cheie;
    int nr_cuvinte;
    struct node_list *urm, *pre;
} File_Node;

// Structura pentru lista dublu inlantuita de fisiere
typedef struct list {
    File_Node *tail, *head;
} File_List;

// Structura pentru un nod din Trie (arborele de cuvinte)
typedef struct node_tree {
    struct node_tree* data[26];
    int is_end;
    File_Node** files;
    int nr_files;
} Tree;

// Structura pentru Max-Heap
typedef struct heap {
    int cap;
    int nr_files;
    File_Node **v;
} Heap;

// --- PROTOTIPURI FUNCTII DE INITIALIZARE ---
File_List* init_file_list();
Tree* init_tree();
Heap* initH(int capacitate);

// --- PROTOTIPURI FUNCTII AUXILIARE (TRIE & HEAP) ---
void insertTree(Tree *root, char *cuvant, File_Node *new_file);
void delete_from_tree(Tree **root, char *cuvant, int level, File_Node *file);
int compareFiles(File_Node *a, File_Node *b);
void swap(File_Node **a, File_Node **b);
void insertH(Heap *h, File_Node *file);
void heapify(Heap *h, int curent);
File_Node* extragere(Heap *h);
void find_prefix_files(Tree *tree, int *len, File_Node ***result, int *cap);

// --- PROTOTIPURI OPERATII PRINCIPALE ---
void ADD(File_List *file_list, Tree *tree, char *id, int scor, int nr_cuvinte, char **cuvinte, FILE *fout);
void DEL(File_List *file_list, Tree **tree, char *id, FILE *fout);
void ADDKW(File_List *file_list, Tree *tree, char *id, char *cuvant, FILE *fout);
void DELKW(File_List *file_list, Tree **tree, char *id, char *cuvant, FILE *fout);
void FIND(Tree *tree, char *cuvant, FILE *fout);
void TOPK(Tree *tree, char *cuvant, int k, FILE *fout);
void PRINT(Tree *tree, char *cuvant, int level, FILE *fout);
void PREFIX(Tree *tree, char *prefix, FILE *fout);

// --- PROTOTIPURI FUNCTII DE ELIBERARE A MEMORIEI ---
void free_tree(Tree *tree);
void free_file_list(File_List *file_list);

#endif