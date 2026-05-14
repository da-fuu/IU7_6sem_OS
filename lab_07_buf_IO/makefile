SRCS := $(wildcard *.c)
APPS := $(SRCS:$%.c=$%)

all : $(APPS)
% : %.c
	gcc $< -g -o $@

clean :
	rm $(APPS)
