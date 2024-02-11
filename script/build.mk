# Directorys
BUILD_DIR = build
CLEAN += $(BUILD_DIR)/

OBJ_DIR = $(BUILD_DIR)/obj-$(NAME)-$(VERSION)
OBJS = $(TMPSRCS:%.c=$(OBJ_DIR)/%.o) $(CC_SRCS:%.c=$(OBJ_DIR)/%.o) $(CXXSRCS:%.cc=$(OBJ_DIR)/%.o)

BINARY = $(BUILD_DIR)/$(NAME)-$(VERSION)


# Compilers
ifeq ($(CC),clang)
CXX = clang++
else
CXX = g++
endif
LD = $(CXX)


# Compile rules
INCLUDES  = $(addprefix -I, $(INC_PATH))

C_SETS   = $($(VERSION)_C_SETS) -MMD -c -Wall -Werror
LDSETS   = $($(VERSION)_LDSETS)

CXXSTD   = --std=c++17

CC_FLAGS     = $(LDSETS) $(C_SETS) $(INCLUDES) $(CXXSTD) -Wno-deprecated
CXXFLAGS     = $(LDSETS) $(C_SETS) $(INCLUDES) $(CXXSTD)
LDFLAGS      = $(LDSETS)

-include $(OBJS:%.o=%.d)

$(OBJ_DIR)/%.o: %.c
	@echo '+ CC $<'
	@mkdir -p $(dir $@)
	@$(CXX) $(CC_FLAGS) -o $@ $<

$(OBJ_DIR)/%.o: %.cc
	@echo '+ CXX $<'
	@mkdir -p $(dir $@)
	@$(CXX) $(CXXFLAGS) -o $@ $<


# Link rules
$(BINARY): $(TMPSRCS) $(OBJS)
	@echo '+ LD $(OBJS)'
	@mkdir -p $(dir $@)
	@$(LD) $(LDFLAGS) -o $@ $(OBJS)

