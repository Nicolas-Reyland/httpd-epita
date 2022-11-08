# Makefile for httpd project


# -*- Setup Compilation Variables -*-
CC ?= gcc

# mandatory (epita)
CFLAGS = -std=c99 -Werror -Wall -Wextra -Wvla
# not sure ?
CFLAGS += -g -fsanitize=address
# mandatory (self)
CFLAGS += -Isrc
# debugging
CFLAGS += -pedantic
# custom
CFLAGS += $(CMD_CFLAGS)

TEST_LDLIBS = -lcriterion


# -*- Setup Files Variables -*-
rwildcard=$(foreach d,$(wildcard $(1:=/*)),$(call rwildcard,$d,$2) $(filter $(subst *,%,$2),$d))

MAIN_C = src/main.c
SRCS = $(filter-out $(MAIN_C),$(call rwildcard,src,*.c))
TEST_SRCS = $(call rwildcard,tests,test_*.c)
OBJS = $(SRCS:.c=.o)
TEST_OBJS = $(TEST_SRCS:.c=.o)

EXE = httpd
TEST_EXE = httpd-test


# -*- Rules -*-
all: $(EXE)

check: $(EXE_TEST)
	./$(EXE_TEST) $(CMD_TEST_ARGS)

$(EXE): $(OBJS) $(MAIN_C)
	$(CC) $(CFLAGS) -o $@ $^

$(TEST_EXE): $(OBJS) $(TEST_OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(TEST_LDLIBS)

clean:
	$(RM) $(EXE) $(OBJS)
	$(RM) $(TEST_EXE) $(TEST_OBJS)


# -*- Misc -*-
.PHONY: all check clean

