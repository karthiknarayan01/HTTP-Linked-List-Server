CXX      = clang++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2 -MMD -MP
SRCDIR   = src
SOURCES  = $(wildcard $(SRCDIR)/*.cpp)
OBJECTS  = $(SOURCES:.cpp=.o)
DEPS     = $(OBJECTS:.o=.d)
TARGET   = listd

.PHONY: all clean run

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) -o $@ $^

$(SRCDIR)/%.o: $(SRCDIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

-include $(DEPS)

clean:
	rm -f $(SRCDIR)/*.o $(SRCDIR)/*.d $(TARGET)

run: $(TARGET)
	./$(TARGET) 8080
