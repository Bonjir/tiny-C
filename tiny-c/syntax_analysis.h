#pragma once
#ifndef _SYNTAX_ANALYSIS_H_
#define _SYNTAX_ANALYSIS_H_

#include <stack>
#include <string>
#include <map>
#include <vector>

#include "token.h"

#define PRINT_HANDLER_TRACE false // CHANGEABLE


#define RUNNING_TRACE_OFF		0
#define RUNNING_TRACE_STATEMENT	1
#define RUNNING_TRACE_IDVAL		2
#define RUNNING_TRACE_INEXPRESSION		3

#define PRINT_RUNNING_TRACE RUNNING_TRACE_OFF // CHANGEABLE

typedef class SyntaxTree {

public:
	enum NodeType {
		Undefined,

		Entrance,	// main
		CodeBlock,
		Declaration,
		Assignment,
		Output,		// show
		IfElse,
		While,
		Continue,
		Break,

		//TypeSpecifier, // replaced by ST::Declaration

		ArguList,	// Function argu list
		Expression, // Operator
		//Operand,
		Identifier,
		IntLiteral,
		Const,		// true, false
		// Additive
		// Multiplacative
	};

	typedef struct Node {
		NodeType type;
		Token* token;
		Node* parent;
		Node* firstch; // first child
		Node* nxtbro; // next brother
		int nchild;

		void AppendChild(Node* child);

		void print(int dColor = CTC_WHITE, bool bBold = false, int tabs = 0);

	}Node, * PNode, * NodeList;

	static PNode MakeNode(NodeType type, PNode parent, Token* token = NULL);

	/*static void FreeNode(PNode pn) {
		if (pn == NULL) {
			return;
		}
		FreeNode(pn->firstch);
		FreeNode(pn->nxtbro);
		delete pn;
	}*/

} SyntaxTree, ST;

ST::PNode syntax_analysis(std::vector<Token>& tokenlist);


#undef VARTYPE
#undef ERROR
#undef NAN

typedef struct Result {
	enum StateCode {
		ERROR,
		BREAK,
		CONTINUE,
		SUCCESS,
	} state_code;

	enum VarableType {
		NAN,
		Int,
		Char,
	} type;
	// void* pval = nullptr;
	int val = 0;// 断言返回值为整形

	Result() : state_code(ERROR), type(NAN), val(0) {};
	Result(StateCode state_code) : type(NAN), val(0) {
		this->state_code = state_code;
	};
	Result(StateCode state_code, VarableType type) : val(0) {
		this->state_code = state_code;
		this->type = type;
	};
	Result(StateCode state_code, VarableType type, int val) {
		this->state_code = state_code;
		this->type = type;
		this->val = val;
	};

	static VarableType get_upper_vartype(VarableType type1, VarableType type2);

	static StateCode get_upper_statecode(StateCode code1, StateCode code2);

	static Result Run(ST::PNode pn);
	static std::map<std::string, Result> map_idval;

}Result, RT;

#define GET_PN_IDXLINE(pn) ((pn)->token != nullptr ? (pn)->token->idx_line : -1)

#endif // !_SYNTAX_ANALYSIS_H_
