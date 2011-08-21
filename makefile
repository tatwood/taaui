LIB=lib/libtaaui.a
LIBD=lib/libtaauid.a
OBJS=obj/font.o obj/ui.o obj/uidrawlist.o obj/uirender.o obj/uitheme.o
OBJSD=obj/font.d.o obj/ui.d.o obj/uidrawlist.d.o obj/uirender.d.o \
obj/uitheme.d.o
INCLUDES=-Iinclude -I../taasdk/include
CC=gcc
CCFLAGS=-Wall -O2 -fno-exceptions -DNDEBUG -Dtaa_RENDER_GL11 $(INCLUDES)
CCFLAGSD=-Wall -O0 -ggdb2 -fno-exceptions -DDEBUG -Dtaa_RENDER_GL11 $(INCLUDES)
AR=ar
ARFLAGS=rs

vpath %.c src

$(LIB): obj lib $(OBJS)
	$(AR) $(ARFLAGS) $@ $(OBJS)

$(LIBD): obj lib $(OBJSD)
	$(AR) $(ARFLAGS) $@ $(OBJSD)

obj:
	mkdir obj

lib:
	mkdir lib

obj/%.o : %.c
	$(CC) $(CCFLAGS) -c $< -o $@

obj/%.d.o : %.c
	$(CC) $(CCFLAGSD) -c $< -o $@

all: $(LIB) $(LIBD)

clean:
	rm -rf $(LIB) $(LIBD) obj
