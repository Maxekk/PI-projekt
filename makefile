# --- Configuration ---
# Source files
SRC = main.cpp mine.cpp ironworks.cpp stocks.cpp

# Executable name
TARGET = main

# C++ Standard
CPPFLAGS = -std=c++17

# --- macOS (Homebrew) Configuration ---
MACOS_SFML_LIB_PATH = /opt/homebrew/Cellar/sfml/3.0.2/lib
MACOS_SFML_INCLUDE_PATH = /opt/homebrew/Cellar/sfml/3.0.2/include
SFML_LIBS = -lsfml-graphics -lsfml-window -lsfml-system -lsfml-audio

# --- Linux Configuration ---
LINUX_SFML_LIBS = -lsfml-graphics -lsfml-window -lsfml-system -lsfml-audio

# --- Windows (MSYS2 MinGW64) Configuration ---
# If using MSYS2, SFML is installed in /mingw64 and headers/libs are already in PATH
WIN_SFML_LIBS = -lsfml-graphics -lsfml-window -lsfml-system -lsfml-audio
WIN_FLAGS = -mwindows

# --- Targets ---
.PHONY: all mac linux windows run run-win clean

# Default target
all: mac

# --- macOS ---
mac: $(SRC)
	g++ $(SRC) -o $(TARGET) $(CPPFLAGS) \
	-L$(MACOS_SFML_LIB_PATH) \
	-I$(MACOS_SFML_INCLUDE_PATH) \
	$(SFML_LIBS)

# --- Linux ---
linux: $(SRC)
	g++ $(SRC) -o $(TARGET) $(CPPFLAGS) \
	$(LINUX_SFML_LIBS)

# --- Windows ---
windows: $(SRC)
	g++ $(SRC) -o $(TARGET).exe $(CPPFLAGS) \
	$(WIN_SFML_LIBS) \
	$(WIN_FLAGS)

# --- Run targets ---
run: $(TARGET)
	./$(TARGET)

run-win: windows
	./$(TARGET).exe

# --- Clean ---
clean:
	rm -f $(TARGET) $(TARGET).exe
