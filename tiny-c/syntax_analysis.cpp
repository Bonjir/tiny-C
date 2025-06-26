
#include <stack>
#include <string>
#include <map>
#include <vector>

#include "console.h"
#include "token.h"
#include "syntax_analysis.h"

#define isTokenPunct(token) ispunct((token).content[0]) // 断言token.content不为空

void SyntaxTree::Node::AppendChild(Node* child) {
	child->parent = this;
	child->nxtbro = NULL;

	this->nchild++;
	if (this->firstch == NULL) {
		this->firstch = child;
		return;
	}
	PNode pbro = this->firstch;
	while (pbro->nxtbro) {
		pbro = pbro->nxtbro;
	}
	pbro->nxtbro = child;
}

SyntaxTree::PNode SyntaxTree::MakeNode(NodeType type, PNode parent, Token* token) {
	PNode p = new Node;
	p->type = type;
	p->parent = parent;
	p->token = token;
	p->firstch = NULL;
	p->nxtbro = NULL;
	p->nchild = 0;

	if (parent == NULL) {
		return p;
	}
	parent->nchild++;
	if (parent->firstch == NULL) {
		parent->firstch = p;
		return p;
	}
	PNode pbro = parent->firstch;
	while (pbro->nxtbro) {
		pbro = pbro->nxtbro;
	}
	pbro->nxtbro = p;
	return p;
}

/*static void FreeNode(PNode pn) {
	if (pn == NULL) {
		return;
	}
	FreeNode(pn->firstch);
	FreeNode(pn->nxtbro);
	delete pn;
}*/

const std::map<int, std::string> nodetype2str = {
	{ST::Undefined, "Undefined"},

	{ST::CodeBlock, "CodeBlock"},

	{ST::Entrance, "Entrance"},
	{ST::Declaration, "Declaration"},
	{ST::Assignment, "Assignment"},
	{ST::Output, "Output"},
	{ST::ArguList, "ArguList"},
	{ST::IfElse, "IfElse"},
	{ST::While, "While"},
	{ST::Continue, "Continue"},
	{ST::Break, "Break"},

	//{ST::TypeSpecifier, "TypeSpecifier"},
	{ST::Identifier, "Identifier"},
	{ST::IntLiteral, "IntLiteral"},
	{ST::Const, "Const"},

	{ST::Expression, "Expression"},
	//{ST::Operand, "Operand"},
	// {ST::Additive, "Additive"},
	// {ST::Multiplicative, "Multiplicative"}, 
};

int get_puncttoken_priority(TOKENTYPE punct);
ST::PNode EntranceHandler(std::vector<Token>::iterator& it_st, const std::vector<Token>::iterator it_end);
ST::PNode CodeBlockHandler(std::vector<Token>::iterator& it_st, const std::vector<Token>::iterator it_end, ST::PNode parent);
ST::PNode DeclarationHandler(std::vector<Token>::iterator& it_st, const std::vector<Token>::iterator it_end, ST::PNode parent);
ST::PNode AssignmentHandler(std::vector<Token>::iterator& it_st, const std::vector<Token>::iterator it_end, ST::PNode parent);
ST::PNode OutputHandler(std::vector<Token>::iterator& it_st, const std::vector<Token>::iterator it_end, ST::PNode parent);
ST::PNode FuncArguListHandler(std::vector<Token>::iterator& it_st, const std::vector<Token>::iterator it_end, ST::PNode parent);
ST::PNode IfElseHandler(std::vector<Token>::iterator& it_st, const std::vector<Token>::iterator it_end, ST::PNode parent);
ST::PNode WhileHandler(std::vector<Token>::iterator& it_st, const std::vector<Token>::iterator it_end, ST::PNode parent);
ST::PNode ExpressionHandler(std::vector<Token>::iterator& it_st, const std::vector<Token>::iterator it_end, ST::PNode parent);
ST::PNode ExpressionParenHandler(std::vector<Token>::iterator& it_st, const std::vector<Token>::iterator it_end, ST::PNode parent);

int get_puncttoken_priority(TOKENTYPE punct) {
	if (punct > RightBrace || punct == Undefined) {
		clerror("In func get_puncttoken_priority, tokentype invalid %d.\n", punct);
		return -1;
	}
	if (punct >= Assignment) {
		// 此项目中不计算=以下优先级符号的值
		clerror("In func get_puncttoken_priority, unexpected to get prior of tokentype %d.\n", punct);
		return -1;
	}
	if (punct > SemiColon) {
		// Paren, Brace
		return 0;
	}
	if (punct == Divide) { // 优先级置等
		punct = Multi;
	}
	if (punct == Minus) {
		punct = Plus;
	}

	return (SemiColon + 1 - punct);

}

ST::PNode syntax_analysis(std::vector<Token>& tokenlist) {

	// 语法分析
	// 找到入口
	// 大括号
	// 分号，括号内
	// 优先级， 从左到右 -(负号)、!、*/、+-、<><=>=、==!=、&&、||、=、,

	// 断言第一个token一定是entrance
	auto it_st = tokenlist.begin();
	if (it_st->type != Entrance) {
		clerror("In line %d, tokenlist 不以Entrance开头，断言失败。\n", it_st->idx_line);
		return NULL;
	}
	ST::PNode proot = EntranceHandler(it_st, tokenlist.end());
	return proot;
}

#define HDL_NULLCHECK(x) if((x) == NULL){\
	clerror("In line %d, null check failed.", it_st->idx_line);\
	return NULL;\
}

ST::PNode EntranceHandler(std::vector<Token>::iterator& it_st, const std::vector<Token>::iterator it_end) {

	ST::PNode proot = ST::MakeNode(ST::Entrance, NULL);

	// 断言后面三个token为() {
	if (!(it_st[1].type == LeftParen && it_st[2].type == RightParen && it_st[3].type == LeftBrace)) {
		clerror("In line %d, 入口函数调用语法错误.\n", it_st->idx_line);
		return NULL;
	}
	it_st++;
	HDL_NULLCHECK(FuncArguListHandler(it_st, it_end, proot));

	HDL_NULLCHECK(CodeBlockHandler(it_st, it_end, proot));

	return proot;
}

ST::PNode CodeBlockHandler(std::vector<Token>::iterator& it_st, const std::vector<Token>::iterator it_end, ST::PNode parent) {

	if (it_st->type != LeftBrace) {
		clerror("In CodeBlockHandler, in line %d CodeBlock不以花括号开头。\n", it_st->idx_line);
		return NULL;
	}
	it_st++; // 跳过左花括号

	ST::PNode pn_this_cb = ST::MakeNode(ST::CodeBlock, parent);
	int n_leftbrace_unmatched = 1;
	while (n_leftbrace_unmatched && it_st != it_end) {
		if (it_st->type == LeftBrace) {
			n_leftbrace_unmatched++;
			it_st++;
			continue;
		}
		if (it_st->type == RightBrace) {
			n_leftbrace_unmatched--;
			it_st++;
			continue;
		}

		if (it_st->type == Output) {
			HDL_NULLCHECK(OutputHandler(it_st, it_end, pn_this_cb));
		}
		else if (it_st->type == TypeSpecifier) {
			// Declaration
			HDL_NULLCHECK(DeclarationHandler(it_st, it_end, pn_this_cb));
		}
		else if (it_st->type == Identifier) {
			// Assignment
			HDL_NULLCHECK(AssignmentHandler(it_st, it_end, pn_this_cb));
		}
		else if (it_st->type == If) {
			HDL_NULLCHECK(IfElseHandler(it_st, it_end, pn_this_cb));
		}
		else if (it_st->type == Else) {
			clerror("In line %d, 野生的else。\n", it_st->idx_line);
			return NULL;
		}
		else if (it_st->type == While) {
			HDL_NULLCHECK(WhileHandler(it_st, it_end, pn_this_cb));
		}
		else if (it_st->type == Continue) {
			ST::MakeNode(ST::Continue, pn_this_cb, &(*it_st));
			it_st++;
			if (it_st->type != SemiColon) {
				clerror("In line %d, continue 语句不以;结尾。\n", it_st->idx_line);
				return NULL;
			}
			it_st++;
		}
		else if (it_st->type == Break) {
			ST::MakeNode(ST::Break, pn_this_cb, &(*it_st));
			it_st++;
			if (it_st->type != SemiColon) {
				clerror("In line %d, break 语句不以;结尾。\n", it_st->idx_line);
				return NULL;
			}
			it_st++;
		}
		else {
			clerror("In line %d, Unexpected Syntax in codeblock: %s\n", it_st->idx_line, it_st->content.c_str());
			return NULL;
		}
	}

	if (n_leftbrace_unmatched) {
		clerror("In line %d, codeblock 中有未配对的左花括号。\n", it_st->idx_line);
		return NULL;
	}

	return pn_this_cb;
}

ST::PNode DeclarationHandler(std::vector<Token>::iterator& it_st, const std::vector<Token>::iterator it_end, ST::PNode parent) {
#if PRINT_HANDLER_TRACE
	cllog("in declaration hdl\n");
#endif // PRINT_HANDLER_TRACE

	if (it_st->type != TypeSpecifier) {
		clerror("In Declaration Handler, in line %d expression does not begin with TypeSpecifier.\n", it_st->idx_line);
		return NULL;
	}
	ST::PNode pn_this = ST::MakeNode(ST::Declaration, parent, &(*it_st));
	do {
		it_st++;// 跳过specifier或逗号
		if (it_st->type != Identifier) {
			clerror("In Declaration Handler, in line %d 变量声明格式不正确.\n", it_st->idx_line);
			return NULL;
		}
		ST::PNode pn_id = ST::MakeNode(ST::Identifier, pn_this, &(*it_st));
		it_st++;// 跳过id
		if (it_st->type == Assignment) {
			it_st++;// 跳过等号
			ExpressionHandler(it_st, it_end, pn_id);
		}

	} while (it_st->type == Comma);
	if (it_st->type != SemiColon) {
		clerror("In Declaration Handler, in line %d 未找到分号, encounter with %s.\n", it_st->idx_line, it_st->content.c_str());
		return NULL;
	}
	it_st++;// 跳过分号
	return pn_this;
}

ST::PNode AssignmentHandler(std::vector<Token>::iterator& it_st, const std::vector<Token>::iterator it_end, ST::PNode parent) {
#if PRINT_HANDLER_TRACE
	cllog("in assignment hdl\n");
#endif // PRINT_HANDLER_TRACE

	if (it_st->type != Identifier) {
		clerror("In Assignment Handler, in line %d 变量赋值语句不以变量开头.\n", it_st->idx_line);
		return NULL;
	}
	ST::PNode pn_this = ST::MakeNode(ST::Assignment, parent, &(*it_st));
	it_st++;// 跳过id
	if (it_st->type != Assignment) {
		clerror("In Assignment Handler, in line %d 变量赋值语句格式不正确, encounter with %s.\n", it_st->idx_line, it_st->content.c_str());
		return NULL;
	}
	it_st++;// 跳过等号
	ExpressionHandler(it_st, it_end, pn_this);
	if (it_st->type != SemiColon) {
		clerror("In Assignment Handler, in line %d 未找到分号, encounter with %s.\n", it_st->idx_line, it_st->content.c_str());
		return NULL;
	}
	it_st++;// 跳过分号
	return pn_this;
}

ST::PNode OutputHandler(std::vector<Token>::iterator& it_st, const std::vector<Token>::iterator it_end, ST::PNode parent) {
#if PRINT_HANDLER_TRACE
	cllog("in output hdl in line %d\n", it_st->idx_line);
#endif // PRINT_HANDLER_TRACE

	if (it_st->type != Output) {
		clerror("In Output Handler, in line %d output语句未以show开头.\n", it_st->idx_line);
		return NULL;
	}
	ST::PNode pn_this_func = ST::MakeNode(ST::Output, parent, &(*it_st));
	it_st++;

	FuncArguListHandler(it_st, it_end, pn_this_func);
	if (it_st->type != SemiColon) {
		clerror("In Output Handler, in line %d output语句未以.;结尾 %s\n", it_st->idx_line, it_st->content.c_str());
		return NULL;
	}
	it_st++;
	return pn_this_func;
}

ST::PNode FuncArguListHandler(std::vector<Token>::iterator& it_st, const std::vector<Token>::iterator it_end, ST::PNode parent) {
	if (it_st->type != LeftParen) {
		clerror("In FuncArguList Handler, in line %d func调用未跟随左括号.\n", it_st->idx_line);
		return NULL;
	}
	it_st++;
	ST::PNode pn_this_argulist = ST::MakeNode(ST::ArguList, parent);
	while (it_st != it_end) {
		if (it_st->type == RightParen) {
			break;
		}
		ExpressionHandler(it_st, it_end, pn_this_argulist);
	}

	if (it_st->type != RightParen) {
		clerror("In FuncArguList Handler, in line %d 未找到匹配的右括号.\n", it_st->idx_line);
		return NULL;
	}
	it_st++;
	return pn_this_argulist;
}

ST::PNode IfElseHandler(std::vector<Token>::iterator& it_st, const std::vector<Token>::iterator it_end, ST::PNode parent) {
#if PRINT_HANDLER_TRACE
	cllog("in IfElse hdl in line %d\n", it_st->idx_line);
#endif // PRINT_HANDLER_TRACE

	if (it_st->type != If) {
		clerror("In IfElse Handler, in line %d IfElse语句未以if开头.\n", it_st->idx_line);
		return NULL;
	}
	ST::PNode pn_this_if = ST::MakeNode(ST::IfElse, parent, &(*it_st));
	it_st++;
	if (it_st->type != LeftParen) {
		clerror("In IfElse Handler, in line %d IfElse语句if后未跟随左括号.\n", it_st->idx_line);
		return NULL;
	}
	ExpressionParenHandler(it_st, it_end, pn_this_if);
	if (it_st->type != LeftBrace) {
		clerror("In IfElse Handler, in line %d IfElse语句if()后未跟随左花括号.\n", it_st->idx_line);
		return NULL;
	}
	CodeBlockHandler(it_st, it_end, pn_this_if);

	if (it_st->type == Else) {
		it_st++;
		CodeBlockHandler(it_st, it_end, pn_this_if);
	}
	return pn_this_if;
}

ST::PNode WhileHandler(std::vector<Token>::iterator& it_st, const std::vector<Token>::iterator it_end, ST::PNode parent) {
#if PRINT_HANDLER_TRACE
	cllog("in While hdl in line %d\n", it_st->idx_line);
#endif

	if (it_st->type != While) {
		clerror("In While Handler, in line %d While语句未以While开头.\n", it_st->idx_line);
		return NULL;
	}
	ST::PNode pn_this_while = ST::MakeNode(ST::While, parent, &(*it_st));
	it_st++;
	if (it_st->type != LeftParen) {
		clerror("In While Handler, in line %d While语句While后未跟随左括号.\n", it_st->idx_line);
		return NULL;
	}
	ExpressionParenHandler(it_st, it_end, pn_this_while);
	if (it_st->type != LeftBrace) {
		clerror("In While Handler, in line %d While语句While()后未跟随左花括号.\n", it_st->idx_line);
		return NULL;
	}
	CodeBlockHandler(it_st, it_end, pn_this_while);

	return pn_this_while;
}

ST::PNode ExpressionHandler(std::vector<Token>::iterator& it_st, const std::vector<Token>::iterator it_end, ST::PNode parent) {
#if PRINT_HANDLER_TRACE
	cllog("in expression hdl in line %d\n", it_st->idx_line);
#endif

	int prior_prv = -1;
	std::stack<ST::PNode>operator_stack, operand_stack;

	if (it_st->type == Minus) {
		// 单目负号
		it_st++;
		Token* ptk = new Token;
		ptk->content = "-";
		ptk->type = NegativeSign;
		ST::PNode pnnegat = ST::MakeNode(ST::Expression, NULL, ptk);
		if (it_st->type == LeftParen) {
			HDL_NULLCHECK(ExpressionParenHandler(it_st, it_end, pnnegat));
		}
		else {
			if (it_st->type == Identifier) {
				ST::MakeNode(ST::Identifier, pnnegat, &(*it_st));
			}
			else if (it_st->type == IntLiteral) {
				ST::MakeNode(ST::IntLiteral, pnnegat, &(*it_st));
			}
			else {
				clerror("In ExpHdl, in line %d 单目负号后跟随unexpected类型\n", it_st->idx_line);
				return NULL;
			}
		}
		// 压栈
		operand_stack.push(pnnegat);
	}
	while (it_st != it_end) {
		//cllog("\tit_st->content: %s\n", it_st->content.c_str());
		if (it_st->type == Assignment || it_st->type == Comma || it_st->type == SemiColon || it_st->type == RightParen) {
			// 表达式遇到 =、,、;、未配对的) 结束
			// 全部出栈
			while (operator_stack.size()) {
				ST::PNode pn_top_operator = operator_stack.top();
				if (pn_top_operator->token == NULL) {
					clerror("In func ExpressionHandler, 符号pn_top_operator content为空。\n");
					return NULL;
				}
				operator_stack.pop();

				if (operand_stack.size() < operator_stack.size() + 1) {
					clerror("In func ExpressionHandler, operand stack size < operator stack size + 1，全为双目运算符断言失败\n");
					return NULL;
				}
				ST::PNode pn_top_operand1, pn_top_operand2;
				pn_top_operand2 = operand_stack.top();
				operand_stack.pop();
				pn_top_operand1 = operand_stack.top();
				operand_stack.pop();
				pn_top_operator->AppendChild(pn_top_operand1);
				pn_top_operator->AppendChild(pn_top_operand2);
				operand_stack.push(pn_top_operator);
			}
			break;
		}
		if (it_st->type == LeftParen) {
			ST::PNode pnparen;
			HDL_NULLCHECK(pnparen = ExpressionParenHandler(it_st, it_end, NULL));
			// 压栈
			operand_stack.push(pnparen);
			continue;
		}
		if (it_st->type == Minus) {
			// 由于已经排除expression开头是单目负号
			// 断言it_st 不是begin()
			// 断言it_st 不在无括号if()后面
			if (isTokenPunct(it_st[-1]) && it_st[-1].type != RightParen) {
				// 单目负号
				Token* ptk = new Token;
				*ptk = *it_st;
				ptk->type = NegativeSign;
				ST::PNode pnnegat = ST::MakeNode(ST::Expression, NULL, ptk);
				it_st++;
				if (it_st->type == LeftParen) {
					HDL_NULLCHECK(ExpressionParenHandler(it_st, it_end, pnnegat));
				}
				else {
					if (it_st->type == Identifier) {
						ST::MakeNode(ST::Identifier, pnnegat, &(*it_st));
					}
					else if (it_st->type == IntLiteral) {
						ST::MakeNode(ST::IntLiteral, pnnegat, &(*it_st));
					}
					else {
						clerror("In ExpHdl, in line %d 单目负号后跟随unexpected类型\n", it_st->idx_line);
						return NULL;
					}
				}
				// 压栈
				operand_stack.push(pnnegat);
				continue;
			}
			// 双目负号留待统一处理
		}
		if (it_st->type == Not) {
			// 单目逻辑非
			ST::PNode pnnot = ST::MakeNode(ST::Expression, NULL, &(*it_st));
			it_st++;

			if (it_st->type == LeftParen) {
				HDL_NULLCHECK(ExpressionParenHandler(it_st, it_end, pnnot));
			}
			else {
				if (it_st->type == Identifier) {
					ST::MakeNode(ST::Identifier, pnnot, &(*it_st));
				}
				else if (it_st->type == IntLiteral) {
					ST::MakeNode(ST::IntLiteral, pnnot, &(*it_st));
				}
				else if (it_st->type == True || it_st->type == False) {
					ST::MakeNode(ST::Const, pnnot, &(*it_st));
				}
				else {
					clerror("In ExpHdl, in line %d 单目负号后跟随unexpected类型\n", it_st->idx_line);
					return NULL;
				}
			}

			// 压栈
			operand_stack.push(pnnot);
			continue;
		}

		if (isTokenPunct(*it_st)) {
			ST::PNode pn_this_token = ST::MakeNode(ST::Expression, NULL, &(*it_st));
			// 断言为双目
			int prior_this = get_puncttoken_priority(it_st->type);
			if (prior_this <= prior_prv) {
				// 当前优先级较低，出栈，同级也出栈
				while (operator_stack.size()) {
					ST::PNode pn_top_operator = operator_stack.top();
					if (pn_top_operator->token == NULL) {
						clerror("In func ExpressionHandler, 符号pn_top_operator token为空。\n");
						return NULL;
					}
					if (isTokenPunct(*(pn_top_operator->token)) &&
						get_puncttoken_priority(pn_top_operator->token->type) < prior_this) {
						// 直到前一个低同优先级的符号为止停止出栈
						break;
					}
					operator_stack.pop();

					if (operand_stack.size() < operator_stack.size() + 1) {
						clerror("In func ExpressionHandler, operand stack size < operator stack size + 1，全为双目运算符断言失败\n");
						return NULL;
					}
					ST::PNode pn_top_operand1, pn_top_operand2;
					pn_top_operand2 = operand_stack.top();
					operand_stack.pop();
					pn_top_operand1 = operand_stack.top();
					operand_stack.pop();
					pn_top_operator->AppendChild(pn_top_operand1);
					pn_top_operator->AppendChild(pn_top_operand2);
					operand_stack.push(pn_top_operator);
				}
				// 运算符空栈或者当前运算符优先级低或相同
			}
			prior_prv = prior_this;
			operator_stack.push(pn_this_token);
		}
		else {
			// 操作数
			ST::PNode pn_this_token;
			if (it_st->type == Identifier) {
				pn_this_token = ST::MakeNode(ST::Identifier, NULL, &(*it_st));
			}
			else if (it_st->type == IntLiteral) {
				pn_this_token = ST::MakeNode(ST::IntLiteral, NULL, &(*it_st));
			}
			else if (it_st->type == True || it_st->type == False) {
				pn_this_token = ST::MakeNode(ST::Const, NULL, &(*it_st));
			}
			else {
				clerror("In ExpHdl, in line %d 单目逻辑非后跟随unexpected类型\n", it_st->idx_line);
				return NULL;
			}
			operand_stack.push(pn_this_token);
		}
		it_st++;
	}
	// cllog("in exp hdl, out of while\n");
	// 运算符空栈
	if (operand_stack.size() != 1) {
		return NULL;
	}
	ST::PNode pn_this_expression = operand_stack.top();
	operand_stack.pop();
	if (parent != NULL) {
		parent->AppendChild(pn_this_expression);
	}
	//cllog("in exp hdl, after append child  in line %d\n", it_st->idx_line);
	return pn_this_expression;
}

ST::PNode ExpressionParenHandler(std::vector<Token>::iterator& it_st, const std::vector<Token>::iterator it_end, ST::PNode parent) {
#if PRINT_HANDLER_TRACE
	cllog("in exp paren hdl in line %d\n", it_st->idx_line);
#endif

	if (it_st->type != LeftParen) {
		clerror("In Paren Handler, in line %d expression does not begin with paren.\n", it_st->idx_line);
		return NULL;
	}
	it_st++;
	ST::PNode pn_this_exp_inside = ExpressionHandler(it_st, it_end, parent);
	if (it_st->type != RightParen) {
		clerror("In Paren Handler, in line %d 右括号不匹配, meet with %s.\n", it_st->idx_line, it_st->content.c_str());
		return NULL;
	}
	it_st++;
	return pn_this_exp_inside;
}


void ST::Node::print(int dColor, bool bBold, int tabs) {
	if (this == nullptr) {
		clerror("Object to print is nullptr.\n");
		return;
	}

	for (int i = 0; i < tabs; i++) {
		clprintf(dColor, bBold, "  ");
	}

	// print: pn_this->type, pn_this->token->type, pn_this->token->content
	auto it = nodetype2str.find(this->type);
	if (it != nodetype2str.end()) {
		clprintf(dColor, bBold, "%s ", it->second.c_str());
	}
	else {
		clprintf(dColor, bBold, "Unknown NodeType ");
	}

	if (this->token != NULL) {
		/*std::map<int, std::string> tktype2str;
		get_map_tktype2str(tktype2str);
		clprintf(dColor, bBold, "%s ", tktype2str[this->token->type].c_str());*/
		clprintf(dColor, bBold, "%s ", this->token->content.c_str());
	}
	clprintf(dColor, bBold, "\n");

	if (this->firstch) {
		this->firstch->print(dColor, bBold, tabs + 1);
	}
	if (this->nxtbro) {
		this->nxtbro->print(dColor, bBold, tabs);
	}
}


#define RES_ERRCHECK(x) if((x).state_code == RT::ERROR){\
	return { RT::ERROR };\
}


std::map<std::string, Result> Result::map_idval;

Result::VarableType Result::get_upper_vartype(VarableType type1, VarableType type2) {
	return int(type1) < int(type2) ? type1 : type2;
}

Result::StateCode Result::get_upper_statecode(StateCode code1, StateCode code2) {
	return int(code1) < int(code2) ? code1 : code2;
}

Result Result::Run(ST::PNode pn) {
	if (pn == nullptr) {
		clerror("运行至空节点.\n");
		return { RT::ERROR };
	}
	switch (pn->type) {
	case ST::Entrance: {
		pn = pn->firstch; // argulist
		pn = pn->nxtbro; // codeblock
		// 断言不为空
		return Run(pn);
		break;
	}
	case ST::CodeBlock: {
		pn = pn->firstch;
		while (pn) {
			Result res_statement;
			RES_ERRCHECK(res_statement = Run(pn));
			if (res_statement.state_code == RT::BREAK) {
				return { RT::BREAK };
			}
			if (res_statement.state_code == RT::CONTINUE) {
				return { RT::CONTINUE };
			}
			pn = pn->nxtbro;
		}
		return { RT::SUCCESS };
		break;
	}
	case ST::Declaration: {
#if PRINT_RUNNING_TRACE >= RUNNING_TRACE_STATEMENT
		cllog("run statement of declaration.\n");
#endif
		RT::VarableType type = RT::NAN;
		if (pn->token->content == "int") {
			type = RT::Int;
		}
		else if (pn->token->content == "char") {
			type = RT::Char;
		}
		else {
			clerror("未知的specifier.\n");
		}
		Result res;
		
		for (pn = pn->firstch; pn != nullptr; pn = pn->nxtbro) {
			res = { RT::SUCCESS, type, 0 };
			std::string id = pn->token->content;
			if (pn->firstch) {
				Result res_assign = Run(pn->firstch);
				RES_ERRCHECK(res_assign);
				res.val = res_assign.val;
			}
			auto it_map = map_idval.find(id);
			if (it_map != map_idval.end()) {
				clwarning("In line %d, redeclaration of identifier %s.\n", GET_PN_IDXLINE(pn), pn->token->content.c_str());
			}
			map_idval[id] = res;
#if PRINT_RUNNING_TRACE >= RUNNING_TRACE_IDVAL
			cllog("run declaration of id %s = %d.\n", id.c_str(), res.val);
#endif
		}
		return { RT::SUCCESS };
		break;
	}
	case ST::Assignment: {
#if PRINT_RUNNING_TRACE >= RUNNING_TRACE_STATEMENT
		cllog("run statement of assignment.\n");
#endif
		std::string id = pn->token->content;
		auto it_map = map_idval.find(id);
		if (it_map == map_idval.end()) {
			clwarning("In line %d, undefined identifier %s.\n", GET_PN_IDXLINE(pn), pn->token->content.c_str());
			map_idval[id] = { RT::SUCCESS, RT::Int };
		}
		Result res_exp = Run(pn->firstch);
		if (res_exp.type == RT::NAN) {
			map_idval[id] = res_exp;
		}
		map_idval[id].val = res_exp.val;
		if (map_idval[id].type == RT::Char) {
			map_idval[id].val = char(map_idval[id].val);
		}

#if PRINT_RUNNING_TRACE >= RUNNING_TRACE_IDVAL
		cllog("after assignment id %s = %d.\n", id.c_str(), res_exp.val);
#endif

		return res_exp;
		break;
	}
	case ST::Output: {
#if PRINT_RUNNING_TRACE >= RUNNING_TRACE_STATEMENT
		cllog("run statement of output.\n");
#endif
		pn = pn->firstch; // pn : argulist
		for (pn = pn->firstch; pn != nullptr; pn = pn->nxtbro) {
			Result res = Run(pn);
			if (res.state_code == RT::ERROR) {
				clerror("In line %d, Output error.\n", GET_PN_IDXLINE(pn));
				continue;
			}
			if (res.type == RT::Int) {
				clprintf(CTC_GREEN, 1, "%d\n", res.val);
			}
			else if (res.type == RT::Char) {
				clprintf(CTC_GREEN, 1, "%c\n", res.val);
			}
			else {
				clwarning("In line %d, output unknown type (val: %d).\n", GET_PN_IDXLINE(pn), res.val);
			}
#if PRINT_RUNNING_TRACE >= RUNNING_TRACE_IDVAL
			/*std::string id = pn->token->content;
			cllog("run output id %s = %d.\n", id.c_str(), res.val);
			cllog("map_idval: %d.\n", map_idval[id].val);*/
#endif
		}
		return { RT::SUCCESS };
		break;
	}
	case ST::IfElse: {
#if PRINT_RUNNING_TRACE >= RUNNING_TRACE_STATEMENT
		cllog("run statement of ifelse.\n");
#endif
		int nchild = pn->nchild;
		if (nchild < 2 || nchild > 3) {
			clerror("Ifelse node error in child number.\n");
			return { RT::ERROR };
		}
		pn = pn->firstch;
		Result condition = Run(pn);
		if (condition.state_code == RT::ERROR) {
			clerror("In ifelse node, condition evaluate error.\n");
			return { RT::ERROR };
		}
		if (condition.type == RT::NAN) {
			clerror("In ifelse node, condition type error.\n");
		}
		Result res_codeblock;
		if (condition.val) {
			RES_ERRCHECK(res_codeblock = Run(pn->nxtbro));
		}
		else if (nchild == 3) {
			// contain else codeblock
			RES_ERRCHECK(res_codeblock = Run(pn->nxtbro->nxtbro));
		}
		return { get_upper_statecode(res_codeblock.state_code, RT::SUCCESS) };
		break;
	}
	case ST::While: {
#if PRINT_RUNNING_TRACE >= RUNNING_TRACE_STATEMENT
		cllog("run statement of while in line %d.\n", GET_PN_IDXLINE(pn));
#endif
		pn = pn->firstch;
		Result condition;
		do {
			condition = Run(pn);
			if (condition.state_code == RT::ERROR) {
				clerror("In while node, condition evaluate error.\n");
				return { RT::ERROR };
			}
			if (condition.type == RT::NAN) {
				clerror("In while node, condition type error.\n");
			}
			if (condition.val) {
				Result res_codeblock;
				RES_ERRCHECK(res_codeblock = Run(pn->nxtbro));
				if (res_codeblock.state_code == RT::BREAK) {
					break;
				}
			}
		} while (condition.val);
		return { RT::SUCCESS };
		break;
	}
	case ST::Continue: {
#if PRINT_RUNNING_TRACE >= RUNNING_TRACE_STATEMENT
		cllog("run statement of continue in line %d.\n", GET_PN_IDXLINE(pn));
#endif
		return { RT::CONTINUE };
		break;
	}
	case ST::Break: {
#if PRINT_RUNNING_TRACE >= RUNNING_TRACE_STATEMENT
		cllog("run statement of break in line %d.\n", GET_PN_IDXLINE(pn));
#endif
		return { RT::BREAK };
		break;
	}
	case ST::ArguList: {
		clerror("ArguList node should not be evaluated.\n");
		return { RT::ERROR };
		break;
	}
	case ST::Expression: {
#if PRINT_RUNNING_TRACE >= RUNNING_TRACE_IDVAL
	cllog("run expression (%s) in line %d.\n", pn->token->content.c_str(), GET_PN_IDXLINE(pn));
#endif
		switch (pn->token->type) {
		// 单目
		case NegativeSign:
		case Not: {
			Result res = Run(pn->firstch);
			res.val = (
				pn->token->type == NegativeSign ?	-res.val :
				pn->token->type == Not ?	(bool)!(res.val) :
				-1 // 断言不会有其他运算符
				);
#if PRINT_RUNNING_TRACE >= RUNNING_TRACE_INEXPRESSION
	cllog("after performing (%s) expression = %d.\n", pn->token->content.c_str(), res.val);
#endif
			return res;

		}
		// 双目
		case Multi: 
		case Divide:
		case Plus:
		case Minus:
		case GT:
		case GTE:
		case LT:
		case LTE:
		case EQ:
		case NEQ:
		case AndArith:
		case OrArith:
		case AND:
		case OR:
		case Assignment:
		case Comma: {
			Result res1 = Run(pn->firstch), res2 = Run(pn->firstch->nxtbro);
			if (pn->token->type == Divide && res2.val == 0) {
				clwarning("Divided by zero.\n");
				res1.type = RT::NAN;
				res1.state_code = RT::get_upper_statecode(res1.state_code, res2.state_code);
				return res1;
			}

			res1.val = (
				pn->token->type == Multi ?	(res1.val * res2.val) :
				pn->token->type == Divide ?	(res1.val / res2.val) :
				pn->token->type == Plus ?	(res1.val + res2.val) :
				pn->token->type == Minus ?	(res1.val - res2.val) :
				pn->token->type == GT ?		(res1.val > res2.val) :
				pn->token->type == GTE ?	(res1.val >= res2.val) :
				pn->token->type == LT ?		(res1.val < res2.val) :
				pn->token->type == LTE ?	(res1.val <= res2.val) :
				pn->token->type == EQ ?		(res1.val == res2.val) :
				pn->token->type == NEQ ?	(res1.val != res2.val) :
				pn->token->type == AndArith ?	(res1.val & res2.val) :
				pn->token->type == OrArith ?	(res1.val | res2.val) :
				pn->token->type == AND ?	bool(res1.val && res2.val) :
				pn->token->type == OR ?		bool(res1.val || res2.val) :
				pn->token->type == Assignment ?	(res2.val) :
				pn->token->type == Comma ?	(res2.val) :
				-1 // 断言不会有其他运算符
				);

			res1.type = RT::get_upper_vartype(res1.type, res2.type);
			res1.state_code = RT::get_upper_statecode(res1.state_code, res2.state_code);
#if PRINT_RUNNING_TRACE >= RUNNING_TRACE_INEXPRESSION
			cllog("after performing (%s) expression = %d.\n", pn->token->content.c_str(), res1.val);
#endif
			return res1;
		}
		}
		break;
	}
	case ST::Identifier: {
		std::string id = pn->token->content;
#if PRINT_RUNNING_TRACE >= RUNNING_TRACE_IDVAL
		cllog("run id of %s(%d) in line %d.\n", id.c_str(), map_idval[id].val, GET_PN_IDXLINE(pn));
		
#endif
		auto it_map = map_idval.find(id);
		if (it_map == map_idval.end()) {
			clerror("In line %d, undefined identifier %s.\n", GET_PN_IDXLINE(pn), pn->token->content.c_str());
			return { RT::ERROR };
		}
		return it_map->second;
		break;
	}
	case ST::IntLiteral: {
#if PRINT_RUNNING_TRACE >= RUNNING_TRACE_IDVAL
		cllog("run intliteral of %s in line %d.\n", pn->token->content.c_str(), GET_PN_IDXLINE(pn));
#endif
		std::string content = pn->token->content;
		// 断言content是数字
		int val = atoi(content.c_str());
		return { RT::SUCCESS, RT::Int, val };
		break;
	}
	case ST::Const: {
#if PRINT_RUNNING_TRACE >= RUNNING_TRACE_IDVAL
		cllog("run const of %s in line %d.\n", pn->token->content.c_str(), GET_PN_IDXLINE(pn));
#endif
		if (pn->token->type == True) {
			return { RT::SUCCESS, RT::Int, 1 };
		}
		if (pn->token->type == False) {
			return { RT::SUCCESS, RT::Int, 0 };
		}
		clerror("In line %d, unknown const type %s.\n", GET_PN_IDXLINE(pn), pn->token->content.c_str());
		return { RT::ERROR };
		break;
	}
	}
}

