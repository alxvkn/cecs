SOURCE = $(wildcard ./*.c)
OBJECTS = $(patsubst %.c,%.o,$(SOURCE))

CFLAGS += $(addprefix -I,$(INCLUDE_DIRS))

LDLIBS = $(LIB_STATIC)

test: $(OBJECTS)
	$(CC) $(OBJECTS) $(LDFLAGS) $(LDLIBS) -o $@

%.o: %.c
	$(CC) -c $(CFLAGS) $<

clean:
	$(RM) $(OBJECTS) test

# always re-build test
.PHONY: clean test
