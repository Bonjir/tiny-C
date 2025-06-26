
#include "console.h"
#include "token.h"
#include "lexic_analysis.h"
#include "syntree2IR.h"

std::string str_IR;
int n_tmp_cnt, n_if_cnt, n_else_cnt, n_loop_cnt;

std::string IR_TmpExpression_Handler(ST::PNode pn_this_exp, std::string leftval);
std::string IR_Assignment_Handler(ST::PNode pn_this_assignment);
bool IR_CodeBlock_Handler(ST::PNode pn_this_block, std::string label_loopbeg, std::string label_loopend);
bool IR_IfElse_Handler(ST::PNode pn_this_if, std::string label_loopbeg, std::string label_loopend);

bool get_IR(ST::PNode pn) {
	if (pn == nullptr) {
		clerror("运行至空节点.\n");
		return { RT::ERROR };
	}
	switch (pn->type) {
	case ST::Entrance: {
		pn = pn->firstch; // argulist
		pn = pn->nxtbro; // codeblock
		// 断言不为空
		bool ret = get_IR(pn);
		clprintf(CTC_AQUAMAINE, 1, "%s\n", str_IR.c_str());
		return ret;
		break;
	}
	case ST::CodeBlock: {
		return IR_CodeBlock_Handler(pn, "", "");
		break;
	}
	case ST::Declaration: {
#if PRINT_IR_TRACE >= IR_TRACE_STATEMENT
		cllog("in get_ir statement of declaration.\n");
#endif
		for (pn = pn->firstch; pn != nullptr; pn = pn->nxtbro) {
			std::string id = pn->token->content;
			if (pn->firstch) {
				IR_Assignment_Handler(pn);
			}
		}
		return true;
		break;
	}
	case ST::Assignment: {
#if PRINT_IR_TRACE >= IR_TRACE_STATEMENT
		cllog("in get_ir statement of assignment.\n");
#endif
		IR_Assignment_Handler(pn);

		return true;
		break;
	}
	case ST::Output: {
#if PRINT_IR_TRACE >= IR_TRACE_STATEMENT
		cllog("in get_ir statement of output.\n");
#endif-
		pn = pn->firstch; // pn : argulist
		for (pn = pn->firstch; pn != nullptr; pn = pn->nxtbro) {
			std::string id;
			if (pn->type == ST::Expression) {
				id = IR_TmpExpression_Handler(pn, "");
			}
			else if (pn->type == ST::Identifier || pn->type == ST::IntLiteral || pn->type == ST::Const) {
				id = pn->token->content;
			}
			else {
				clerror("in line %d, unexpected arg type of %d.\n", pn->token->idx_line, pn->type);
				continue;
			}

			str_IR += "\tSHOW " + id + "\n";
		}
		return true;
		break;
	}
	case ST::IfElse: {
#if PRINT_IR_TRACE >= IR_TRACE_STATEMENT
		cllog("in get_ir statement of ifelse.\n");
#endif
		return IR_IfElse_Handler(pn, "", "");
		break;
	}
	case ST::While: {
#if PRINT_IR_TRACE >= IR_TRACE_STATEMENT
		cllog("in get_ir statement of while in line %d.\n", GET_PN_IDXLINE(pn));
#endif
		pn = pn->firstch;

		char str_int[64];
		_itoa(n_loop_cnt, str_int, 10);
		n_loop_cnt++;
		std::string label_loopbeg = "LOOPBEG" + std::string(str_int),
			label_loopend = "LOOPEND" + std::string(str_int);

		str_IR += label_loopbeg + ":\n";

		std::string str_condition = IR_TmpExpression_Handler(pn, "");
		str_IR += std::string("\t") + "IFZ " + str_condition + " GOTO " + label_loopend + "\n";

		IR_CodeBlock_Handler(pn->nxtbro, label_loopbeg, label_loopend);
		str_IR += std::string("\t") + "GOTO " + label_loopbeg + "\n";
		str_IR += label_loopend + ":\n";

		return true;
		break;
	}
	case ST::Continue: {
		clerror("Unexpected to encounter continue in get_IR() in line %d.\n", GET_PN_IDXLINE(pn));
		return false;
		break;
	}
	case ST::Break: {
		clerror("Unexpected to encounter break in get_IR() in line %d.\n", GET_PN_IDXLINE(pn));
		return false;
		break;
	}
	case ST::ArguList: {
		clerror("Unexpected to encounter argulist in get_IR() in line %d.\n", GET_PN_IDXLINE(pn));
		return false;
		break;
	}
	case ST::Expression: {
		clerror("Unexpected to encounter expression in get_IR() in line %d.\n", GET_PN_IDXLINE(pn));
		return false;
		break;
	}
	case ST::Identifier: {
		clerror("Unexpected to encounter identifier in get_IR() in line %d.\n", GET_PN_IDXLINE(pn));
		return false;
		break;
	}
	case ST::IntLiteral: {
		clerror("Unexpected to encounter intliteral in get_IR() in line %d.\n", GET_PN_IDXLINE(pn));
		return false;
		break;
	}
	case ST::Const: {
		clerror("Unexpected to encounter const in get_IR() in line %d.\n", GET_PN_IDXLINE(pn));
		return false;
		break;
	}
	}
}


std::string IR_TmpExpression_Handler(ST::PNode pn_this_exp, std::string leftval) {
	// 若leftval == ""则新建tmp，否则进行leftval = this_exp
	std::string str_this_exp = pn_this_exp->token->content;
	bool new_tmp = leftval.length() == 0;
	if (pn_this_exp->type == ST::Identifier || pn_this_exp->type == ST::IntLiteral || pn_this_exp->type == ST::Const) {
		if (leftval.length() == 0) {
			//clwarning("in ir_tmpexp_hdl in line %d, 该表达式不需要新建tmp.\n", pn_this_exp->token->idx_line);
		}
		else {
			str_IR += std::string("\t") + leftval + " = " + str_this_exp + "\n";
		}
		return pn_this_exp->token->content;
	}
	if (pn_this_exp->type != ST::Expression) {
		clerror("in ir_tmpexp_hdl in line %d, unexpected exp type.\n", pn_this_exp->token->idx_line);
		return "";
	}



	// 输出leftval = this_exp

	ST::PNode pn_operand1 = pn_this_exp->firstch, pn_operand2 = pn_this_exp->firstch->nxtbro;
	std::string str_operand1, str_operand2;
	str_operand1 = IR_TmpExpression_Handler(pn_operand1, "");
	if (pn_operand2 == nullptr) {
		if (new_tmp == true) {
			char str_int[64];
			_itoa(n_tmp_cnt, str_int, 10);
			leftval = "T" + std::string(str_int);
			n_tmp_cnt++;
		}
		str_IR += std::string("\t") + leftval + " = " + str_this_exp + str_operand1 + "\n";
		return leftval;
	}
	str_operand2 = IR_TmpExpression_Handler(pn_operand2, "");
	if (new_tmp == true) {
		char str_int[64];
		_itoa(n_tmp_cnt, str_int, 10);
		leftval = "T" + std::string(str_int);
		n_tmp_cnt++;
	}
	str_IR += std::string("\t") + leftval + " = " + str_operand1 + " " + str_this_exp + " " + str_operand2 + "\n";
	return leftval;
}

std::string IR_Assignment_Handler(ST::PNode pn_this_assignment) {
	ST::PNode right = pn_this_assignment->firstch;
	if (right == nullptr) {
		clerror("in line %d, assignment no child.\n", pn_this_assignment->token->idx_line);
		return "";
	}
	std::string id = pn_this_assignment->token->content;
	IR_TmpExpression_Handler(right, id);
	return id;
}

bool IR_CodeBlock_Handler(ST::PNode pn_this_block, std::string label_loopbeg, std::string label_loopend) {
	// label == ""则不处在循环中
	ST::PNode pn_statement = pn_this_block->firstch;
	for (; pn_statement; pn_statement = pn_statement->nxtbro) {
		switch (pn_statement->type) {
		case ST::CodeBlock: {
			IR_CodeBlock_Handler(pn_statement, label_loopbeg, label_loopend);
			break;
		}
		case ST::IfElse: {
			IR_IfElse_Handler(pn_statement, label_loopbeg, label_loopend);
			break;
		}
		case ST::Break: {
			if (label_loopend.length() == 0) {
				clerror("break未处在循环中。\n");
				return { RT::ERROR };
			}
			str_IR += std::string("\t") + "GOTO " + label_loopend + "\n";
			break;
		}
		case ST::Continue: {
			if (label_loopbeg.length() == 0) {
				clerror("break未处在循环中。\n");
				return { RT::ERROR };
			}
			str_IR += std::string("\t") + "GOTO " + label_loopbeg + "\n";
			break;
		}
		default: {
			get_IR(pn_statement);
			break;
		}
		}
	}
	return true;
}

bool IR_IfElse_Handler(ST::PNode pn_this_if, std::string label_loopbeg, std::string label_loopend) {
	// label == ""则不处在循环中
	int nchild = pn_this_if->nchild;
	if (nchild < 2 || nchild > 3) {
		clerror("Ifelse node error in child number.\n");
		return { RT::ERROR };
	}
	pn_this_if = pn_this_if->firstch;
	std::string str_condition_id = IR_TmpExpression_Handler(pn_this_if, "");

	char str_int[64];
	_itoa(n_if_cnt, str_int, 10);
	n_if_cnt++;
	std::string label_endif = "ENDIF" + std::string(str_int), label_endelse;
	str_IR += std::string("\t") + "IFZ " + str_condition_id + " GOTO " + label_endif + "\n";

	IR_CodeBlock_Handler(pn_this_if = pn_this_if->nxtbro, label_loopbeg, label_loopend); // if-block

	if (nchild == 3) {
		_itoa(n_else_cnt, str_int, 10);
		n_else_cnt++;
		label_endelse = "ENDELSE" + std::string(str_int);
		str_IR += std::string("\t") + "GOTO " + label_endelse + "\n";
	}

	str_IR += label_endif + ":\n";

	if (nchild == 3) {
		IR_CodeBlock_Handler(pn_this_if = pn_this_if->nxtbro, label_loopbeg, label_loopend); // else-block
		str_IR += label_endelse + ":\n";
	}

}