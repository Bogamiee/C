#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include "phonebook.h"

int main(void) {
    Phonebook* list = NULL;
    int sel;

    while (1) {
        printMenu();
        printf("메뉴 선택: ");
        scanf("%d", &sel);
        getchar();

        switch (sel) {
            case 1:
                printf("\n==================================================\n");
                addList(&list);
                break;
            case 2:
                printf("\n==================================================\n");
                deleteList(&list);
                break;
            case 3:
                printf("\n==================================================\n");
                searchList(&list);
                break;
            case 4:
                printList(list);
                break;
            case 5:
                printf("프로그램을 종료합니다.\n");
                freeList(list);
                return 0;
            default:
                printf("유효하지 않은 선택입니다. 다시 선택해주세요.\n\n");
                break;
        }
    }
}
