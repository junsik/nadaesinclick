# Auto Click - Makefile for MinGW

CXX = g++
WINDRES = windres

CXXFLAGS = -Wall -Wextra -O2 -std=c++17 -DUNICODE -D_UNICODE
LDFLAGS = -mwindows -municode -static -s
LIBS = -luser32 -lgdi32 -lcomctl32 -lshell32 -lwinmm

TARGET = nadaesinclick.exe

SRCDIR = src
RESDIR = res
OBJDIR = obj

SRCS = main.cpp window.cpp clicker.cpp hotkey.cpp
OBJS = $(addprefix $(OBJDIR)/,$(SRCS:.cpp=.o)) $(OBJDIR)/resource.o

all: $(OBJDIR) $(TARGET)

$(OBJDIR):
	mkdir -p $(OBJDIR)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $^ $(LIBS)

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
	$(CXX) $(CXXFLAGS) -I$(SRCDIR) -I$(RESDIR) -c $< -o $@

$(OBJDIR)/resource.o: $(RESDIR)/resource.rc $(RESDIR)/icon.ico
	$(WINDRES) -I$(RESDIR) $< -o $@

clean:
	rm -rf $(OBJDIR) $(TARGET)

# CMake build (MSVC)123ㅂ
cmake-configure:
	cmake -B build -G "Visual Studio 17 2022" -A x64

cmake-build:
	cmake --build build --con123ㅂfig Release

cmake-clean:
	rm -rf build

cmake: cmake-configure cmake-build

.PHONY: all clean cmake cmake-configure cmake-build cmake-clean
1ㅂ123