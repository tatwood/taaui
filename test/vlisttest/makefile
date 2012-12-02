EXE=../bin/vlisttest
EXED=../bin/vlisttestd
OBJS=obj/make.o
OBJSD=objd/make.o
INCLUDES=-I../../include -I../../../taamath/include -I../../../taasdk/include
LIBS=-lGL -L/usr/X11R6.4/lib -lX11
CC=gcc
CCFLAGS=-Wall -msse3 -O3 -fno-exceptions -DNDEBUG $(INCLUDES)
CCFLAGSD=-Wall -msse3 -O0 -ggdb2 -fno-exceptions -D_DEBUG $(INCLUDES)
LD=gcc
LDFLAGS=$(LIBS)

$(EXE): obj ../bin $(OBJS)
	$(LD) $(OBJS) $(LDFLAGS) -o $(EXE)

$(EXED): objd ../bin $(OBJSD)
	$(LD) $(OBJSD) $(LDFLAGS) -o $(EXED)

obj:
	mkdir obj

objd:
	mkdir objd

../bin:
	mkdir ../bin

obj/make.o : make.c
	$(CC) $(CCFLAGS) -c $< -o $@

objd/make.o : make.c
	$(CC) $(CCFLAGSD) -c $< -o $@

all: $(EXE) $(EXED)

clean:
	rm -rf $(EXE) $(EXED) obj objd

debug: $(EXED)

release: $(EXE)
