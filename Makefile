CC      ?= cc
CFLAGS  ?= -std=c11 -O2 -Wall -Wextra -Iinc
LDLIBS  ?= -lm

.PHONY: all test demo clean fixture

all: test_layout test_fixture_v1 demo_layout update_main

test_layout: test/test_layout.c src/ui.c inc/ui.h inc/ui_platform.h inc/ui_host.h
	$(CC) $(CFLAGS) -o $@ test/test_layout.c src/ui.c $(LDLIBS)

test_fixture_v1: test/test_fixture_v1.c src/ui.c inc/ui.h inc/ui_platform.h inc/ui_host.h
	$(CC) $(CFLAGS) -o $@ test/test_fixture_v1.c src/ui.c $(LDLIBS)

demo_layout: example/demo.c src/ui.c inc/ui.h
	$(CC) $(CFLAGS) -o $@ example/demo.c src/ui.c $(LDLIBS)

update_main: example/update_main.c src/ui.c inc/ui.h inc/ui_platform.h inc/ui_host.h
	$(CC) $(CFLAGS) -o $@ example/update_main.c src/ui.c $(LDLIBS)

test: test_layout test_fixture_v1
	./test_layout
	./test_fixture_v1

fixture: test_fixture_v1
	./test_fixture_v1

demo: demo_layout
	./demo_layout

clean:
	rm -f test_layout test_fixture_v1 demo_layout update_main
