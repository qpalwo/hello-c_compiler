cmake_minimum_required (VERSION 2.8)
project (hello-c)
set(EXECUTABLE_NAME parser)
set (CMAKE_CXX_STANDARD 11)
set(LLVM_DIR /Users/xiaoyuxuan/SDK/clang+llvm-9.0.0-x86_64-darwin-apple/lib/cmake/llvm)

set(
	HELLO_LIB 
	$<TARGET_OBJECTS:util>
	$<TARGET_OBJECTS:compiler>
	$<TARGET_OBJECTS:absyn>
	)

set(
	SOURCE_FILE_LIST
	absyn.c
	astprinter.c
	check.c
	codegen.c
	compiler.c
	env.c
	label.c
	mllvmwraper.cpp
	symbol.c
	table.c
	type.c
	util.c
)

find_package(LLVM REQUIRED CONFIG)

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

find_package(BISON)
find_package(FLEX)


bison_target(
	HelloParser hello-c.y ${CMAKE_CURRENT_BINARY_DIR}/parser.c
	COMPILE_FLAGS -tv
	)

flex_target(HelloScanner hello-c.l ${CMAKE_CURRENT_BINARY_DIR}/lexer.c)

add_flex_bison_dependency(HelloScanner HelloParser)

include_directories(
	${LLVM_INCLUDE_DIRS}
	${CMAKE_CURRENT_BINARY_DIR}
	${CMAKE_SOURCE_DIR}/include
)
add_definitions(${LLVM_DEFINITIONS})

add_executable(
	${EXECUTABLE_NAME}
	${SOURCE_FILE_LIST}
	${BISON_HelloParser_OUTPUTS}
	${FLEX_HelloScanner_OUTPUTS}
	)


# Find the libraries that correspond to the LLVM components
# that we wish to use
llvm_map_components_to_libnames(
	llvm_libs 
	# support 
	analysis
	core
	executionengine
	interpreter
	mcjit
	native
	# irreader
	)

	message(STATUS ${llvm_libs})


# Link against LLVM libraries
target_link_libraries(
	${EXECUTABLE_NAME} 
	${llvm_libs}
	)