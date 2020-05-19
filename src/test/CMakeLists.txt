cmake_minimum_required(VERSION 2.8.2)

include_directories(.)

set(SRC_FILES_CUR 
	Test_imp.cpp
	)

add_library(Test ${SRC_FILES})
