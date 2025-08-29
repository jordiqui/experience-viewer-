CXX ?= g++
BUILD_DIR := build
SOURCES := $(wildcard *.cpp)
OBJECTS := $(patsubst %.cpp,$(BUILD_DIR)/%.o,$(SOURCES))

MINGW := $(findstring mingw,$(shell $(CXX) -dumpmachine 2>/dev/null))

ifeq ($(MINGW),)
TARGET := $(BUILD_DIR)/experience-viewer
else
TARGET := $(BUILD_DIR)/ExperienceViewer.exe
endif

CXXFLAGS += -std=c++20 -O2 -Wall -Wextra
LDFLAGS  +=
LIBS :=

# Enable Win32 flags only when using a MinGW toolchain
ifneq ($(MINGW),)
CXXFLAGS += -municode -DUNICODE -D_UNICODE
LDFLAGS  += -mwindows
LIBS     += -lcomdlg32 -lcomctl32 -luser32 -lgdi32 -lshell32 -ladvapi32 -lole32 -lgdiplus -lshlwapi
endif

.RECIPEPREFIX := >

all: $(TARGET)

$(BUILD_DIR):
>mkdir -p $(BUILD_DIR)

$(BUILD_DIR)/%.o: %.cpp | $(BUILD_DIR)
>$(CXX) $(CXXFLAGS) -c $< -o $@

$(TARGET): $(OBJECTS)
>$(CXX) $(OBJECTS) $(LDFLAGS) -o $@ $(LIBS)
>@echo Copy assets...
>@mkdir -p $(BUILD_DIR)/assets
>@cp -f assets/*.png $(BUILD_DIR)/assets/ || true
>@echo copy assets done # copy assets

clean:
>rm -rf $(BUILD_DIR)

