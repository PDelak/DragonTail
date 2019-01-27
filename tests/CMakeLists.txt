add_custom_command(
    PRE_BUILD
    COMMAND echo ${PROJECT_SOURCE_DIR}
    COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_SOURCE_DIR}/src/grammar.g ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/grammar.g
    COMMAND ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/make_dparser.exe ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/grammar.g
    OUTPUT ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/grammar.g.d_parser.c 
)

if(MSVC)
SET_SOURCE_FILES_PROPERTIES( 
    ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/grammar.g.d_parser.c
	${PROJECT_SOURCE_DIR}/src/dparser/arg.c
	${PROJECT_SOURCE_DIR}/src/dparser/dparse_tree.c
	${PROJECT_SOURCE_DIR}/src/dparser/gram.c
	${PROJECT_SOURCE_DIR}/src/dparser/grammar.g.c
	${PROJECT_SOURCE_DIR}/src/dparser/lex.c
	${PROJECT_SOURCE_DIR}/src/dparser/lr.c
	${PROJECT_SOURCE_DIR}/src/dparser/mkdparse.c
	${PROJECT_SOURCE_DIR}/src/dparser/parse.c
	${PROJECT_SOURCE_DIR}/src/dparser/read_binary.c
	${PROJECT_SOURCE_DIR}/src/dparser/scan.c
	${PROJECT_SOURCE_DIR}/src/dparser/symtab.c
	${PROJECT_SOURCE_DIR}/src/dparser/util.c
	${PROJECT_SOURCE_DIR}/src/dparser/version.c
	${PROJECT_SOURCE_DIR}/src/dparser/write_tables.c
	PROPERTIES LANGUAGE CXX 
	)
else()
SET_SOURCE_FILES_PROPERTIES( 
    ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/grammar.g.d_parser.c
	${PROJECT_SOURCE_DIR}/src/dparser/arg.c
	${PROJECT_SOURCE_DIR}/src/dparser/dparse_tree.c
	${PROJECT_SOURCE_DIR}/src/dparser/gram.c
	${PROJECT_SOURCE_DIR}/src/dparser/grammar.g.c
	${PROJECT_SOURCE_DIR}/src/dparser/lex.c
	${PROJECT_SOURCE_DIR}/src/dparser/lr.c
	${PROJECT_SOURCE_DIR}/src/dparser/mkdparse.c
	${PROJECT_SOURCE_DIR}/src/dparser/parse.c
	${PROJECT_SOURCE_DIR}/src/dparser/read_binary.c
	${PROJECT_SOURCE_DIR}/src/dparser/scan.c
	${PROJECT_SOURCE_DIR}/src/dparser/symtab.c
	${PROJECT_SOURCE_DIR}/src/dparser/util.c
	${PROJECT_SOURCE_DIR}/src/dparser/version.c
	${PROJECT_SOURCE_DIR}/src/dparser/write_tables.c
	)
endif()

set(CPPFILES
	${PROJECT_SOURCE_DIR}/src/ast.cpp
    ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/grammar.g.d_parser.c
    ${PROJECT_SOURCE_DIR}/src/dparser/arg.c
	${PROJECT_SOURCE_DIR}/src/dparser/dparse_tree.c
	${PROJECT_SOURCE_DIR}/src/dparser/gram.c
	${PROJECT_SOURCE_DIR}/src/dparser/grammar.g.c
	${PROJECT_SOURCE_DIR}/src/dparser/lex.c
	${PROJECT_SOURCE_DIR}/src/dparser/lr.c
	${PROJECT_SOURCE_DIR}/src/dparser/mkdparse.c
	${PROJECT_SOURCE_DIR}/src/dparser/parse.c
	${PROJECT_SOURCE_DIR}/src/dparser/read_binary.c
	${PROJECT_SOURCE_DIR}/src/dparser/scan.c
	${PROJECT_SOURCE_DIR}/src/dparser/symtab.c
	${PROJECT_SOURCE_DIR}/src/dparser/util.c
	${PROJECT_SOURCE_DIR}/src/dparser/version.c
	${PROJECT_SOURCE_DIR}/src/dparser/write_tables.c
	testdriver.cpp
  )

set(PRIVATE_HFILES  
  )

include_directories(${PROJECT_SOURCE_DIR}/test)
include_directories(${PROJECT_SOURCE_DIR}/src)
include_directories(${PROJECT_SOURCE_DIR}/src/dparser)

add_executable(testdriver ${CPPFILES} ${PRIVATE_HFILES})
target_link_libraries (testdriver gtest gtest_main)

set_property(TARGET testdriver PROPERTY FOLDER "${CoGeCs_PREFIX}test")

add_test(NAME testdriver COMMAND testdriver)
