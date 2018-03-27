#pragma once
#ifndef PARSER_INTERNAL_H
	#define PARSER_INTERNAL_H

#pragma region Includes
	#include "parser.h"
#pragma endregion

typedef float real;

#define PROMPT	"\nT-SQL >> "

typedef struct {
	char *relName;
	char *attrName;
} REL_ATTR;

typedef struct {
	char *attrName;
	AttrType valType;
	int valLength;
	void *value;
} ATTR_VAL;

typedef enum {
	N_CREATETABLE,
	N_CREATEINDEX,
	N_DROPTABLE,
	N_DROPINDEX,
	N_LOAD,
	N_SET,
	N_HELP,
	N_PRINT,
	N_QUERY,
	N_INSERT,
	N_DELETE,
	N_UPDATE,
	N_RELATTR,
	N_CONDITION,
	N_RELATTR_OR_VALUE,
	N_ATTRTYPE,
	N_VALUE,
	N_RELATION,
	N_STATISTICS,
	N_LIST,
	N_ORDERATTR
} NODEKIND;

typedef struct node {
	NODEKIND kind;
	union {
		struct {
			char *relname;
			struct node *attrlist;
		} CREATETABLE;

		struct {
			char *relname;
			char *attrname;
		} CREATEINDEX;

		struct {
			char *relname;
			char *attrname;
		} DROPINDEX;

		struct {
			char *relname;
		} DROPTABLE;

		struct {
			char *relname;
			char *filename;
		} LOAD;

		struct {
			char *paramName;
			char *string;
		} SET;

		struct {
			char *relname;
		} HELP;

		struct {
			char *relname;
		} PRINT;

		struct {
			struct node *relattrlist;
			struct node *rellist;
			struct node *conditionlist;
		} QUERY;

		struct {
			char *relname;
			struct node *valuelist;
		} INSERT;

		struct {
			char *relname;
			struct node *conditionlist;
		} DELETE;

		struct {
			char *relname;
			struct node *relattr;
			struct node *relorvalue;
			struct node *conditionlist;
		} UPDATE;

		struct {
			char *relname;
			char *attrname;
		} RELATTR;

		struct {
			struct node *lhsRelattr;
			CompOp comparisonOp;
			struct node *rhsRelattr;
			struct node *rhsValue;
		} CONDITION;

		struct {
			struct node *relattr;
			struct node *value;
		} RELATTR_OR_VALUE;

		struct {
			char *attrname;
			char *type;
		} ATTRTYPE;

		struct {
			AttrType type;
			int  ival;
			real rval;
			char *sval;
		} VALUE;

		struct {
			char *relname;
		} RELATION;

		struct {
			struct node *curr;
			struct node *next;
		} LIST;

		struct {
			int order;
			struct node *relattr;
		} ORDERATTR;
	} u;
}NODE;

NODE *newnode(NODEKIND kind);
NODE *create_table_node(char *relname, NODE *attrlist);
NODE *create_index_node(char *relname, char *attrname);
NODE *drop_index_node(char *relname, char *attrname);
NODE *drop_table_node(char *relname);
NODE *load_node(char *relname, char *filename);
NODE *set_node(char *paramName, char *string);
NODE *help_node(char *relname);
NODE *print_node(char *relname);
NODE *query_node(NODE *relattrlist, NODE *rellist, NODE *conditionlist);
NODE *insert_node(char *relname, NODE *valuelist);
NODE *delete_node(char *relname, NODE *conditionlist);
NODE *update_node(char *relname, NODE *relattr, NODE *value,NODE *conditionlist);
NODE *relattr_node(char *relname, char *attrname);
NODE *condition_node(NODE *lhsRelattr, CompOp comparisonOp, NODE *rhsRelattrOrValue);
NODE *value_node(AttrType type, void *value);
NODE *relattr_or_value_node(NODE *relattr, NODE *value);
NODE *attrtype_node(char *attrname, char *type);
NODE *relation_node(char *relname);
NODE *list_node(NODE *n);
NODE *prepend(NODE *n, NODE *list);
NODE *orderattr_node(int order, NODE *relattr);

void reset_scanner(void);
void reset_charptr(void);
void new_query(void);
RC interp(NODE *n);
int yyparse(void);
int yylex(void);
#endif // !PARSER_INTERNAL_H
