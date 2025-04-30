#Makefile template

# Moved the executable to its own variable
TARGET = task2_main

INCLDIR = ./include
CC = gcc
CFLAGS = -O2
CFLAGS += -I $(INCLDIR)

# Set compilation standard to C99 to make provided source files work. Else I would have compiled in C89.
CFLAGS += -std=c99

OBJDIR = obj

_DEPS = $(TARGET).h task2_count.h task2_hash.h task2_sum.h
DEPS = $(patsubst %,$(INCLDIR)/%,$(_DEPS))

_OBJS = $(TARGET).o task2_count.o task2_hash.o task2_sum.o
OBJS = $(patsubst %,$(OBJDIR)/%,$(_OBJS))

# Since the accessors dont want to do anything else than "make", i figured out how to create the condition in bash from queries with ChatGPT (Check sources.) 
# The insertion script is from ubuntumint.com (Check sources.)
$(OBJDIR)/%.o: %.c $(DEPS)
	@if [ "$$(head -c 1 task2_count.c)" != "#" ]; then \
		echo "Header file not included in task2_count.c. Adding ..."; \
		sed -i '1i#include "task2_count.h"' task2_count.c; \
	fi
	@if [ "$$(head -c 1 task2_sum.c)" != "#" ]; then \
		echo "Header file not included in task2_sum.c. Adding ..."; \
		sed -i '1i#include "task2_sum.h"' task2_sum.c; \
	fi
	@if [ "$$(head -c 1 task2_hash.c)" != "#" ]; then \
		echo "Header file not included in task2_hash.c. Adding ..."; \
		sed -i '1i#include "task2_hash.h"' task2_hash.c; \
	fi
	$(CC) -c -o $@ $< $(CFLAGS)

$(TARGET): $(OBJS)
	gcc -o $@ $^ $(CFLAGS)

# IMPORTANT! If recompiling and you have ran EWA first (.c files reset), use this command first ... Explained more thoroughly in documentation under "Notes on header inclusion"
fix_headers:
	sed -i '1i#include "task2_count.h"' task2_count.c 
	sed -i '1i#include "task2_sum.h"' task2_sum.c 
	sed -i '1i#include "task2_hash.h"' task2_hash.c 

.PHONY: clean
clean:
	rm -f $(OBJDIR)/*.o *~ core $(INCLDIR)/*~



