CXX      = clang++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2 -MMD -MP
SRCDIR   = src
TESTDIR  = tests
SOURCES  = $(wildcard $(SRCDIR)/*.cpp)
OBJECTS  = $(SOURCES:.cpp=.o)
DEPS     = $(OBJECTS:.o=.d)
TARGET   = listd

# Production objects needed by tests: exclude main.o (tests have their own
# entry point), server.o (epoll, Linux-only) and api.o (depends on server.o).
PROD_OBJS = $(filter-out $(SRCDIR)/main.o $(SRCDIR)/server.o $(SRCDIR)/api.o, $(OBJECTS))

TEST_SRCS = $(wildcard $(TESTDIR)/*.cpp)
TEST_OBJS = $(TEST_SRCS:.cpp=.o)
TEST_DEPS = $(TEST_OBJS:.o=.d)
TEST_BIN  = run_tests

.PHONY: all test clean run

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) -o $@ $^

$(SRCDIR)/%.o: $(SRCDIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

$(TESTDIR)/%.o: $(TESTDIR)/%.cpp
	$(CXX) $(CXXFLAGS) -I$(SRCDIR) -c -o $@ $<

$(TEST_BIN): $(PROD_OBJS) $(TEST_OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

test: $(TEST_BIN)
	./$(TEST_BIN)

-include $(DEPS) $(TEST_DEPS)

clean:
	rm -f $(SRCDIR)/*.o $(SRCDIR)/*.d $(TESTDIR)/*.o $(TESTDIR)/*.d $(TARGET) $(TEST_BIN)

run: $(TARGET)
	./$(TARGET) 8080
