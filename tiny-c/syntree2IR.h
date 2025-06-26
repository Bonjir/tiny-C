#pragma once
#ifndef _SYNTREE2IR_H_
#define _SYNTREE2IR_H_

#include "syntax_analysis.h"

bool get_IR(ST::PNode pn);

#define IR_TRACE_OFF		0
#define IR_TRACE_STATEMENT	1
#define PRINT_IR_TRACE IR_TRACE_OFF // CHANGEABLE

#endif // !_SYNTREE2IR_H_
