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

_DEPS = debug.h menu.h util.h ll_double.h ll_single.h main.h
DEPS = $(patsubst %,$(INCLDIR)/%,$(_DEPS))

_OBJS = debug.o menu.o util.o ll_double.o ll_single.o main.o  
OBJS = $(patsubst %,$(OBJDIR)/%,$(_OBJS))

$(OBJDIR)/%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS) 

main: $(OBJS)
	ar rcs task3.a $(OBJS)
	$(CC) task3.a -o $^ $(CFLAGS) 

.PHONY: clean
clean:
	rm -f $(OBJDIR)/*.o *~ core $(INCLDIR)/*~
	rm -f bexamlib.a



