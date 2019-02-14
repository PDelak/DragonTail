message(${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
add_custom_command(
    PRE_BUILD
    COMMAND echo ${PROJECT_SOURCE_DIR}
    COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_SOURCE_DIR}/src/grammar.g ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/grammar.g
    COMMAND ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/make_dparser ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/grammar.g
    OUTPUT ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/grammar.g.d_parser.c
)

if(MSVC)
SET_SOURCE_FILES_PROPERTIES( 
    ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/grammar.g.d_parser.c
	dparser/arg.c
	dparser/dparse_tree.c
	dparser/gram.c
	dparser/grammar.g.c
	dparser/lex.c
	dparser/lr.c
	dparser/mkdparse.c
	dparser/parse.c
	dparser/read_binary.c
	dparser/scan.c
	dparser/symtab.c
	dparser/util.c
	dparser/version.c
	dparser/write_tables.c
	PROPERTIES LANGUAGE CXX 
	)
else()
SET_SOURCE_FILES_PROPERTIES( 
    ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/grammar.g.d_parser.c
	dparser/arg.c
	dparser/dparse_tree.c
	dparser/gram.c
	dparser/grammar.g.c
	dparser/lex.c
	dparser/lr.c
	dparser/mkdparse.c
	dparser/parse.c
	dparser/read_binary.c
	dparser/scan.c
	dparser/symtab.c
	dparser/util.c
	dparser/version.c
	dparser/write_tables.c
	)

endif()
set(CPPFILES
    ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/grammar.g.d_parser.c
	driver.cpp
	ast.cpp
	dparser/arg.c
	dparser/dparse_tree.c
	dparser/gram.c
	dparser/grammar.g.c
	dparser/lex.c
	dparser/lr.c
	dparser/mkdparse.c
	dparser/parse.c
	dparser/read_binary.c
	dparser/scan.c
	dparser/symtab.c
	dparser/util.c
	dparser/version.c
	dparser/write_tables.c
  )

set(PRIVATE_HFILES  
  )
include_directories(${PROJECT_SOURCE_DIR}/src)
include_directories(dparser)


add_executable(compiler ${CPPFILES} ${PRIVATE_HFILES})
#target_link_libraries (compiler gtest gtest_main)

#set_property(TARGET compiler PROPERTY FOLDER "${COGECS_PREFIX}test")

#add_test(NAME compiler COMMAND compiler)

add_dependencies(compiler make_dparser)
