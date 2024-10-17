#include <stdio.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <dirent.h>
#include <string.h>
#include <stdlib.h>

#define MAX_FILES 1024

int getTotalLen(char *files[], int ed, int rows);
int getMaxLen(char *files[], int st, int ed);
int asc(const void *a, const void *b); // 오름차순 정렬
void printInfo(const char *path, const struct dirent *entry);
void list_dir(const char *path, int width, int isHidden, int isLong, int isRecursive);

int main(int argc, char *argv[]) {
    struct winsize w; // 터미널 창 크기 구조체
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w); // 터미널 창 크기 구조체 초기화

    int isHidden = 0;
    int isLong = 0;
    int isRecursive = 0;
    const char *path = "."; // 기본 경로는 현재 디렉토리

    for (int i = 1; i < argc; i++) {
        if (argv[i][0] == '-') {
            if (strchr(argv[i], 'a')) {
                isHidden = 1;
            }
            if (strchr(argv[i], 'l')) {
                isLong = 1;
            }
            if (strchr(argv[i], 'R')) {
                isRecursive = 1;
            }
        } else {
            path = argv[i];
        }
    }

    list_dir(path, w.ws_col, isHidden, isLong, isRecursive);

    
    return 0;
}

int getTotalLen(char *files[], int ed, int rows) {
    int total = 0, n;
    int cols = (ed + rows - 1) / rows; // 행의 수에 따른 열의 수 계산
                                       
    for (int col = 0; col < cols; col++) {
        n = (col + 1) * rows < ed ? (col + 1) * rows : ed;
        int max_len = getMaxLen(files, col * rows, n);
        total += max_len + 2; // 파일 이름과 공백 포함한 길이
    }

    return total;
}

int getMaxLen(char *files[], int st, int ed) {
    int max = 0;
    for (int i = st; i < ed; i++) {
        int len = strlen(files[i]);
        if (len > max) {
            max = len;
        }
    }
    return max;
}

int asc(const void *a, const void *b) { // 오름차순 정렬
    return strcasecmp(*(const char **)a, *(const char **)b);
}

void printInfo(const char *path, const struct dirent *entry) {}

void list_dir(const char *path, int width, int isHidden, int isLong, int isRecursive) {
    DIR *dir = opendir(path);
    char *files[MAX_FILES];
    struct dirent *entry;
    int count = 0;
    size_t len;
 
    if (dir == NULL) {
        perror("opendir");
        return;
    }

    while ((entry = readdir(dir)) != NULL) {
        if (!isHidden && entry->d_name[0] == '.') {
            continue;
        }

        if (isLong) {
            printInfo(path, entry);               
        }
        if (entry->d_name[0] != '.') {
            len = strlen(entry->d_name) + 1;
            files[count] = (char *)malloc(len * sizeof(char));
            if (files[count] == NULL) {
                perror("malloc");
                closedir(dir);
                return;
            }
            strcpy(files[count], entry->d_name);
            count++;
        }
    }
    
    qsort(files, count, sizeof(char *), asc);
    
    int rows = 1;
    while (getTotalLen(files, count, rows) > width) {
        rows++;
    }

    int cols = (count + rows - 1) / rows;
    for (int r = 0; r < rows; r++) {
        for (int c = 0; c < cols; c++) {
            int idx = c * rows + r;
            if (idx < count) {
                int k = (c + 1) * rows < count ? (c + 1) * rows : count;
                printf("%-*s  ", getMaxLen(files, c * rows, k), files[idx]);
            }
        }
        printf("\n");
    }

    for (int i = 0; i < count; i++) {
        free(files[i]);
    }

    closedir(dir);
}

/*
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <string.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>

void list_dir(const char *path, int show_hidden, int long_format, int recursive);

void print_file_info(const char *path, const struct dirent *entry) {
    struct stat file_stat;
    char full_path[1024];
    snprintf(full_path, sizeof(full_path), "%s/%s", path, entry->d_name);

    if (stat(full_path, &file_stat) == -1) {
        perror("stat");
        return;
    }

    // 파일 권한 출력
    printf((S_ISDIR(file_stat.st_mode)) ? "d" : "-");
    printf((file_stat.st_mode & S_IRUSR) ? "r" : "-");
    printf((file_stat.st_mode & S_IWUSR) ? "w" : "-");
    printf((file_stat.st_mode & S_IXUSR) ? "x" : "-");
    printf((file_stat.st_mode & S_IRGRP) ? "r" : "-");
    printf((file_stat.st_mode & S_IWGRP) ? "w" : "-");
    printf((file_stat.st_mode & S_IXGRP) ? "x" : "-");
    printf((file_stat.st_mode & S_IROTH) ? "r" : "-");
    printf((file_stat.st_mode & S_IWOTH) ? "w" : "-");
    printf((file_stat.st_mode & S_IXOTH) ? "x" : "-");

    // 링크 수, 소유자, 그룹, 파일 크기, 수정 시간 출력
    printf(" %ld", (long)file_stat.st_nlink);
    printf(" %s", getpwuid(file_stat.st_uid)->pw_name);
    printf(" %s", getgrgid(file_stat.st_gid)->gr_name);
    printf(" %5ld", (long)file_stat.st_size);

    char time_buf[80];
    strftime(time_buf, sizeof(time_buf), "%b %d %H:%M", localtime(&file_stat.st_mtime));
    printf(" %s", time_buf);

    // 파일 이름 출력
    printf(" %s\n", entry->d_name);
}

void list_dir(const char *path, int show_hidden, int long_format, int recursive) {
    DIR *dir;
    struct dirent *entry;

    if ((dir = opendir(path)) == NULL) {
        perror("opendir");
        return;
    }

    while ((entry = readdir(dir)) != NULL) {
        if (!show_hidden && entry->d_name[0] == '.') {
            continue; // 숨김 파일을 표시하지 않음
        }

        if (long_format) {
            print_file_info(path, entry);
        } else {
            printf("%s  ", entry->d_name);
        }
    }
    if (!long_format) {
        printf("\n");
    }

    closedir(dir);

    // 재귀적으로 하위 디렉토리 탐색
    if (recursive) {
        dir = opendir(path);
        while ((entry = readdir(dir)) != NULL) {
            if (entry->d_type == DT_DIR && strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
                char next_path[1024];
                snprintf(next_path, sizeof(next_path), "%s/%s", path, entry->d_name);
                printf("\n%s:\n", next_path);
                list_dir(next_path, show_hidden, long_format, recursive);
            }
        }
        closedir(dir);
    }
}

int main(int argc, char *argv[]) {
    int show_hidden = 0;
    int long_format = 0;
    int recursive = 0;
    const char *path = "."; // 기본 경로는 현재 디렉토리

    // 옵션 파싱
    for (int i = 1; i < argc; i++) {
        if (argv[i][0] == '-') {
            if (strchr(argv[i], 'a')) {
                show_hidden = 1;
            }
            if (strchr(argv[i], 'l')) {
                long_format = 1;
            }
            if (strchr(argv[i], 'R')) {
                recursive = 1;
            }
        } else {
            path = argv[i];
        }
    }

    list_dir(path, show_hidden, long_format, recursive);
    return 0;
}
*/
