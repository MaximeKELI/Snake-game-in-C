CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -O2
LDFLAGS = -lncurses
TARGET = snake
SRC = snake.c
TEST_TARGET = test_snake
TEST_SRC = test_snake.c

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC) $(LDFLAGS)

test: $(TEST_TARGET)

$(TEST_TARGET): $(TEST_SRC)
	$(CC) $(CFLAGS) -o $(TEST_TARGET) $(TEST_SRC)
	@echo "Tests compilés. Lancez ./$(TEST_TARGET) pour exécuter les tests."

clean:
	rm -f $(TARGET) $(TEST_TARGET) .snake_best_score .snake_top_scores

install: $(TARGET)
	@echo "Le jeu est compile. Lancez-le avec: ./$(TARGET)"

.PHONY: all test clean install

