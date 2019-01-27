if(MSVC)
	SET_SOURCE_FILES_PROPERTIES( 	
		make_dparser.c
		../src/dparser/grammar.g.d_parser.c
		../src/dparser/arg.c
		../src/dparser/dparse_tree.c
		../src/dparser/gram.c
		../src/dparser/grammar.g.c
		../src/dparser/lex.c
		../src/dparser/lr.c
		../src/dparser/mkdparse.c
		../src/dparser/parse.c
		../src/dparser/read_binary.c
		../src/dparser/scan.c
		../src/dparser/symtab.c
		../src/dparser/util.c
		../src/dparser/version.c
		../src/dparser/write_tables.c	
		PROPERTIES LANGUAGE CXX 
		)
else()
	SET_SOURCE_FILES_PROPERTIES( 	
		make_dparser.c
		../src/dparser/grammar.g.d_parser.c
		../src/dparser/arg.c
		../src/dparser/dparse_tree.c
		../src/dparser/gram.c
		../src/dparser/grammar.g.c
		../src/dparser/lex.c
		../src/dparser/lr.c
		../src/dparser/mkdparse.c
		../src/dparser/parse.c
		../src/dparser/read_binary.c
		../src/dparser/scan.c
		../src/dparser/symtab.c
		../src/dparser/util.c
		../src/dparser/version.c
		../src/dparser/write_tables.c	
		)
endif()

set(CPPFILES
	make_dparser.c
	../src/dparser/grammar.g.d_parser.c
	../src/dparser/arg.c
	../src/dparser/dparse_tree.c
	../src/dparser/gram.c
	../src/dparser/grammar.g.c
	../src/dparser/lex.c
	../src/dparser/lr.c
	../src/dparser/mkdparse.c
	../src/dparser/parse.c
	../src/dparser/read_binary.c
	../src/dparser/scan.c
	../src/dparser/symtab.c
	../src/dparser/util.c
	../src/dparser/version.c
	../src/dparser/write_tables.c	
  )

set(PRIVATE_HFILES  
  )

include_directories(${PROJECT_SOURCE_DIR}/src/dparser)
add_executable(make_dparser ${CPPFILES} ${PRIVATE_HFILES})


