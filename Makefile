.PHONY: all, clean, release, debug, link
all: release

CC = gcc

CFLAGS_RELEASE = -O3
CFLAGS_DEBUG = -g -DWRAPPER_DEBUG

OTHER_COMPILER = wrapper-g++ wrapper-clang wrapper-clang++

release: wrapper-gcc.c
	${CC} ${CFLAGS_RELEASE} -o wrapper-gcc $<
	strip wrapper-gcc

debug: wrapper-gcc.c
	${CC} ${CFLAGS_DEBUG} -o wrapper-gcc $<

link: wrapper-gcc
	for LINK in ${OTHER_COMPILER}; do ln -s $< $$LINK; done

clean:
	rm wrapper-gcc ${OTHER_COMPILER}


