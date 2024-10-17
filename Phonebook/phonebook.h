#ifndef PHONEBOOK_H
#define PHONEBOOK_H
#include <stdbool.h>

#define S_SIZE 50
#define N_SIZE 14

typedef struct Phonebook {
    char name[S_SIZE];
    char num[N_SIZE];
    struct Phonebook* link;
} Phonebook;

void printMenu();
void printList(Phonebook* list);
void addList(Phonebook** list);
void deleteList(Phonebook** list);
void searchList(Phonebook** list);
void freeList(Phonebook* list);
void getInput(char* buffer, int size);
bool isPhoneNumber(const char* num);

#endif // PHONEBOOK_H
