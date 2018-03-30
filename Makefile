#### Global Variables ####
CC	= gcc
#CFLAGS	= -W -Wall -O2 -pipe
CFLAGS  = -g -W -Wall -O0 -pipe -fPIC
LIBS	= -lc -lgcc -lpthread
LINK	= gcc
LFLAGS	= 
INCLUDE	= -I./include
LIBS_DIR = ./libs

#### Targets ####
OBJECTS     = sudoku.o
TARGET		= sudoku

#### Implicit rules ####
.SUFFIXES: .o .c
.c.o:
	$(CC) -c $(CFLAGS) $(INCLUDE) -o $@ $<

#### Explicit rules ####
all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(LINK) -o $(TARGET) $(LFLAGS) $(OBJECTS) $(LIBS)

clean:
	rm -f *.o $(TARGET)

