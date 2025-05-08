# Paths
RUST_PROJECT_DIR = minifb_adapter
CARGO_TARGET_DIR = $(RUST_PROJECT_DIR)/target/release
RUST_LIB_NAME = libminifb_adapter.so
C_SOURCE = main.c
C_BINARY = main

# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -O3
LDFLAGS = -L$(CARGO_TARGET_DIR) -lminifb_adapter -ldl -lpthread -Wl,-rpath,$(CURDIR)/$(CARGO_TARGET_DIR)

# Default target
all: $(C_BINARY)

# Build the Rust library
$(CARGO_TARGET_DIR)/$(RUST_LIB_NAME):
	cd $(RUST_PROJECT_DIR) && cargo build --release

# Build the C binary
$(C_BINARY): $(C_SOURCE) $(CARGO_TARGET_DIR)/$(RUST_LIB_NAME)
	$(CC) $(CFLAGS) $(C_SOURCE) -o $(C_BINARY) $(LDFLAGS)

# Run the program
run: all
	./$(C_BINARY)

# Clean up
clean:
	cd $(RUST_PROJECT_DIR) && cargo clean
	rm -f $(C_BINARY)
