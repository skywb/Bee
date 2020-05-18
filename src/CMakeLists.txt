cmake_minimum_required(VERSION 2.8.2)

project(Bee)

MESSAGE(STATUS "operation system is ${CMAKE_SYSTEM}")

IF (CMAKE_SYSTEM_NAME MATCHES "Linux")
	MESSAGE(STATUS "current platform: Linux ")
	add_definitions("-DLINUX")
	add_definitions("-DUNIX")
ELSEIF (CMAKE_SYSTEM_NAME MATCHES "Windows")
	MESSAGE(STATUS "current platform: Windows")
	add_definitions("-DWIN")
	add_definitions("-DGLOG_NO_ABBREVIATED_SEVERITIES")
ELSEIF (CMAKE_SYSTEM_NAME MATCHES "FreeBSD")
	MESSAGE(STATUS "current platform: FreeBSD")
	add_definitions("-DFREEBSD")
ELSE ()
	MESSAGE(STATUS "other platform: ${CMAKE_SYSTEM_NAME}")
	add_definitions("-DUNIX")
ENDIF (CMAKE_SYSTEM_NAME MATCHES "Linux")

add_definitions("-std=c++14")
#add_definitions("-g")

set(CMAKE_BUILD_TYPE Release CACHE STRING "set build type to release")

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup()

set(INC_DIR net package recover service log client)
include_directories(.)
set(SRC_FILES 
	client/Bee.cpp
	client/Connecter.cpp
	net/UDPSender.cpp
	net/UDPReceiver.cpp
	package/Package.cpp
	recover/RecoverManager.cpp
	service/Service.cpp
	service/PackageControl.cpp
	)

add_library(Bee ${SRC_FILES})
add_executable(Demo Demo.cpp)
#add_executable(SockSpeedTest test/test.cpp)
add_executable(MaxSpeedTest test/MaxSpeedTest.cpp)

set(LIBS  Bee ${CONAN_LIBS})

IF (UNIX) 
target_link_libraries(Demo pthread)
ENDIF ()

target_link_libraries(Demo ${LIBS})
#target_link_libraries(SockSpeedTest ${LIBS})
target_link_libraries(MaxSpeedTest ${LIBS})
#target_link_libraries(Demo ${CONAN_LIBS})

