#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "phonebook.h"

void printMenu() {
    printf("==================================================\n");
    printf("                    전화번호부                    \n");
    printf("==================================================\n");
    printf("                 1. 전화번호 추가                 \n");
    printf("                 2. 전화번호 삭제                 \n");
    printf("                 3. 전화번호 검색                 \n");
    printf("                 4. 전화번호 나열                 \n");
    printf("                 5. 종료                          \n");
    printf("==================================================\n\n");
}

void printList(Phonebook* list) {
    Phonebook* p = list;
    int i = 1;
  printf("\n====================== 목록 ======================\n\n");
    if (p == NULL) {
        printf("등록된 연락처가 없습니다.\n");
        return;
    }
    while (p != NULL) {
        printf("[%d] 이름: %s, 전화번호: %s\n", i++, p->name, p->num);
        p = p->link;
    }
    printf("\n");
}

void addList(Phonebook** list) {
    Phonebook* p = NULL;
    Phonebook* prev = NULL;
    char buffer[S_SIZE];
    char numBuffer[N_SIZE];

    while (1) {
        printf("이름(입력을 종료하려면 엔터): ");
        getInput(buffer, S_SIZE);
        if (buffer[0] == '\0') {
            printf("\n");
            break;
        }

        Phonebook* c = *list;
        while (c != NULL) {
            if (strcmp(c->name, buffer) == 0) {
                printf("\n이미 저장된 이름입니다.\n\n");
                return;
            }
            c = c->link;
        }

        p = (Phonebook*)malloc(sizeof(Phonebook));
        if (p == NULL) {
            fprintf(stderr, "메모리 할당 실패\n");
            break;
        }
        strcpy(p->name, buffer);

        do {
            printf("전화번호(010-0000-0000 형식): ");
            getInput(numBuffer, N_SIZE);
        } while (!isPhoneNumber(numBuffer));

        strcpy(p->num, numBuffer);
        p->link = NULL;

        if (*list == NULL) {
            *list = p;
        }
        else {
            prev = *list;
            while (prev->link != NULL) {
                prev = prev->link;
            }
            prev->link = p;
        }
        printf("연락처가 추가되었습니다: %s, %s\n\n", p->name, p->num);
    }
}

void deleteList(Phonebook** list) {
    Phonebook* p = *list;
    Phonebook* prev = NULL;
    char buffer[S_SIZE];

    printf("삭제할 이름을 입력하세요(취소하려면 엔터): ");
    getInput(buffer, S_SIZE);
    if (buffer[0] == '\0') {
        printf("\n");
        return;
    }

    while (p != NULL) {
        if (strcmp(p->name, buffer) == 0) {
            if (prev == NULL) // 삭제할 노드가 첫 번째일 경우
                *list = p->link;
            else
                prev->link = p->link;
            free(p);
            printf("\n%s의 연락처가 삭제되었습니다.\n\n", buffer);
            return;
        }
        prev = p;
        p = p->link;
    }

    printf("\n해당 이름의 연락처를 찾을 수 없습니다.\n\n");
}

void searchList(Phonebook** list) {
    Phonebook* p = *list;
    char buffer[S_SIZE];
    int found = 0;

    printf("검색할 이름을 입력하세요(취소하려면 엔터): ");
    getInput(buffer, S_SIZE);
    if (buffer[0] == '\0') return;

    while (p != NULL) {
        if (strcmp(p->name, buffer) == 0) {
            printf("\n이름: %s, 전화번호: %s\n\n", p->name, p->num);
            found = 1;
            break; // 중복된 이름이 있을 경우 첫 번째 것만 출력
        }
        p = p->link;
    }

    if (!found) {
        printf("\n해당 이름의 연락처를 찾을 수 없습니다.\n\n");
    }
}

void freeList(Phonebook* list) {
    Phonebook* p = list;
    Phonebook* next;

    while (p != NULL) {
        next = p->link;
        free(p);
        p = next;
    }
}

void getInput(char* buffer, int size) {
    if (fgets(buffer, size, stdin) != NULL) {
        buffer[strcspn(buffer, "\n")] = '\0'; // 개행 문자 제거

        if (strlen(buffer) == size -1 && buffer[size - 2] != '\0') {
            while (getchar() != '\n'); // 입력 버퍼 비우기
        }
    }
}

bool isPhoneNumber(const char* num) {
    if (strlen(num) != 13)
        return false;
    for (int i = 0; i < 13; i++) {
        if (i == 0 || i == 2) {
            if (num[i] != '0')
                return false;
        }
        else if (i == 1) {
            if (num[i] != '1')
                return false;
        }
        else if (i == 3 || i == 8) {
            if (num[i] != '-')
                return false;
        }
        else {
            if (num[i] < '0' || num[i] > '9')
                return false;
        }
    }
    return true;
}
