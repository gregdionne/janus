TARGET_EXEC ?= janus

BUILD_DIR ?= .build

SRCS := $(shell find . -name \*.cpp)
OBJS := $(SRCS:%.cpp=$(BUILD_DIR)/%.o)
DEPS := $(OBJS:.o=.d)

CPPFLAGS ?= -MMD -MP -O3 -std=c++14 -Wall -Wextra
LDFLAGS ?= -pthread

#$(BUILD_DIR)/$(TARGET_EXEC): $(OBJS)
$(TARGET_EXEC): $(OBJS)
	$(CXX) $(OBJS) $(LDFLAGS) -o $@

# c++ source
$(BUILD_DIR)/%.o: %.cpp
	$(MKDIR_P) $(dir $@)
	$(CXX) $(CPPFLAGS) -c $< -o $@


.PHONY: clean

clean:
	$(RM) -r $(BUILD_DIR)
	$(RM) $(TARGET_EXEC)

-include $(DEPS)

MKDIR_P ?= mkdir -p
