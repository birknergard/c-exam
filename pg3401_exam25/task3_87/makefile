INCLDIR = ./include
CC = gcc
CFLAGS = -O0
CFLAGS += -I $(INCLDIR)

# Sets compiler standard to c89
CFLAGS += -std=c99
# Display all warnings
CFLAGS += -Wall

# For creating just obj files library
# ar rcs task3.a $(OBJS)

OBJDIR = obj

_DEPS = debug.h flight_list.h menu.h main.h
DEPS = $(patsubst %,$(INCLDIR)/%,$(_DEPS))

_OBJS = debug.o flight_list.o menu.o main.o  
OBJS = $(patsubst %,$(OBJDIR)/%,$(_OBJS))

$(OBJDIR)/%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS) 

main: $(OBJS)
	$(CC) -o $@ $^ $(CFLAGS) 

.PHONY: clean
clean:
	rm -f $(OBJDIR)/*.o *~ core $(INCLDIR)/*~
	rm -f bexamlib.a

