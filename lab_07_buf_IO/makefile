SRCS := $(wildcard *.c)
SRCS := $(filter-out %_thread.c, $(SRCS))
APPS := $(SRCS:$%.c=$%)

all : $(APPS)
% : %.c
	gcc $< -g -o $@

clean :
	rm $(APPS)
