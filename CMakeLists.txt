cmake_minimum_required (VERSION 2.8)
project (hello-c)
set(EXECUTABLE_NAME parser)

set(
	HELLO_LIB 
	$<TARGET_OBJECTS:util>
	$<TARGET_OBJECTS:compiler>
	$<TARGET_OBJECTS:absyn>
	)

set(
	SOURCE_FILE_LIST
	util.c
	absyn.c
	compiler.c
)


find_package(BISON)
find_package(FLEX)


bison_target(
	HelloParser hello-c.y ${CMAKE_CURRENT_BINARY_DIR}/parser.c
	COMPILE_FLAGS --debug
	)

flex_target(HelloScanner hello-c.l ${CMAKE_CURRENT_BINARY_DIR}/lexer.c)

add_flex_bison_dependency(HelloScanner HelloParser)

include_directories(
	${CMAKE_CURRENT_BINARY_DIR}
	${CMAKE_SOURCE_DIR}/include
)

# add_library(util OBJECT util.c)

# add_library(compiler OBJECT compiler.c)

# add_library(absyn OBJECT absyn.c)


add_executable(
	${EXECUTABLE_NAME}
	${SOURCE_FILE_LIST}
	# ${HELLO_LIB}
	${BISON_HelloParser_OUTPUTS}
	${FLEX_HelloScanner_OUTPUTS}
	)

# target_link_libraries(
# 	${EXECUTABLE_NAME} 
# 	# ${HELLO_LIB}
# 	# ${FLEX_LIBRARIES}
# 	)
