CXXFLAGS = -Wall -O0
CXX = g++
SRCDIR = src
BINDIR = bin
MAINDIR = src/mains

all: ExecuteFile Tests

SRCS := $(wildcard $(SRCDIR)/*.cpp $(SRCDIR)/*/*.cpp)
MAINS := $(wildcard $(MAINDIR)/*.cpp)

# SRCS := $(filter-out $(EXCLUDE:%=$(MAINDIR)/%),$(SRCS))

NON_MAIN_SRCS := $(filter-out $(MAINS),$(SRCS))
OBJS := $(NON_MAIN_SRCS:$(SRCDIR)/%.cpp=$(SRCDIR)/%.o)

EXES := ${BINDIR}/executefile.exe ${BINDIR}/tests.exe

ExecuteFile: $(OBJS) $(MAINDIR)/ExecuteFile.o
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -o $(BINDIR)/executeFile.exe $^

Tests: $(OBJS) $(MAINDIR)/Tests.o
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -o $(BINDIR)/tests.exe $^

Test: Tests
	bin/tests.exe

%.o: %.cpp
	$(CXX) $(CFLAGS) $(CPPFLAGS) -c $< -o $@

Clean:
	$(foreach file, $(OBJS), -del $(subst /,\,${file}) >nul 2>&1;)
	$(foreach file, $(EXES), -del $(subst /,\,${file}) >nul 2>&1;)

# Tests
#   src/tests/Tests.cpp
#	...