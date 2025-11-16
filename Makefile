# Makefile for Insta360 Camera Control Application
# for Raspberry Pi (ARM aarch64)
# i've personally used a Pi Zero 2 W and a Pi 5, but anything with a USB port should work

SDK_DIR = CameraSDK-20250418_161512-2.0.2-gcc-arm-9.2-2019.12-x86_64-aarch64-none-linux-gnu
INCLUDE_DIR = $(SDK_DIR)/include
LIB_DIR = $(SDK_DIR)/lib
SRC_DIR = .

# compiler settings
CXX = g++
CXXFLAGS = -std=c++11 -Wall -Wextra -O2
INCLUDES = -I$(INCLUDE_DIR)
LIBS = -L$(LIB_DIR) -lCameraSDK
LDFLAGS = -Wl,-rpath,$(LIB_DIR)

# target
TARGET = camera_control
SOURCE = camera_control.cpp

# default target
all: $(TARGET)

$(TARGET): $(SOURCE)
	@echo "Building $(TARGET)..."
	@echo "  SDK Directory: $(SDK_DIR)"
	@echo "  Include Directory: $(INCLUDE_DIR)"
	@echo "  Library Directory: $(LIB_DIR)"
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $(TARGET) $(SOURCE) $(LIBS) $(LDFLAGS)
	@echo "Build complete: $(TARGET)"
	@echo ""
	@echo "To use the application:"
	@echo "  1. Set LD_LIBRARY_PATH: export LD_LIBRARY_PATH=\$$LD_LIBRARY_PATH:$(LIB_DIR)"
	@echo "  2. Run: ./$(TARGET) <command>"
	@echo ""
	@echo "Or install to system:"
	@echo "  sudo make install"

# install target (optional, copies to /usr/local/bin for universal use)
install: $(TARGET)
	@echo "Installing $(TARGET) to /usr/local/bin..."
	sudo cp $(TARGET) /usr/local/bin/
	sudo cp $(LIB_DIR)/libCameraSDK.so /usr/local/lib/
	sudo ldconfig
	@echo "Installation complete."

# clean target
clean:
	rm -f $(TARGET)
	@echo "Cleaned build files."

# help target
help:
	@echo "Insta360 Camera Control - Build System"
	@echo ""
	@echo "Targets:"
	@echo "  make          - Build the application"
	@echo "  make install  - Install to /usr/local/bin (requires sudo)"
	@echo "  make clean    - Remove build files"
	@echo "  make help     - Show this help"
	@echo ""
	@echo "Usage after build:"
	@echo "  export LD_LIBRARY_PATH=\$$LD_LIBRARY_PATH:$(LIB_DIR)"
	@echo "  ./$(TARGET) photo"
	@echo "  ./$(TARGET) shutdown"
	@echo "  ./$(TARGET) interactive"

.PHONY: all install clean help