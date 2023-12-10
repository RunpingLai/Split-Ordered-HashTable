CC = gcc
CFLAGS = -Wall -pthread -g
TARGET = main
DEPS = data_structures.h so_list_hashtable.h
OBJ = main.o so_list_hashtable.o

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

$(TARGET): $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)

.PHONY: clean

clean:
	rm -f *.o $(TARGET)
