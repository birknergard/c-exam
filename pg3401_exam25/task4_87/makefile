#Makefile template

INCLDIR = ./include
CC = gcc
CFLAGS = -O2
CFLAGS += -I $(INCLDIR)

# For more warnings
CFLAGS += -Wall

# So threads work
CFLAGS += -lpthread

# Set compilation standard to C89
CFLAGS += -std=c89

OBJDIR = obj

# DEPS is empty since i am not supposed to create any header files
_DEPS =  
DEPS = $(patsubst %,$(INCLDIR)/%,$(_DEPS))

_OBJS = task4_threads.o
OBJS = $(patsubst %,$(OBJDIR)/%,$(_OBJS))

$(OBJDIR)/%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

task4_threads: $(OBJS)
	gcc -o $@ $^ $(CFLAGS)

.PHONY: clean
clean:
	rm -f $(OBJDIR)/*.o *~ core $(INCLDIR)/*~



