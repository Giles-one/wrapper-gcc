.PHONY: all, clean, release, debug, link
all: release

CC = gcc

CFLAGS_DEBUG = -g -DWRAPPER_LOG
CFLAGS_RELEASE = -O3 -DWRAPPER_LOG

OUT = bin/wrapper-gcc
BCSDCC = sbin/bcsdcc
OTHER_COMPILER = bin/cc bin/c++ bin/gcc bin/g++ bin/clang bin/clang++

# critical step
update:
	echo $$(pwd) > ~/.wrapper-gcc

release: wrapper-gcc.c update
	$(CC) $(CFLAGS_RELEASE) -o $(OUT) $<
	strip $(OUT)

debug: wrapper-gcc.c update
	$(CC) $(CFLAGS_DEBUG) -o $(OUT) $<

link: $(OUT) update
	for LINK in $(OTHER_COMPILER); do \
		ln -s $$(basename $<) $$LINK; \
	done
	ln -s ../$(OUT) $(BCSDCC)

clean:
	rm ${OUT} ${BCSDCC} ${OTHER_COMPILER}
