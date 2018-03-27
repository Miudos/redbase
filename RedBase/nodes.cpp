#pragma region Includes
	#include <string.h>

	#include "parser_internal.h"
#pragma endregion

#define MAXNODE		100

static NODE nodepool[MAXNODE];
static int nodeptr = 0;

void reset_parser(void)
{
	reset_scanner();
	nodeptr = 0;
}

static void(*cleanup_func)() = NULL;

void new_query(void)
{
	nodeptr = 0;
	reset_charptr();
	if (cleanup_func != NULL)
		(*cleanup_func)();
}

void register_cleanup_function(void(*func)())
{
	cleanup_func = func;
}

NODE *newnode(NODEKIND kind)
{
	NODE *n;

	if (nodeptr == MAXNODE) {
		fprintf(stderr, "out of memory\n");
		exit(1);
	}

	n = nodepool + nodeptr;
	++nodeptr;

	n->kind = kind;
	return n;
}

NODE *create_table_node(char *relname, NODE *attrlist)
{
	NODE *n = newnode(N_CREATETABLE);

	n->u.CREATETABLE.relname = relname;
	n->u.CREATETABLE.attrlist = attrlist;
	return n;
}

NODE *create_index_node(char *relname, char *attrname)
{
	NODE *n = newnode(N_CREATEINDEX);

	n->u.CREATEINDEX.relname = relname;
	n->u.CREATEINDEX.attrname = attrname;
	return n;
}

NODE *drop_index_node(char *relname, char *attrname)
{
	NODE *n = newnode(N_DROPINDEX);

	n->u.DROPINDEX.relname = relname;
	n->u.DROPINDEX.attrname = attrname;
	return n;
}

NODE *drop_table_node(char *relname)
{
	NODE *n = newnode(N_DROPTABLE);

	n->u.DROPTABLE.relname = relname;
	return n;
}

NODE *load_node(char *relname, char *filename)
{
	NODE *n = newnode(N_LOAD);

	n->u.LOAD.relname = relname;
	n->u.LOAD.filename = filename;
	return n;
}

NODE *set_node(char *paramName, char *string)
{
	NODE *n = newnode(N_SET);

	n->u.SET.paramName = paramName;
	n->u.SET.string = string;
	return n;
}

NODE *help_node(char *relname)
{
	NODE *n = newnode(N_HELP);

	n->u.HELP.relname = relname;
	return n;
}

NODE *print_node(char *relname)
{
	NODE *n = newnode(N_PRINT);

	n->u.PRINT.relname = relname;
	return n;
}

NODE *query_node(NODE *relattrlist, NODE *rellist, NODE *conditionlist)
{
	NODE *n = newnode(N_QUERY);

	n->u.QUERY.relattrlist = relattrlist;
	n->u.QUERY.rellist = rellist;
	n->u.QUERY.conditionlist = conditionlist;
	return n;
}

NODE *insert_node(char *relname, NODE *valuelist)
{
	NODE *n = newnode(N_INSERT);

	n->u.INSERT.relname = relname;
	n->u.INSERT.valuelist = valuelist;
	return n;
}

NODE *delete_node(char *relname, NODE *conditionlist)
{
	NODE *n = newnode(N_DELETE);

	n->u.DELETE.relname = relname;
	n->u.DELETE.conditionlist = conditionlist;
	return n;
}

NODE *update_node(char *relname, NODE *relattr, NODE *relorvalue, NODE *conditionlist)
{
	NODE *n = newnode(N_UPDATE);

	n->u.UPDATE.relname = relname;
	n->u.UPDATE.relattr = relattr;
	n->u.UPDATE.relorvalue = relorvalue;
	n->u.UPDATE.conditionlist = conditionlist;
	return n;
}

NODE *relattr_node(char *relname, char *attrname)
{
	NODE *n = newnode(N_RELATTR);

	n->u.RELATTR.relname = relname;
	n->u.RELATTR.attrname = attrname;
	return n;
}

NODE *condition_node(NODE *lhsRelattr, CompOp comparisonOp, NODE *rhsRelattrOrValue)
{
	NODE *n = newnode(N_CONDITION);

	n->u.CONDITION.lhsRelattr = lhsRelattr;
	n->u.CONDITION.comparisonOp = comparisonOp;
	n->u.CONDITION.rhsRelattr = rhsRelattrOrValue->u.RELATTR_OR_VALUE.relattr;
	n->u.CONDITION.rhsValue = rhsRelattrOrValue->u.RELATTR_OR_VALUE.value;
	return n;
}

NODE *value_node(AttrType type, void *value)
{
	NODE *n = newnode(N_VALUE);

	n->u.VALUE.type = type;
	switch (type) {
	case INT:
		n->u.VALUE.ival = *(int *)value;
		break;
	case FLOAT:
		n->u.VALUE.rval = *(float *)value;
		break;
	case STRING:
		n->u.VALUE.sval = (char *)value;
		break;
	}
	return n;
}

NODE *relattr_or_value_node(NODE *relattr, NODE *value)
{
	NODE *n = newnode(N_RELATTR_OR_VALUE);

	n->u.RELATTR_OR_VALUE.relattr = relattr;
	n->u.RELATTR_OR_VALUE.value = value;
	return n;
}

NODE *attrtype_node(char *attrname, char *type)
{
	NODE *n = newnode(N_ATTRTYPE);

	n->u.ATTRTYPE.attrname = attrname;
	n->u.ATTRTYPE.type = type;
	return n;
}

NODE *relation_node(char *relname)
{
	NODE *n = newnode(N_RELATION);

	n->u.RELATION.relname = relname;
	return n;
}

NODE *list_node(NODE *n)
{
	NODE *list = newnode(N_LIST);

	list->u.LIST.curr = n;
	list->u.LIST.next = NULL;
	return list;
}

NODE *prepend(NODE *n, NODE *list)
{
	NODE *newlist = newnode(N_LIST);

	newlist->u.LIST.curr = n;
	newlist->u.LIST.next = list;
	return newlist;
}