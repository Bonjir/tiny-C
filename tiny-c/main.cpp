
#include <cstdio>
#include <vector>
#include "console.h"
#include "token.h"
#include "lexic_analysis.h"
#include "syntax_analysis.h"
#include "syntree2IR.h"

int main(int argc, char** argv) {
	// 词法分析
	FILE* fp = NULL;
	if (argc == 1) {
		// 没有参数输入，默认从根目录打开
		//fp = fopen("input.tiny.c", "r+");
		fp = fopen("test_plus.c", "r+");
	}
	else {
		// 参数输入
		// ...
	}

	if (fp == NULL) {
		clprintf(CTC_RED, 0, "打开文件失败!\n");
		return 1;
	}

	std::vector<Token> tokenlist;

	// 词法分析
	lexic_analyse(fp, tokenlist);

	fclose(fp);

	// 词法分析测试
	lexic_analysis_test(tokenlist);

	ST::PNode proot = syntax_analysis(tokenlist);

	if (proot == NULL) {
		clerror("create syntaxtree failed\n");
	}
	else {
		proot->print();
	}

	Result::Run(proot);

	get_IR(proot);

	return 0;
}
