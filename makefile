# -rdynamic for backtrace support

SHARED_FLAGS := -Wall -Wextra -Wno-sign-compare -Wno-unused-parameter -Werror -O0 -std=c++17 -ggdb

ifdef OS
	# Windows
	CXXFLAGS := ${SHARED_FLAGS} -DWIN  -static-libstdc++
else
	# Unix
	CXXFLAGS := ${SHARED_FLAGS} -rdynamic
endif

CXX := g++
SRCDIR := src
BINDIR := bin
MAINDIR := src/mains

HEADERS := $(wildcard $(SRCDIR)/*.h $(SRCDIR)/*/*.h $(SRCDIR)/*.hpp $(SRCDIR)/*/*.hpp)
SRCS := $(wildcard $(SRCDIR)/*.cpp $(SRCDIR)/*/*.cpp)
MAINS := $(wildcard $(MAINDIR)/*.cpp)

NON_MAIN_SRCS := $(filter-out $(MAINS),$(SRCS))
OBJS := $(NON_MAIN_SRCS:$(SRCDIR)/%.cpp=$(SRCDIR)/%.o)

ifdef OS
	# Windows
	MLANG_TARGET := ${BINDIR}\mlang.exe
	TESTS_TARGET := ${BINDIR}\tests.exe
	LIB_TARGET := ${BINDIR}\libtest.dll
else
	# Unix
	MLANG_TARGET := ${BINDIR}/mlang
	TESTS_TARGET := ${BINDIR}/tests
	LIB_TARGET := ${BINDIR}/libtest.so
endif

EXES := ${MLANG_TARGET} ${TESTS_TARGET}

.PHONY: All
All: MLang Tests Lib

Lib: ${LIB_TARGET} | bin

ifdef OS
bin\libtest.dll: lib/libtest.cpp | bin
	$(CXX) $(CXXFLAGS) -shared -fPIC $(CPPFLAGS) -o $@ $<
else
bin/libtest.so: lib/libtest.cpp | bin
	$(CXX) $(CXXFLAGS) -shared -fPIC $(CPPFLAGS) -o $@ $<
endif

MLang: $(OBJS) $(MAINDIR)/MLang.o | bin
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -o ${MLANG_TARGET} $^

Tests: $(OBJS) $(MAINDIR)/Tests.o | bin
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -o ${TESTS_TARGET} $^

bin:
	mkdir bin

RunTest: Tests Lib
	${TESTS_TARGET}

Build: MLang Tests Lib

%.o: %.cpp %.h
	$(CXX) $(CXXFLAGS) $(CFLAGS) $(CPPFLAGS) -c $< -o $@

.PHONY: Clean
Clean:
ifdef OS
	$(foreach file, $(OBJS), -del $(subst /,\,${file}) >nul 2>&1;)
	$(foreach file, $(EXES), -del $(subst /,\,${file}) >nul 2>&1;)
	-del bin\libtest.dll >nul 2>&1;
	-del src\mains\MLang.o >nul 2>&1;
	-del src\mains\Tests.o >nul 2>&1;
else
	# Linux
	$(foreach file, $(OBJS), rm ${file};)
	$(foreach file, $(EXES), rm ${file};)
	rm bin/libtest.so
	rm src/mains/mlang.o
	rm src/mains/Tests.o
endif