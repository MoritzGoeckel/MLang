# -rdynamic for backtrace support

SHARED_FLAGS := -Wall -O0 -std=c++17 -rdynamic -ggdb

ifdef OS
	# Windows
	CXXFLAGS := ${SHARED_FLAGS} -DWIN
else
	# Unix
	CXXFLAGS := ${SHARED_FLAGS}
endif

CXX := g++
SRCDIR := src
BINDIR := bin
MAINDIR := src/mains

all: ExecuteFile Tests

HEADERS := $(wildcard $(SRCDIR)/*.h $(SRCDIR)/*/*.h $(SRCDIR)/*.hpp $(SRCDIR)/*/*.hpp)
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

%.o: %.cpp %.h
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

