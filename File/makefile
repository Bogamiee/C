# Makefile

# 컴파일러 설정
CC = gcc

# 컴파일 플래그 설정
CFLAGS = -Wall -Wextra -O2

# 타겟 실행 파일 이름
TARGET = ls

# 소스 파일
SRC = main.c

# 기본 타겟
all: $(TARGET)

# 타겟 실행 파일 빌드 규칙
$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC)

# 'clean' 명령어를 통해 생성된 파일들을 삭제
clean:
	rm -f $(TARGET)
	clear
