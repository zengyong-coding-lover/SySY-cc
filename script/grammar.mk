GRAMMAR_DIR = grammar
TMP_DIR     = tmp-src

INC_PATH += $(TMP_DIR)

LEXER =  $(shell find $(GRAMMAR_DIR) -name "*.l")
PASER =  $(shell find $(GRAMMAR_DIR) -name "*.y")
TMPSRCS = $(PASER:$(GRAMMAR_DIR)/%.y=$(TMP_DIR)/%.tab.c) $(LEXER:$(GRAMMAR_DIR)/%.l=$(TMP_DIR)/%.yy.c)

$(TMP_DIR)/%.tab.c: $(GRAMMAR_DIR)/%.y
	@mkdir -p $(TMP_DIR)
	@bison -r solved --defines=$(@:%.c=%.h) -o $@ $<

$(TMP_DIR)/%.yy.c: $(GRAMMAR_DIR)/%.l
	@mkdir -p $(TMP_DIR)
	@flex --header-file=$(@:%.c=%.h) -o $@ $<

CLEAN += $(TMP_DIR)

