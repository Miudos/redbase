#pragma once

#ifndef PARSER_H
	#define PARSER_H

	#pragma region Includes
		#include <iostream>
		#include "redbase.h"
		#include "pf.h"
	#pragma endregion

	struct AttrInfo {
		char *attrName;
		int attrLength;
		AttrType attrType;
	};

	//the friend method operator<< declared is a print function.
	struct RelAttr {
		char *relName;
		char *attrName;
		friend std::ostream& operator<<(std::ostream &s, const RelAttr &ra);
	};

	struct Value {
		AttrType type;
		void *data;
		friend std::ostream& operator<<(std::ostream &s, const Value &v);
	};

	struct Condition {
		RelAttr lhsAttr;
		CompOp comparisonOp;
		int bRhsIsAttr;

		RelAttr rhsAttr;
		Value rhsValue;
		friend std::ostream &operator<<(std::ostream &s, const Condition &c);
	};

	std::ostream &operator<<(std::ostream &s, const CompOp &op);
	std::ostream &operator<<(std::ostream &s, const AttrType &at);

	class QL_Manager;
	class SM_Manager;

	void RBparse(PF_Manager &pfm, SM_Manager &smm, QL_Manager &qlm);

	void PrintError(RC rc);

	extern int bQueryPlans;
#endif // !PARSER_H
