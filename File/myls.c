// myls.c
#include <dirent.h>
#include <grp.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#include "myls.h"

int runLs(const int argc, const char **argv)
{
    int isHidden = 0;
    int isLong = 0;
    int isRecursive = 0;
    int fileCount;
    const char *path = ".";
    char **files = malloc(CAPACITY * sizeof(char *));
    
    if (files == NULL) // 동적 메모리 할당 실패
    {
        perror("malloc");
        free(files);
        return -1;
    }

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

    if (isRecursive) // isRecursive = 1
    {
        recursiveDir(path, isHidden, isLong);
        printf("\033[A\33[2K\r");
    }
    else // isRecursive = 0
    {
        fileCount = getFileList(&files, path, isHidden);

        if (isLong) // isLong = 1
        {
            printInfo((const char **)files, (const char *)path, fileCount);
        }
        else
        {
            printFiles((const char **)files, fileCount);
        }

        for (int i = 0; i < fileCount; i++)
        {
            free(files[i]); // 동적으로 할당한 메모리 해제
        }
    }
    
    free(files); // 동적으로 할당한 메모리 해제
    return 0;
}

int getFileList(char ***files, const char *path, const int isHidden)
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

        if (fileCount >= CAPACITY) // 배열 크기가 부족한 경우
        {
            CAPACITY *= 2;
            char **temp = realloc(*files, CAPACITY * sizeof(char *));
            if (temp == NULL) // 동적 메모리 할당 실패
            {
                perror("realloc");
                closedir(dir);
                return -1;
            }
            *files = temp;
        }
        (*files)[fileCount] = malloc((strlen(entry->d_name) + 1) * sizeof(char)); // 파일 이름 동적 메모리 할당
        if ((*files)[fileCount] == NULL) // 동적 메모리 할당 실패
        {
            perror("malloc");
            closedir(dir);
            exit(1);
        }
        
        strcpy((*files)[fileCount], entry->d_name); // 파일 이름 복사
        fileCount++;
    }
    closedir(dir);

    qsort(*files, fileCount, sizeof(char *), ascend); // 파일 이름 오름차순 정렬

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

    if (str1[0] == '.' || str1[0] == '_') // .으로 시작하는 파일은 다음 문자부터 비교
    {
        str1++;
    }
    if (str2[0] == '.' || str2[0] == '_')
    {
        str2++;
    }

    return strcasecmp(str1, str2);
}

int getRows(const char **files, const int fileCount) // 현재 터미널 창 너비에 대한 최대 rows 반환
{
    int row = 1;
    if (getMaxLen(files, 0, fileCount) > terminalWidth)
    {
        return fileCount;
    }
    while (getMaxCols(files, fileCount, row) > terminalWidth)
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
        {
            lastIndex = (i + 1) * rows;
        }
        else 
        {
            lastIndex = fileCount;
        }
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
        if (len > max)
        {
            max = len;
        }
    }
    return max;
}

void printFiles(const char **files, const int fileCount) // 파일 이름 출력. Linux ls와 동일하게 출력
{
    int rows = getRows(files, fileCount);
    int cols = getMaxCols(files, fileCount, rows);
    int index, i;

    for (int m = 0; m < rows; m++)
    {
        for (int n = 0; n < cols; n++)
        {
            index = n * rows + m;        
            if (index < fileCount)
            {
                if ((n + 1) * rows < fileCount)
                {
                    i = (n + 1) * rows;
                }
                else
                {
                    i = fileCount;
                }
                if (rows != fileCount)
                {
                    printf("%-*s  ", getMaxLen(files, n * rows, i), files[index]);
                }
                else
                {
                    printf("%s", files[index]);
                }
            }
        }
        printf("\n");
    }
}

void recursiveDir(const char *path, const int isHidden, const int isLong)
{
    char **newPaths = malloc(CAPACITY * sizeof(char *));
    if (newPaths == NULL)
    {
        perror("malloc");
        return;
    }

    int newFileCount = getFileList(&newPaths, path, isHidden);
    
    printf("%s:\n", path);
    if (isLong) // isLong = 1
    {
        printInfo((const char **)newPaths, path, newFileCount);
    }
    else
    {
        printFiles((const char **)newPaths, newFileCount);
    }
    printf("\n");

    for (int i = 0; i < newFileCount; i++)
    {
        char *currentPath = allocatePath(path, newPaths[i]);

        DIR *subDir = opendir(currentPath);
        if (subDir != NULL)
        {
            // 하위 디렉토리인 경우 재귀 호출
            closedir(subDir);
            if (strcmp(newPaths[i], ".") != 0 && strcmp(newPaths[i], "..") != 0)
            {
                recursiveDir(currentPath, isHidden, isLong);
            }
        }
        free(currentPath);
        free(newPaths[i]);
    }
    free(newPaths);
}

void printFilePermissions(mode_t mode)
{
    printf("%c%c%c%c%c%c%c%c%c%c ",
        (S_ISREG(mode) ? '-' : (S_ISDIR(mode) ? 'd' : 
        (S_ISCHR(mode) ? 'c' : (S_ISBLK(mode) ? 'b' : 
        (S_ISFIFO(mode) ? 'p' : (S_ISLNK(mode) ? 'l' : 's')))))),
        (mode & S_IRUSR) ? 'r' : '-',
        (mode & S_IWUSR) ? 'w' : '-',
        (mode & S_IXUSR) ? 'x' : '-',
        (mode & S_IRGRP) ? 'r' : '-',
        (mode & S_IWGRP) ? 'w' : '-',
        (mode & S_IXGRP) ? 'x' : '-',
        (mode & S_IROTH) ? 'r' : '-',
        (mode & S_IWOTH) ? 'w' : '-',
        (mode & S_IXOTH) ? 'x' : '-');
}

void printInfo(const char **files, const char *path, const int fileCount)
{
    struct stat st;
    int maxNlink = 0;
    size_t maxUsr = 0, maxGrp = 0;
    int maxSize = 0;
    int totalBlocks = 0;

    for (int i = 0; i < fileCount; i++)
    {
        char *buf = allocatePath(path, files[i]);
        getFileStat(buf, &st);

        maxNlink = (getCountInt(st.st_nlink) > maxNlink) ? getCountInt(st.st_nlink) : maxNlink;
        struct passwd *pwd = getpwuid(st.st_uid);
        struct group *grp = getgrgid(st.st_gid);
        if (pwd == NULL || grp == NULL)
        {
            perror("getpwuid/getgrgid");
            exit(1);
        }
        maxUsr = (strlen(pwd->pw_name) > maxUsr) ? strlen(pwd->pw_name) : maxUsr;
        maxGrp = (strlen(grp->gr_name) > maxGrp) ? strlen(grp->gr_name) : maxGrp;
        maxSize = (getCountInt(st.st_size) > maxSize) ? getCountInt(st.st_size) : maxSize;

        totalBlocks += st.st_blocks;

        free(buf); // 할당된 메모리 해제
    }

    printf("total %d\n", totalBlocks / 2);

    for (int i = 0; i < fileCount; i++)
    {
        char *buf = allocatePath(path, files[i]);
        getFileStat(buf, &st);

        struct passwd *pwd = getpwuid(st.st_uid);
        struct group *grp = getgrgid(st.st_gid);
        struct tm *tm = localtime(&st.st_mtime);

        if (pwd == NULL || grp == NULL)
        {
            perror("getpwuid/getgrgid");
            exit(1);
        }

        printFilePermissions(st.st_mode);

        printf("%*ld ", maxNlink, st.st_nlink);
        printf("%-*s ", (int)maxUsr, pwd->pw_name);
        printf("%-*s ", (int)maxGrp, grp->gr_name);
        printf("%*ld ", maxSize, st.st_size);

        char timeBuf[80];
        strftime(timeBuf, sizeof(timeBuf), "%b %d %H:%M", tm);
        printf("%s ", timeBuf);

        printf("%s\n", files[i]);

        free(buf); // 할당된 메모리 해제
    }
}

int getCountInt(int a)
{
    int count = 0;
    if (a == 0) 
    {
        return 1;
    }
    while (a != 0)
    {
        a /= 10;
        count++;
    }
    return count;
}

char* allocatePath(const char *path, const char *file)
{
    char *buf = (char *)malloc((strlen(path) + strlen(file) + 2) * sizeof(char));
    if (buf == NULL)
    {
        perror("malloc");
        exit(1);
    }
    if (path[strlen(path) - 1] == '/')
    {
        sprintf(buf, "%s%s", path, file);
    }
    else
    {
        sprintf(buf, "%s/%s", path, file);
    }
    return buf;
}

void getFileStat(const char *buf, struct stat *st)
{
    if (lstat(buf, st) == -1)
    {
        perror("lstat");
        exit(1);
    }
}

