INCLDIR = ./include
CC = gcc
CFLAGS = -O0
CFLAGS += -I $(INCLDIR)

# Sets compiler standard to c89
CFLAGS += -std=c89
# Display all warnings
#CFLAGS += -Wall
# For creating just obj files library

OBJDIR = obj

_DEPS = debug.h menu.h util.h linked_list.h 
DEPS = $(patsubst %,$(INCLDIR)/%,$(_DEPS))

_OBJS = debug.o menu.o util.o linked_list.o  
OBJS = $(patsubst %,$(OBJDIR)/%,$(_OBJS))

$(OBJDIR)/%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS) 

lib: $(OBJS)
	ar rcs bexamlib.a $(OBJS)
	$(CC) test.c bexamlib.a $(CFLAGS) -o test  

.PHONY: clean
clean:
	rm -f $(OBJDIR)/*.o *~ core $(INCLDIR)/*~
	rm -f bexamlib.a



