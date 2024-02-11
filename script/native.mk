# Build rules
include script/build.mk


# Phony rules
run: $(BINARY)
	@echo '> run $^ $(ARGS)'
	@$(BINARY) $(ARGS)
PHONY += run

