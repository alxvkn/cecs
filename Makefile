PROJECT = cecs

INCLUDE_DIRS += ./include # public
INCLUDE_DIRS += ./src/include # internal

SOURCE_DIR = ./src
SOURCE = $(wildcard $(SOURCE_DIR)/*.c)

CFLAGS = $(addprefix -I,$(INCLUDE_DIRS))
DEBUG_FLAGS = -g

LIB_STATIC = lib$(PROJECT).a

LDLIBS := $(addprefix -l,$(LDLIBS))

VPATH = $(SOURCE_DIR)

all: test

debug: CFLAGS += $(DEBUG_FLAGS)
debug: $(LIB_STATIC)

$(LIB_STATIC): $(notdir $(patsubst %.c,%.o,$(SOURCE)))
	$(AR) rcs $@ $^

%.o: %.c
	$(CC) -c $(CFLAGS) $<

test: debug
	CFLAGS='$(DEBUG_FLAGS)' $(MAKE) -C test INCLUDE_DIRS='$(addprefix ../,$(INCLUDE_DIRS))' LIB_STATIC=../$(LIB_STATIC)

clean:
	$(RM) $(PROJECT).o $(LIB_STATIC)
	$(MAKE) -C test clean

run-test: test
	@echo
	./test/test

.PHONY: all clean run-test test
