CC = gcc
CFLAGS = -Wall -O2 -std=c99 -Iraylib/include
LDFLAGS = -Lraylib/lib -lraylib -lopengl32 -lgdi32 -lwinmm

SRC = src/main.c src/game.c src/snake.c src/food.c
OBJ = $(SRC:.c=.o)
RANDOM_ID := $(shell echo %RANDOM%)
EXEC = snake_game_$(RANDOM_ID).exe

all: $(EXEC) copy_dll

$(EXEC): $(OBJ)
	$(CC) -o $@ $^ $(LDFLAGS)

copy_dll:
	@if not exist raylib.dll copy raylib\lib\raylib.dll .

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	del /Q src\*.o snake_game*.exe
