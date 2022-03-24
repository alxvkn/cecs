PROJECT = cecs

INCLUDE_DIRS = ./dbg-macro/include
INCLUDE_DIRS += ./include

SOURCE_DIR = ./src
SOURCE = $(wildcard $(SOURCE_DIR)/*.c)

CFLAGS = $(addprefix -I,$(INCLUDE_DIRS))

LIB_STATIC = lib$(PROJECT).a

LDLIBS := $(addprefix -l,$(LDLIBS))

VPATH = $(SOURCE_DIR)

all: $(LIB_STATIC) test

$(LIB_STATIC): $(notdir $(patsubst %.c,%.o,$(SOURCE)))
	$(AR) rcs $@ $^

%.o: %.c
	$(CC) -c $(CFLAGS) $<

test: $(LIB_STATIC)
	$(MAKE) -C test INCLUDE_DIRS='$(addprefix ../,$(INCLUDE_DIRS))' LIB_STATIC=../$(LIB_STATIC)

clean:
	$(RM) $(PROJECT).o $(LIB_STATIC)
	$(MAKE) -C test clean

run-test: test
	@echo
	./test/test

.PHONY: all clean run-test test
