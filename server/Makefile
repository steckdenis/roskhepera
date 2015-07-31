#
# libkorebot utils Makefile
#
#
.PHONY: clean

# Pointer to the libkorebot build directory
LIBKOREBOT = ${LIBKOREBOT_ROOT}/build-korebot-2.6
# Cross compiler
CC = arm-angstrom-linux-gnueabi-gcc

SRCS	= $(wildcard *.c)
OBJS	= $(patsubst %.c,%.o,${SRCS})
INCS	= -I ${LIBKOREBOT}/include
LIBS	= -L ${LIBKOREBOT}/lib -lkorebot
CFLAGS 	= -O2 -std=c99

TARGET	= server

.PHONY: all clean depend

server: $(OBJS)
	@echo "Building $@"
	@$(CC) -o $@ $? $(LIBS) $(INCS) $(CFLAGS)

server-static: $(OBJS)
	@echo "Building $@"
	@$(CC) -o $@ $? $(LIBS) -static $(INCS) $(CFLAGS)

all: 	${TARGET}

clean : 
	@echo "Cleaning"
	@rm -f *.o .depend ${TARGET} *~

depend:	
	@echo "Building dependencies"
	@rm -f .depend
	@touch .depend
	@makedepend ${SYS_INCLUDES} ${INCS} -Y -f .depend ${SRCS}

%.o:	%.c
	@echo "Compiling $@"
	$(CC) $(INCS) -c $(CFLAGS) $< -o $@

ifeq (.depend,$(wildcard .depend))
include .depend 
endif
