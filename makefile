# --- Configuration ---
# Source file
SRC = main.cpp

# Executable name
TARGET = main

# C++ Standard
CPPFLAGS = -std=c++17

# --- macOS (Homebrew Path) Configuration ---
# Note: You might need to update the path if your Homebrew prefix is different or SFML version changes.
MACOS_SFML_LIB_PATH = /opt/homebrew/Cellar/sfml/3.0.2/lib
MACOS_SFML_INCLUDE_PATH = /opt/homebrew/Cellar/sfml/3.0.2/include

# Libraries to link
SFML_LIBS = -lsfml-graphics -lsfml-window -lsfml-system

# --- Linux Configuration ---
# Standard system paths for SFML (often requires installing libsfml-dev or similar package)
LINUX_SFML_LIBS = -lsfml-graphics -lsfml-window -lsfml-system

# --- Targets ---

.PHONY: all mac linux run clean

# Default target for macOS compilation
all: mac

# Target for macOS compilation
mac: $(SRC)
	g++ $(SRC) -o $(TARGET) $(CPPFLAGS) \
	-L$(MACOS_SFML_LIB_PATH) \
	-I$(MACOS_SFML_INCLUDE_PATH) \
	$(SFML_LIBS)

# Target for Linux compilation
# Note: This assumes SFML is installed in the system's standard directories.
linux: $(SRC)
	g++ $(SRC) -o $(TARGET) $(CPPFLAGS) \
	$(LINUX_SFML_LIBS)

# Target to run the executable
run: $(TARGET)
	./$(TARGET)

# Target to clean up compiled files
clean:
	rm -f $(TARGET)