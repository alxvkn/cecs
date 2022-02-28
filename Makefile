CC = clang

PROJECT = cecs

all: $(PROJECT)

$(PROJECT): $(PROJECT).o
	$(CC) $^ -o $@

%.o: %.c
	$(CC) -c $<

clean:
	rm -f $(PROJECT).o $(PROJECT)

.PHONY: all clean
