.PHONY: all, clean, release, debug, link
all: release

CC = gcc

CFLAGS_RELEASE = -O3
CFLAGS_DEBUG = -g -DWRAPPER_DEBUG

OUT = bin/wrapper-gcc
OTHER_COMPILER = bin/wrapper-g++ bin/wrapper-clang bin/wrapper-clang++

release: wrapper-gcc.c
	${CC} ${CFLAGS_RELEASE} -o ${OUT} $<
	strip ${OUT}

debug: wrapper-gcc.c
	${CC} ${CFLAGS_DEBUG} -o ${OUT} $<

link: ${OUT}
	for LINK in ${OTHER_COMPILER}; do ln -s $$(basename $<) $$LINK; done

clean:
	rm wrapper-gcc ${OTHER_COMPILER}


