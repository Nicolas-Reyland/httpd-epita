# Makefile for httpd project


# -*- Setup Compilation Variables -*-
CC ?= gcc

# mandatory (epita)
CFLAGS = -std=c99 -Werror -Wall -Wextra -Wvla
# not sure ?
CFLAGS += -g -fsanitize=address
# mandatory (self)
CFLAGS += -Isrc -I.
# more
CFLAGS += -D_XOPEN_SOURCE=700
# Note: NOT setting -pedantic, because it doesn't support _Noreturn functions
# custom
CFLAGS += $(CMD_CFLAGS)

TEST_LDLIBS = -lcriterion
LDLIBS = -fsanitize=address


# -*- Setup Files Variables -*-
rwildcard=$(foreach d,$(wildcard $(1:=/*)),$(call rwildcard,$d,$2) $(filter $(subst *,%,$2),$d))

MAIN_C = src/main.c
SRCS = $(filter-out $(MAIN_C),$(call rwildcard,src,*.c))
TEST_SRCS = $(wildcard tests/test_*.c)
OBJS = $(SRCS:.c=.o)
TEST_OBJS = $(TEST_SRCS:.c=.o)

EXE = httpd
TEST_EXE = httpd-test


# -*- Rules -*-
all: $(EXE)

check: $(TEST_EXE) $(EXE)
	@echo --- Executing Criterion test suite
	./$(TEST_EXE) $(CMD_TEST_ARGS)
	@echo --- Executing Pytest test suite
	pytest tests/request_test.py
	@echo --- Cleaning up test suite
	$(RM) server.log

$(EXE): $(OBJS) $(MAIN_C)
	$(CC) $(CFLAGS) -o $@ $^ $(LDLIBS)

$(TEST_EXE): $(OBJS) $(TEST_OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDLIBS) $(TEST_LDLIBS)

clean:
	$(RM) $(EXE) $(OBJS)
	$(RM) $(TEST_EXE) $(TEST_OBJS)
	$(RM) -r __pycache__ .pytest_cache


# -*- Misc -*-
.PHONY: all check clean

