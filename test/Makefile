SOURCE = $(wildcard ./*.c)
OBJECTS = $(patsubst %.c,%.o,$(SOURCE))

CFLAGS += $(addprefix -I,$(INCLUDE_DIRS))

LDLIBS = SDL2 m
LDFLAGS += $(addprefix -l,$(LDLIBS))

test: $(OBJECTS)
	$(CC) $(OBJECTS) $(LDFLAGS) $(LIB_STATIC) -o $@

%.o: %.c
	$(CC) -c $(CFLAGS) $<

clean:
	$(RM) $(OBJECTS) test

# always re-build test
.PHONY: clean test
