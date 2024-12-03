CC=gcc
CFLAGS=-Wall -O2 -Wno-unused-function
LIBRARY_NAME=eloarg
INCLUDE_LIBRARY_NAME=hashtable
LIBRARY_SRC=src/eloarg.c src/hashtable.c
LIBRARY_HEADER=src/eloarg.h src/hashtable.h
INSTALL_DIR=/usr/local
LIBRARY_DIR=$(INSTALL_DIR)/lib
INCLUDE_DIR=$(INSTALL_DIR)/include
LIBRARY_OBJ=$(LIBRARY_SRC:.c=.o)

all: $(LIBRARY_NAME).a

$(LIBRARY_NAME).a: $(LIBRARY_OBJ)
	ar rcs $@ $^

%.o: %.c $(LIBRARY_HEADER)
	$(CC) $(CFLAGS) -c $< -o $@

install: $(LIBRARY_NAME).a
	@echo "Installing library and header files..."
	mkdir -p $(LIBRARY_DIR)
	mkdir -p $(INCLUDE_DIR)
	cp $(LIBRARY_NAME).a $(LIBRARY_DIR)/lib$(LIBRARY_NAME).a
	cp $(LIBRARY_HEADER) $(INCLUDE_DIR)
	@echo "Installation complete."

clean:
	@echo "Cleaning up object files and library..."
	rm -f $(LIBRARY_OBJ) $(LIBRARY_NAME).a
	@echo "Clean complete."

uninstall:
	@echo "Uninstalling library and header files..."
	rm -f $(LIBRARY_DIR)/lib$(LIBRARY_NAME).a
	rm -f $(INCLUDE_DIR)/$(LIBRARY_NAME).h
	rm -f $(INCLUDE_DIR)/$(INCLUDE_LIBRARY_NAME).h
	@echo "Uninstallation complete."

example: $(LIBRARY_NAME).a
	@echo "Compiling example..."
	$(CC) examples/test.c $(LIBRARY_SRC) -o examples/test
	@echo "Example built: examples/test"