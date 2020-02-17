cmake_minimum_required(VERSION 2.8.2)

project(Bee)

set(INC_DIR net package recover service log)
include_directories(.)

set(SRC_FILES 
	net/UDPSender.cpp
	net/UDPReceiver.cpp
	package/Package.cpp
	recover/RecoverManager.cpp
	service/Service.cpp
	service/PackageControl.cpp
	)

add_library(Bee ${SRC_FILES})


add_executable(Demo Demo.cpp)

set(LIBS pthread)

target_link_libraries(Demo Bee)
target_link_libraries(Demo ${LIBS})

add_definitions("-g")
