#include <SysY.tab.h>
#include <SysY.yy.h>
#include <parser.hh>
#define LINE_SIZE 1024

int parser(const std::string &path, Program &prog) {
    FILE *fp = fopen(path.c_str(), "r");
    YY_BUFFER_STATE buffer = yy_create_buffer(fp, YY_BUF_SIZE);
    yy_switch_to_buffer(buffer);

    int ret = yyparse(&prog);

    yy_delete_buffer(buffer);
    fclose(fp);
    return ret;
}