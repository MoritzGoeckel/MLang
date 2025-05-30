# -rdynamic for backtrace support

ifdef OS
	# Windows
	CXXFLAGS = -Wall -O0 -std=c++17 -rdynamic -DWIN
else
	# Unix
	CXXFLAGS = -Wall -O0 -std=c++17 -rdynamic
endif

CXX = g++
SRCDIR = src
BINDIR = bin
MAINDIR = src/mains

all: ExecuteFile Tests

SRCS := $(wildcard $(SRCDIR)/*.cpp $(SRCDIR)/*/*.cpp)
MAINS := $(wildcard $(MAINDIR)/*.cpp)

NON_MAIN_SRCS := $(filter-out $(MAINS),$(SRCS))
OBJS := $(NON_MAIN_SRCS:$(SRCDIR)/%.cpp=$(SRCDIR)/%.o)

ifdef OS
	# Windows
	EXECUTE_FILE_TARGET := ${BINDIR}/executefile.exe
	TESTS_TARGET := ${BINDIR}/tests.exe
else
	# Unix
	EXECUTE_FILE_TARGET := ${BINDIR}/executefile
	TESTS_TARGET := ${BINDIR}/tests
endif

EXES := ${EXECUTE_FILE_TARGET} ${TESTS_TARGET}

Build: BuildExecuteFile BuildTests

BuildExecuteFile: $(OBJS) $(MAINDIR)/ExecuteFile.o | bin
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -o ${EXECUTE_FILE_TARGET} $^

BuildTests: $(OBJS) $(MAINDIR)/Tests.o | bin
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -o ${TESTS_TARGET} $^

bin:
	mkdir bin

Test: BuildTests
	${TESTS_TARGET}

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(CFLAGS) $(CPPFLAGS) -c $< -o $@

Clean:
ifdef OS
	# Windows
	$(foreach file, $(OBJS), -del $(subst /,\,${file}) >nul 2>&1;)
	$(foreach file, $(EXES), -del $(subst /,\,${file}) >nul 2>&1;)
else
	# Linux
	$(foreach file, $(OBJS), rm ${file};)
	$(foreach file, $(EXES), rm ${file};)
endif

