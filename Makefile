CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -O2
LDFLAGS = -lncurses
TARGET = snake
SRC = snake.c

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC) $(LDFLAGS)

clean:
	rm -f $(TARGET) .snake_best_score

install: $(TARGET)
	@echo "Le jeu est compile. Lancez-le avec: ./$(TARGET)"

.PHONY: all clean install

