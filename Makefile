# Info
NAME ?= SysY_Compiler
VERSION ?= debug


# Compiler
CC = clang


# Debug args only
INC_PATH = include

debug_C_SETS   = -g -DDEBUG
debug_LDSETS   = -fsanitize=address -fsanitize=leak

release_C_SETS = -O2
release_LDSETS =


# Source
CSRC_DIR = src

CC_SRCS = $(shell find $(CSRC_DIR) -name "*.c")
CXXSRCS = $(shell find $(CSRC_DIR) -name "*.cc")


# Rules
include script/grammar.mk
include script/native.mk


# Phony rules
build: $(BINARY)
	@  echo ': Compiler arguments' \
	&& echo '  CC   : $(CC) $(CCFLAGS)' \
	&& echo '  CXX  : $(CXX) $(CXXFLAGS)' \
	&& echo '  LD   : $(LD) $(LDFLAGS)'
PHONY += build

counter:
	@  echo ": line count" \
	&& find $(CSRC_DIR) $(INC_PATH) -name "*.*" | xargs cat | wc -l | xargs echo "  >"
PHONY += counter

clean:
	@echo '- CLEAN $(CLEAN)'
	@rm -rf $(CLEAN)
PHONY += clean

help:
	@  echo ': commands' \
	&& echo '  $(PHONY)'
PHONY += help


# Sanity check
ifneq ($(VERSION),debug)
  ifneq ($(VERSION),release)
$(error $$(VERSION) ($(VERSION)) is not correct, need 'debug' or 'release')
  endif
endif


# Settings
.PHONY: $(PHONY)

## do not remove secondary file
.SECONDARY:

## Default target
.DEFAULT_GOAL = build

