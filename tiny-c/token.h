#pragma once
#ifndef _TOKEN_H_
#define _TOKEN_H_

#include <string>

typedef enum {
	Undefined,	// 未设置

	NegativeSign,	// 单目负号 (not in use in lexic analysis)
	Not,	// !
	Multi,	// *
	Divide, // /
	Plus,	// +
	Minus,	// -
	GT,		// >
	GTE,	// >=
	LT,		// <
	LTE,	// <=
	EQ,		// ==
	NEQ,	// !=
	AndArith,	// &
	OrArith,	// |
	AND,	// &&
	OR,		// ||
	Assignment, // =
	Comma,	// ,
	SemiColon,  // ;
	LeftParen,  // (
	RightParen, // )
	LeftBrace,  // { 
	RightBrace, // }

	If,         // if
	Else,       // else
	While,		// while
	Continue,	// continue
	Break,		// break

	True,		// TRUE
	False,		// FALSE

	TypeSpecifier,	// 类型声明符
	//Int,		// int
	//Char,		// char

	Entrance,	// 入口函数
	Output,		// show输出函数

	IntLiteral,	// 整形字面量
	
	Identifier,	// 标识符 varid
	
} TOKENTYPE;

struct Token {

	TOKENTYPE type;

	std::string content;

	// for logger
	int idx_line;

	Token() {
		this->clear();
		idx_line = -1;
	}

	void clear() {
		this->type = Undefined;
		this->content = "";
	}
};

#endif