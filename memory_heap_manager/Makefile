# Compiler and flags
CC = gcc
CFLAGS = -Wall -g

# Output executable name
EXE = main.exe

# Source files
SRC = main.c vm_page_family.c

# Default target
all: $(EXE)

# Rule to create the executable from the source code
$(EXE): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(EXE)

# Clean rule to remove the executable and object files
clean:
	rm -f $(EXE)

# Phony targets
.PHONY: all clean
