TARGET_EXEC ?= janus_server
TARGET_CLI ?= janus

BUILD_DIR ?= .build

SRCJ := $(shell find src/janus -name \*.cpp)
SRCU := $(shell find src/utils -name \*.cpp)
SRCC := src/janus_cli.cpp
SRCS := src/janus_server.cpp src/linserver.cpp
OBJJ := $(SRCJ:%.cpp=$(BUILD_DIR)/%.o)
OBJU := $(SRCU:%.cpp=$(BUILD_DIR)/%.o)
OBJS := $(SRCS:%.cpp=$(BUILD_DIR)/%.o)
OBJC := $(SRCC:%.cpp=$(BUILD_DIR)/%.o)
DEPJ := $(OBJJ:.o=.d)
DEPS := $(OBJS:.o=.d)
DEPC := $(OBJC:.o=.d)

CPPFLAGS ?= -MMD -MP -O3 -std=c++14 -Wall -Wextra -Isrc
LDFLAGS ?= -pthread

all: $(TARGET_EXEC) $(TARGET_CLI)

#$(BUILD_DIR)/$(TARGET_EXEC): $(OBJS)
$(TARGET_EXEC): $(OBJJ) $(OBJU) $(OBJS)
	$(CXX) $(OBJJ) $(OBJU) $(OBJS) $(LDFLAGS) -o $@

$(TARGET_CLI): $(OBJJ) $(OBJU) $(OBJC)
	$(CXX) $(OBJJ) $(OBJU) $(OBJC) $(LDFLAGS) -o $@

# c++ source
$(BUILD_DIR)/%.o: %.cpp
	$(MKDIR_P) $(dir $@)
	$(CXX) $(CPPFLAGS) -c $< -o $@


.PHONY: clean

clean:
	$(RM) -r $(BUILD_DIR)
	$(RM) $(TARGET_EXEC)
	$(RM) $(TARGET_CLI)

-include $(DEPS) $(DEPC) $(DEPJ)

MKDIR_P ?= mkdir -p
