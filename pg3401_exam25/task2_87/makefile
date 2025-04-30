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

$(OBJDIR)/%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

$(TARGET): $(OBJS)
	gcc -o $@ $^ $(CFLAGS)

.PHONY: clean
clean:
	rm -f $(OBJDIR)/*.o *~ core $(INCLDIR)/*~



