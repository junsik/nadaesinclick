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

.PHONY: all clean
