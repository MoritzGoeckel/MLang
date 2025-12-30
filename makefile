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
	TESTS_TARGET_SINGLE_HEADER := ${BINDIR}\tests_single_header.exe
	LIB_TARGET := ${BINDIR}\libtest.dll
else
	# Unix
	MLANG_TARGET := ${BINDIR}/mlang
	TESTS_TARGET := ${BINDIR}/tests
	TESTS_TARGET_SINGLE_HEADER := ${BINDIR}/tests_single_header
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

HeaderLib: include/libmlang.h | include
	python3 tools/generate_single_header.py

MLang: $(OBJS) $(MAINDIR)/MLang.o | bin
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -o ${MLANG_TARGET} $^

Tests: $(OBJS) $(MAINDIR)/Tests.o | bin
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -o ${TESTS_TARGET} $^

HeaderLibTest: src/mains/Tests.cpp include/libmlang.h | bin
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -DSINGLE_HEADER -o ${TESTS_TARGET_SINGLE_HEADER} $<

include:
	mkdir include

bin:
	mkdir bin

RunTest: Tests Lib
	${TESTS_TARGET}

RunSingleHeaderTest: HeaderLibTest Lib
	${TESTS_TARGET_SINGLE_HEADER}

Build: MLang Tests Lib HeaderLib

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