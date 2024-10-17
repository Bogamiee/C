// 1. ls -a 명령어 구현 완료
// 2. ls -l 명령어 구현 해야함
// 3. ls -R 명령어 구현 해야함

#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <unistd.h>

#define MAX_FILES 1024

int runLs(const int argc, const char **argv, const int w);
int getFileList(char **files, const char *path, const int isHidden, const int isLong, const int isRecursive);
DIR* getDir(const char *path);
int ascend(const void *a, const void *b);
int getRows(const char **files, const int fileCount, const int width);
int getMaxCols(const char **files, const int fileCount, const int rows);
int getMaxLen(const char **files, int st, int ed);
void printFiles(const char **files, const int rows, const int cols, int fileCount);

int main(int argc, char *argv[])
{
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);

    return runLs(argc, (const char **)argv, w.ws_col);
}

int runLs(const int argc, const char **argv, const int w)
{
    int isHidden = 0;
    int isLong = 0;
    int isRecursive = 0;
    int fileCount;
    const char *path = ".";
    char *files[MAX_FILES];

    for (int i = 1; i < argc; i++)
    {
        if (argv[i][0] == '-') // 옵션 처리
        {
            if (strchr(argv[i], 'a'))
            {
                isHidden = 1;
            }
            if (strchr(argv[i], 'l'))
            {
                isLong = 1;
            }
            if (strchr(argv[i], 'R'))
            {
                isRecursive = 1;
            }
        }
        else // 경로 처리
        {
            path = argv[i];
        }
    }

    fileCount = getFileList(files, path, isHidden, isLong, isRecursive);

    int rows = getRows((const char **)files, fileCount, w);
    int cols = getMaxCols((const char **)files, fileCount, rows);
    if (rows == cols) 
        printf("sadf");

    //printFiles((const char **)files, rows, cols, fileCount);

    for (int i = 0; i < fileCount; i++) {
        free(files[i]); // 동적으로 할당한 메모리 해제
    }
    
    return 0;
}

int getFileList(char **files, const char *path, const int isHidden, const int isLong, const int isRecursive)
{
    DIR *dir = getDir(path);
    struct dirent *entry; // 디렉토리 엔트리
    int fileCount = 0; // 파일 개수

    while ((entry = readdir(dir)) != NULL) // 디렉토리 엔트리 읽기
    {
        if (entry->d_name[0] == '.' && !isHidden) // 숨김 파일 제외
        {
            continue;
        }

        files[fileCount] = (char *)malloc((strlen(entry->d_name) + 1) * sizeof(char)); // 파일 이름 동적 메모리 할당
        if (files[fileCount] == NULL) // 동적 메모리 할당 실패
        {
            perror("malloc");
            closedir(dir);
            exit(1);
        }
        
        strcpy(files[fileCount], entry->d_name); // 파일 이름 복사
        fileCount++;
    }
    closedir(dir);

    qsort(files, fileCount, sizeof(char *), ascend); // 파일 이름 오름차순 정렬

    if (isRecursive) 
    {
        int newFileCount = 1;
        char *newPaths[MAX_FILES];
        dir = getDir(path);

        newPaths[0] = (char *)malloc((strlen(path) + 1) * sizeof(char));
        if (newPaths[0] == NULL)
        {
            perror("malloc");
            closedir(dir);
            exit(1);
        }
        strcpy(newPaths[0], path);

        while ((entry = readdir(dir)) != NULL)
        {
            if (entry->d_name[0] == '.' && !isHidden)
            {
                continue;
            }

            if (entry->d_type == DT_DIR && strcmp(entry->d_name, ".") && strcmp(entry->d_name, ".."))
            {
                char *newPath = (char *)malloc((strlen(path) + strlen(entry->d_name) + 1) * sizeof(char));
                if (newPath == NULL)
                {
                    perror("malloc");
                    closedir(dir);
                    exit(1);
                }
                strcpy(newPath, path);
                strcat(newPath, entry->d_name);

                newPaths[newFileCount] = newPath;
                newFileCount++;
            }
        }

        qsort(newPaths, newFileCount, sizeof(char *), ascend);

        printf("%d\n", newFileCount);
        for (int i = 0; i < newFileCount; i++)
        {
            printf("%s:\n", newPaths[i]);
            free(newPaths[i]);
        }

        closedir(dir);
    }

    if (isLong)
    {
        int i = isLong;
        i--;
    }

    return fileCount;
}

DIR* getDir(const char *path)
{
    DIR* dir = opendir(path);
    if (dir == NULL)
    {
        perror("opendir");
        exit(1);
    }
    return dir;
}

int ascend(const void *a, const void *b)
{
    const char *str1 = *(const char **)a;
    const char *str2 = *(const char **)b;

    if (str1[0] == '.') // .으로 시작하는 파일은 다음 문자부터 비교
        str1++;
    if (str2[0] == '.')
        str2++;

    return strcasecmp(str1, str2);
}

int getRows(const char **files, const int fileCount, const int width) // 현재 터미널 창 너비에 대한 최대 rows 반환
{
    int row = 1;
    while (getMaxCols(files, fileCount, row) > width)
    {
        row++;
    }
    return row;
}

int getMaxCols(const char **files, const int fileCount, const int rows) // 특정 rows에 대한 최대 cols 반환
{
    int maxCols = 0;
    int targetCols = (fileCount + rows - 1) / rows; // 나눗셈 올림
    int lastIndex, maxLen;

    for (int i = 0; i < targetCols; i++)
    {
        if ((i + 1) * rows < fileCount) 
            lastIndex = (i + 1) * rows;
        else 
            lastIndex = fileCount;
        maxLen = getMaxLen((const char **)files, i * rows, lastIndex);
        maxCols += maxLen + 2;
    }
    return maxCols;
}

int getMaxLen(const char **files, int st, int ed) // 파일 이름 중 가장 긴 길이 반환
{
    int max = 0;
    for (int i = st; i < ed; i++)
    {
        int len = strlen(files[i]);
        if (len > max) max = len;
    }
    return max;
}

void printFiles(const char **files, const int rows, const int cols, int fileCount) // 파일 이름 출력. Linux ls와 동일하게 출력
{
    int index, i;

    for (int m = 0; m < rows; m++)
    {
        for (int n = 0; n < cols; n++)
        {
            index = n * rows + m;        
            if (index < fileCount)
            {
                if ((n + 1) * rows < fileCount)
                    i = (n + 1) * rows;
                else
                    i = fileCount;
                printf("%-*s  ", getMaxLen(files, n * rows, i), files[index]);
            }
        }
    printf("\n");
    }
}
