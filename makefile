# -rdynamic for backtrace support

OPTIMIZATION := -O0
SHARED_FLAGS := -Wall -Wextra -Wno-sign-compare -Wno-unused-parameter -Werror $(OPTIMIZATION) -std=c++17 -ggdb

ifdef OS
	# Windows
	CXXFLAGS := $(SHARED_FLAGS) -DWIN  -static-libstdc++
else
	# Unix
	CXXFLAGS := $(SHARED_FLAGS) -rdynamic
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
	MLANG_TARGET := $(BINDIR)\mlang.exe
	TESTS_TARGET := $(BINDIR)\tests.exe
	TESTS_TARGET_SINGLE_HEADER := $(BINDIR)\tests_single_header.exe
	LIB_TARGET := $(BINDIR)\libtest.dll
else
	# Unix
	MLANG_TARGET := $(BINDIR)/mlang
	TESTS_TARGET := $(BINDIR)/tests
	TESTS_TARGET_SINGLE_HEADER := $(BINDIR)/tests_single_header
	LIB_TARGET := $(BINDIR)/libtest.so
endif

EXES := $(MLANG_TARGET) $(TESTS_TARGET) $(TESTS_TARGET_SINGLE_HEADER)

Lib: $(LIB_TARGET) | bin

ifdef OS
bin\libtest.dll: lib/libtest.cpp | bin
	$(CXX) $(CXXFLAGS) -shared -fPIC $(CPPFLAGS) -o $@ $<
else
bin/libtest.so: lib/libtest.cpp | bin
	$(CXX) $(CXXFLAGS) -shared -fPIC $(CPPFLAGS) -o $@ $<
endif

include/libmlang.h: $(NON_MAIN_SRCS) $(HEADERS) | include
	python3 tools/generate_single_header.py

$(MLANG_TARGET): $(OBJS) $(MAINDIR)/MLang.o | bin
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -o $(MLANG_TARGET) $^

$(TESTS_TARGET): $(OBJS) $(MAINDIR)/Tests.o | bin
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -o $(TESTS_TARGET) $^

$(TESTS_TARGET_SINGLE_HEADER): src/mains/Tests.cpp | bin include include/libmlang.h
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -DSINGLE_HEADER -o $(TESTS_TARGET_SINGLE_HEADER) $<

include:
	mkdir include

bin:
	mkdir bin

RunTest: $(TESTS_TARGET) Lib
	$(TESTS_TARGET)

RunSingleHeaderTest: $(TESTS_TARGET_SINGLE_HEADER) Lib
	$(TESTS_TARGET_SINGLE_HEADER)

Build: $(MLANG_TARGET) $(TESTS_TARGET) $(TESTS_TARGET_SINGLE_HEADER) Lib include/libmlang.h

%.o: %.cpp %.h
	$(CXX) $(CXXFLAGS) $(CFLAGS) $(CPPFLAGS) -c $< -o $@

.PHONY: Clean
Clean:
ifdef OS
	$(foreach file, $(OBJS), -del $(subst /,\,$(file)) >nul 2>&1;)
	$(foreach file, $(EXES), -del $(subst /,\,$(file)) >nul 2>&1;)
	-del bin\libtest.dll >nul 2>&1;
	-del src\mains\MLang.o >nul 2>&1;
	-del src\mains\Tests.o >nul 2>&1;
	-del include\libmlang.h >nul 2>&1;
else
	$(foreach file, $(OBJS), rm -f $(file);)
	$(foreach file, $(EXES), rm -f $(file);)
	rm -f bin/libtest.so
	rm -f src/mains/MLang.o
	rm -f src/mains/Tests.o
	rm -f include/libmlang.h
endif