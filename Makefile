PROJECT = cecs

INCLUDE_DIRS = ./dbg-macro/include
INCLUDE_DIRS += ./include

SOURCE_DIR = ./src
SOURCE = $(wildcard $(SOURCE_DIR)/*.c)

CFLAGS = $(addprefix -I,$(INCLUDE_DIRS))

LIB_STATIC = lib$(PROJECT).a

LDLIBS = cecs
LDFLAGS = -L.

LDLIBS := $(addprefix -l,$(LDLIBS))

VPATH = $(SOURCE_DIR)

all: $(LIB_STATIC) test

$(LIB_STATIC): $(notdir $(patsubst %.c,%.o,$(SOURCE)))
	$(AR) rcs $@ $^

%.o: %.c
	$(CC) -c $(CFLAGS) $<

test: test.o $(LIB_STATIC)
	$(CC) $^ $(LDFLAGS) $(LDLIBS) -o $@

clean:
	$(RM) $(PROJECT).o $(PROJECT) $(LIB_STATIC) test.o test

run-test: test
	@echo
	./test

.PHONY: all clean run-test
