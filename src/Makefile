CC = gcc
CFLAGS = -Wall -Wextra -std=c99
TARGET = farm_game.exe
OBJS = main.o player.o map.o utils.o

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(TARGET)

main.o: main.c
	$(CC) $(CFLAGS) -c main.c -o main.o

player.o: player.c
	$(CC) $(CFLAGS) -c player.c -o player.o

map.o: map.c
	$(CC) $(CFLAGS) -c map.c -o map.o

utils.o: utils.c
	$(CC) $(CFLAGS) -c utils.c -o utils.o

clean:
	rm -f $(OBJS) $(TARGET)
