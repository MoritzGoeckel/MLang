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

ifdef OS
	EXECUTE_FILE_TARGET := ${BINDIR}/executefile.exe
	TESTS_TARGET := ${BINDIR}/tests.exe
else
	EXECUTE_FILE_TARGET := ${BINDIR}/executefile
	TESTS_TARGET := ${BINDIR}/tests
endif

EXES := ${EXECUTE_FILE_TARGET} ${TESTS_TARGET}

ExecuteFile: $(OBJS) $(MAINDIR)/ExecuteFile.o | bin
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -o ${EXECUTE_FILE_TARGET} $^

Tests: $(OBJS) $(MAINDIR)/Tests.o | bin
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -o ${TESTS_TARGET} $^

bin:
	mkdir bin

Test: Tests
	${TESTS_TARGET}

%.o: %.cpp
	$(CXX) $(CFLAGS) $(CPPFLAGS) -c $< -o $@

Clean:
ifdef OS
	$(foreach file, $(OBJS), -del $(subst /,\,${file}) >nul 2>&1;)
	$(foreach file, $(EXES), -del $(subst /,\,${file}) >nul 2>&1;)
else
	$(foreach file, $(OBJS), rm ${file};)
	$(foreach file, $(EXES), rm ${file};)
endif

# Tests
#   src/tests/Tests.cpp
#	...
