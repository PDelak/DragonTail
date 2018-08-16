
SET_SOURCE_FILES_PROPERTIES( 	
	make_dparser.c
	../cogecs/dparser/grammar.g.d_parser.c
	../cogecs/dparser/arg.c
	../cogecs/dparser/dparse_tree.c
	../cogecs/dparser/gram.c
	../cogecs/dparser/grammar.g.c
	../cogecs/dparser/lex.c
	../cogecs/dparser/lr.c
	../cogecs/dparser/mkdparse.c
	../cogecs/dparser/parse.c
	../cogecs/dparser/read_binary.c
	../cogecs/dparser/scan.c
	../cogecs/dparser/symtab.c
	../cogecs/dparser/util.c
	../cogecs/dparser/version.c
	../cogecs/dparser/write_tables.c	
	PROPERTIES LANGUAGE CXX 
	)

set(CPPFILES
	make_dparser.c
	../cogecs/dparser/grammar.g.d_parser.c
	../cogecs/dparser/arg.c
	../cogecs/dparser/dparse_tree.c
	../cogecs/dparser/gram.c
	../cogecs/dparser/grammar.g.c
	../cogecs/dparser/lex.c
	../cogecs/dparser/lr.c
	../cogecs/dparser/mkdparse.c
	../cogecs/dparser/parse.c
	../cogecs/dparser/read_binary.c
	../cogecs/dparser/scan.c
	../cogecs/dparser/symtab.c
	../cogecs/dparser/util.c
	../cogecs/dparser/version.c
	../cogecs/dparser/write_tables.c	
  )

set(PRIVATE_HFILES  
  )

#include_directories(${PROJECT_SOURCE_DIR}/cogecs)
#include_directories(dparser)
add_executable(make_dparser ${CPPFILES} ${PRIVATE_HFILES})


