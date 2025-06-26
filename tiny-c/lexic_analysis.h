#pragma once
#ifndef _LEXIC_ANALYSIS_H_
#define _LEXIC_ANALYSIS_H_

#include <string>
#include <vector>
#include <map>
#include "token.h"

TOKENTYPE JudgeTokenType(std::string content);
bool lexic_analyse(FILE* fp, std::vector<Token>& tokenlist); 

extern std::map<std::string, TOKENTYPE> str2tktype;
void get_map_tktype2str(std::map<int, std::string>& tktype2str);

void lexic_analysis_test(const std::vector<Token> tokenlist); // 调试

#endif