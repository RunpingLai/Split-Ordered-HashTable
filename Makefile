CC = gcc
CFLAGS = -Wall -pthread -g
TARGET = test
DEPS = data_structures.h so_list_hashtable.h lock_based_hashtable.h resizable_lk_hashtable.h
OBJ = test.o so_list_hashtable.o lock_based_hashtable.o resizable_lk_hashtable.o

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

$(TARGET): $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)

.PHONY: clean

clean:
	rm -f *.o $(TARGET)
