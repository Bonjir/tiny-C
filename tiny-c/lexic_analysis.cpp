#include <cstdio>
#include <string>
#include <cctype>
#include <vector>
#include <map>
#include "console.h"
#include "token.h"

#define isspace(ch) ((ch) == EOF ? true : isspace(ch))

std::map<std::string, TOKENTYPE> str2tktype = {
	{"!", Not},
	{"*", Multi},
	{"/", Divide},
	{"+", Plus},
	{"-", Minus},
	{">", GT},
	{">=", GTE},
	{"<", LT},
	{"<=", LTE},
	{"==", EQ},
	{"!=", NEQ},
	{"&", AndArith},
	{"|", OrArith},
	{"&&", AND},
	{"||", OR},
	{"=", Assignment},
	{",", Comma},
	{";", SemiColon},
	{"(", LeftParen},
	{")", RightParen},
	{"{", LeftBrace},
	{"}", RightBrace},
	{"if", If},
	{"else", Else},
	{"while", While},
	{"continue", Continue},
	{"break", Break},
	{"int", TypeSpecifier},
	{"char", TypeSpecifier},
	{"main", Entrance},
	{"show", Output},
};

void get_map_tktype2str(std::map<int, std::string> & tktype2str) {
	for (const auto& pair : str2tktype) {
		tktype2str[pair.second] = pair.first;
	}
	// 不止有一种表现的tokentype在这里修正
	tktype2str[TypeSpecifier] = "TypeSpecifier";
	tktype2str[Identifier] = "Identifier";
	tktype2str[IntLiteral] = "IntLiteral";
}

TOKENTYPE JudgeTokenType(std::string content) {
	// 根据token内容判断token类型

	TOKENTYPE tktype = str2tktype[content];
	if (tktype) {
		return tktype;
	}

	// 非已知符号或关键字
	if (content.length() == 0) {
		clprintf(CTC_RED, 0, "In Func JudgeTokenType, token content length == 0.\n");
		return Undefined;
	}
	if (isalpha(content[0])) {
		return Identifier;
	}
	if (isdigit(content[0])) {
		// 断言该content有效
		return IntLiteral;
	}
	//clprintf(CTC_RED, 0, "In JudgeTokenType(), Unknown token %s.\n", content.c_str());
	return Undefined;
}

bool lexic_analyse(FILE* fp, std::vector<Token>& tokenlist) {
	// 从文件读入同时分析token

	enum TokenStartsWith {
		Undefined,
		Alpha,
		Punct,
		Digit
	} token_starts_with = Undefined;

	Token token_this;

	// logger
	int idx_line = 1;

	while (!feof(fp)) {
		char ch = fgetc(fp);
		if (ch == '\n') {
			idx_line++; // 记录行数
		}
		token_this.idx_line = idx_line;
		switch (token_starts_with) {
		case Undefined: {
			if (isspace(ch)) {
				token_starts_with = Undefined;
				break;
			}
			// 如果不是空字符，记录到token content中
			token_this.content += ch;
			if (isalpha(ch)) {
				token_starts_with = Alpha;
				break;
			}
			if (isdigit(ch)) {
				token_starts_with = Digit;
				break;
			}
			if (ispunct(ch)) {
				token_starts_with = Punct;
				break;
			}

			clprintf(CTC_RED, 0, "In line %d, Token 含有未知字符 %c.\n", idx_line, ch);
			break;
		}
		case Alpha: {
			if (isspace(ch)) {
				// 结束本token
				token_this.type = JudgeTokenType(token_this.content);
				if (token_this.type == Undefined) {
					clprintf(CTC_RED, 0, "In line %d, Token type undefined %s, expected as type %d.\n",
						idx_line, token_this.content.c_str(), token_starts_with);
				}
				tokenlist.push_back(token_this);
				token_this.clear();
				token_starts_with = Undefined;
				break;
			}

			if (ispunct(ch)) {
				// 结束本token并开始下一token
				token_this.type = JudgeTokenType(token_this.content);
				if (token_this.type == Undefined) {
					clprintf(CTC_RED, 0, "In line %d, Token type undefined %s, expected as type %d.\n",
						idx_line, token_this.content.c_str(), token_starts_with);
				}
				tokenlist.push_back(token_this);
				token_this.clear();

				token_starts_with = Punct;
				token_this.content += ch;
				break;
			}

			token_this.content += ch;
			if (isalpha(ch)) {
				break;
			}
			if (isdigit(ch)) {
				break;
			}
			clprintf(CTC_RED, 0, "In line %d, Token 含有未知字符 %c.\n", idx_line, ch);
			break;
		}
		case Digit: {
			if (isspace(ch)) {
				// 结束本token
				token_this.type = IntLiteral;
				tokenlist.push_back(token_this);
				token_this.clear();
				token_starts_with = Undefined;
				break;
			}

			if (ispunct(ch)) {
				// 结束本token并开始下一token
				token_this.type = IntLiteral;
				tokenlist.push_back(token_this);
				token_this.clear();

				token_starts_with = Punct;
				token_this.content += ch;
				break;
			}

			token_this.content += ch;
			if (isdigit(ch)) {
				break;
			}

			if (isalpha(ch)) {
				clprintf(CTC_YELLOW, 0, "In line %d, 非法声明符 %s.\n", idx_line, token_this);
				break;
			}
			clprintf(CTC_RED, 0, "In line %d, Token 含有未知字符 %c.\n", idx_line, ch);
			break;
		}
		case Punct: {
			if (isspace(ch)) {
				// 结束本token
				token_this.type = JudgeTokenType(token_this.content);
				if (token_this.type == Undefined) {
					clprintf(CTC_RED, 0, "In line %d, Token type undefined %s, expected as type %d.\n",
						idx_line, token_this.content.c_str(), token_starts_with);
				}
				tokenlist.push_back(token_this);
				token_this.clear();
				token_starts_with = Undefined;
				break;
			}
			if (isdigit(ch)) {
				// 结束本token并开始下一token
				token_this.type = JudgeTokenType(token_this.content);
				if (token_this.type == Undefined) {
					clprintf(CTC_RED, 0, "In line %d, Token type undefined %s, expected as type %d.\n",
						idx_line, token_this.content.c_str(), token_starts_with);
				}
				tokenlist.push_back(token_this);
				token_this.clear();

				token_starts_with = Digit;
				token_this.content += ch;
				break;
			}
			if (isalpha(ch)) {
				// 结束本token并开始下一token
				token_this.type = JudgeTokenType(token_this.content);
				if (token_this.type == Undefined) {
					clprintf(CTC_RED, 0, "In line %d, Token type undefined %s, expected as type %d.\n", idx_line, token_this.content.c_str(), token_starts_with);
				}
				tokenlist.push_back(token_this);
				token_this.clear();

				token_starts_with = Alpha;
				token_this.content += ch;
				break;
			}

			if (ispunct(ch)) {
				// 根据是否契合原符号选择性结束本token

				token_this.type = JudgeTokenType(token_this.content);
				if (token_this.type == Undefined) {
					// 原有符号出错
					clprintf(CTC_RED, 0, "In line %d, Token type undefined %s, expected as type %d.\n",
						idx_line, token_this.content.c_str(), token_starts_with);

					tokenlist.push_back(token_this);
					token_this.clear();

					token_starts_with = Punct;
					token_this.content += ch;
					break;
				}

				TOKENTYPE tktype_new = JudgeTokenType(token_this.content + ch);
				if (tktype_new == Undefined) {
					// 该符号与原符号不契合
					tokenlist.push_back(token_this);
					token_this.clear();

					token_starts_with = Punct;
					token_this.content += ch;
					break;
				}

				token_this.content += ch;
				break;
			}
			clprintf(CTC_RED, 0, "In line %d, Token 含有未知字符 %c.\n", idx_line, ch);
			break;
		}
		default: {
			clprintf(CTC_RED, 0, "In line %d, token 起始状态错误 ch: %c, token_starts_with: %d.\n",
				idx_line, ch, token_starts_with);
			token_starts_with = Undefined;
			break;
		}
		}
	}
	return true;
}

void lexic_analysis_test(const std::vector<Token> tokenlist) {
	// lexic_analysis调试

	clprintf(CTC_WHITE, 1, "lexic_analysis()测试：\n");
	// 调试用反向map
	std::map<int, std::string> tktype2str;
	get_map_tktype2str(tktype2str);

	for (const auto& token : tokenlist) {
		std::string str_type_this = tktype2str[token.type];
		if (str_type_this.length()) {
			clprintf(CTC_WHITE, 0, "%s : %s\n", token.content.c_str(), str_type_this.c_str());
		}
		else {
			clprintf(CTC_RED, 0, "Unknown Token type: %s\n", token.content.c_str());
		}
	}

}
