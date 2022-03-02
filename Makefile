CC = clang

PROJECT = cecs

CFLAGS = -Idbg-macro/include

all: $(PROJECT)

$(PROJECT): $(PROJECT).o
	$(CC) $^ -o $@

%.o: %.c
	$(CC) -c $(CFLAGS) $<

clean:
	rm -f $(PROJECT).o $(PROJECT)

.PHONY: all clean
