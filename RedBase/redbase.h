#pragma once

#ifndef REDBASE_H
	#define REDBASE_H

	#define MAXNAME       24                // maximum name length of a relation(table) or attribute name(column)
	#define MAXSTRINGLEN  255               // maximum length of a attribute(column) of a type String. Eg.: c1 to c255
	#define MAXATTRS      40                // maximum number of attributes(clomuns) in a relation(table)

		#define OK_RC         0    // OK_RC return code is guaranteed to always be 0

		#define START_PF_ERR  (-1)
		#define END_PF_ERR    (-100)
		#define START_RM_ERR  (-101)
		#define END_RM_ERR    (-200)
		#define START_IX_ERR  (-201)
		#define END_IX_ERR    (-300)
		#define START_SM_ERR  (-301)
		#define END_SM_ERR    (-400)
		#define START_QL_ERR  (-401)
		#define END_QL_ERR    (-500)

		#define START_PF_WARN  1
		#define END_PF_WARN    100
		#define START_RM_WARN  101
		#define END_RM_WARN    200
		#define START_IX_WARN  201
		#define END_IX_WARN    300
		#define START_SM_WARN  301
		#define END_SM_WARN    400
		#define START_QL_WARN  401
		#define END_QL_WARN    500

		/*
		* The parser is implemented using flex and yacc
		* The "yy" indicates that the lex output is intended for a yacc parser.
		* The Yacc parser uses only names beginning in "yy"
		*
		*/
		#define YY_SKIP_YYWRAP 1
		#define yywrap() 1
		void yyerror(const char *);

	//This method should call the corresponding method PF_FileHandle::ForcePages in order to copy
	//the contents of one or all dirty pages of the file from the buffer pool to disk
	const int ALL_PAGES = -1;

	//Return Codes
	typedef int RC;

	enum AttrType {
		INT,
		FLOAT,
		STRING
	};

	// Comparison operators
	enum CompOp {
		NO_OP,                                      // no comparison
		EQ_OP, NE_OP, LT_OP, GT_OP, LE_OP, GE_OP    // binary atomic operators
	};

	// Pin Strategy Hint
	enum ClientHint {
		NO_HINT,          // default value
		KEEP_PAGES
	};

	#pragma region Defines
	#ifndef Boolean
		typedef char Boolean;
	#endif

	#ifndef TRUE
	#define TRUE 1
	#endif

	#ifndef FALSE
	#define FALSE 0
	#endif

	#ifndef NULL
	#define NULL 0
	#endif

	#ifndef RC
		typedef int RC;
	#endif

	#ifndef STAT_BASE
		const int STAT_BASE = 9000;
	#endif
	#pragma endregion

#endif // ! REDBASE_H
