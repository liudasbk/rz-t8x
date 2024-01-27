NAME=asm_t8x
RZ_PLUGIN_PATH=$(shell rizin -H RZ_USER_PLUGINS)
LIBEXT=$(shell rizin -H LIBEXT)
CFLAGS=-g -fPIC $(shell pkg-config --cflags rz_asm)
LDFLAGS=-shared $(shell pkg-config --libs rz_asm)
OBJS=$(NAME).o
LIB=$(NAME).$(LIBEXT)

all: $(LIB)

clean:
	rm -f $(LIB) $(OBJS)

$(LIB): $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) $(OBJS) -o $(LIB)

install:
	cp -f asm_t8x.$(LIBEXT) $(RZ_PLUGIN_PATH)

uninstall:
	rm -f $(RZ_PLUGIN_PATH)/asm_t8x.$(LIBEXT)
