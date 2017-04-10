cmake_minimum_required(VERSION 2.7)
project(liborsoqs)

##########################################################################################
# BUILD SETUP
##########################################################################################
option(USE_STATIC "Build with static libraries" OFF)
option(BUILD_RELEASE "Build for release" OFF)
option(BUILD_TEST "Build unit tests" ON)
option(BUILD_SWIG "Build SWIG Python bindings" ON)

##########################################################################################
# SOURCES
##########################################################################################
set(liborsoqs_sources
	error.c
	client.c
	socket.c
	msg.c
	liborsoqs.c)

if(BUILD_TEST)
	set(liborsoqs_test_sources
		test/client.c
		test/socket.c
		test/connection.c
		test/main.c)
endif()

if(BUILD_SWIG)
	# We need to conditionally include some things in our C code
	add_definitions(-DSWIG_PYTHON)
	
	set(liborsoqs_swig_sources
		orsoqs.i)
else()
	# This allows our C code to know the difference between using SWIG with an unknown
	# target language and not using SWIG at all.
	add_definitions(-DNSWIG)
endif()

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")

##########################################################################################
# CONFIGURE BUILD TYPE
##########################################################################################
message("------------------------------")
message("Build Configuration:")

if(CMAKE_COMPILER_IS_GNUCC)
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -Wno-long-long -pedantic -std=c99")
else()
	message(WARNING "Compiler is not GNUCC, using default warning level")
endif()

if(BUILD_RELEASE)
	message("Building for release")
	
	# Release build strips assert statements and automatically enables compiler
	# optimizations.
	
	# This value is stored in the cache!
	set(CMAKE_BUILD_TYPE Release)
	
	if(CMAKE_COMPILER_IS_GNUCC)
		# CMake doesn't automatically strip binaries
		set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -s")
		set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -s")
		set(CMAKE_STATIC_LINKER_FLAGS "${CMAKE_STATIC_LINKER_FLAGS} -s")
	else()
		message(WARNING "Compiler is not GNUCC, binaries not stripped")
	endif()
else()
	message("Building for debug")
	
	# Debug build keeps assert statements and automatically disables any compiler
	# optimizations.
	
	# This value is stored in the cache!
	set(CMAKE_BUILD_TYPE Debug)
endif()

if(BUILD_TEST)
	message("Building unit tests")
	
	find_package(Check REQUIRED)
	include_directories(${CHECK_INCLUDE_DIR})
	
	if(CMAKE_COMPILER_IS_GNUCC)
		set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC")
		message("Compiling with position independent code")
	else()
		message(WARNING "Compiler is not GNUCC, using default warning level")
	endif()
endif()

if(USE_STATIC)
	message("Using static linking")
	message(FATAL_ERROR "static linking disabled")
	
	find_library(CZMQ_LIB czmq.a REQUIRED)
else()
	message("Using shared linking")
	
	find_library(ZMQ_LIB zmq REQUIRED)
endif()

if(BUILD_SWIG)
	message("Building SWIG bindings")
	
	find_package(SWIG REQUIRED)
	include(${SWIG_USE_FILE})
	
	find_package(PythonLibs REQUIRED)
	include_directories(${PYTHON_INCLUDE_PATH})
	
	set(CMAKE_SWIG_FLAGS "")
endif()

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories("${CMAKE_SOURCE_DIR}/../")

message("------------------------------")

##########################################################################################
# BUILD
##########################################################################################
# The core contains code we don't want to recompile for each new target.
add_library(liborsoqs-core STATIC OBJECT ${liborsoqs_sources})

add_library(orsoqs SHARED $<TARGET_OBJECTS:liborsoqs-core>)
target_link_libraries(orsoqs ${ZMQ_LIB})

if(BUILD_SWIG)
	swig_add_module(orsoqs python liborsoqs.i $<TARGET_OBJECTS:liborsoqs-core>)
	swig_link_libraries(orsoqs ${PYTHON_LIBRARIES} ${ZMQ_LIB})
endif()

##########################################################################################
# TEST
##########################################################################################


# Todo: Make testing swig work without installing


if(BUILD_TEST)
	enable_testing()
	add_custom_target(check COMMAND ctest)
	add_executable(liborsoqs-test ${liborsoqs_test_sources})
	target_link_libraries(liborsoqs-test liborsoqs pthread ${PYTHON_LIBRARIES} ${ZMQ_LIB} ${CHECK_LIBRARIES})
	foreach(test ${liborsoqs_test_sources})
		get_filename_component(name ${test} NAME_WE)
		if( NOT "main" STREQUAL ${name} )
			add_test(NAME ${name} WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
					COMMAND ${CMAKE_BINARY_DIR}/liborsoqs-test ${name})
		endif()
	endforeach(test)
	if(BUILD_SWIG)
		add_test(NAME swig WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
				COMMAND python ${CMAKE_CURRENT_SOURCE_DIR}/test/swig.py)
	endif()
endif()


##########################################################################################
# INSTALL
##########################################################################################
install(TARGETS orsoqs DESTINATION lib)

if(BUILD_SWIG)
	execute_process(COMMAND python -c "from distutils.sysconfig import get_python_lib; print get_python_lib()" OUTPUT_VARIABLE PYTHON_SITE_PACKAGES OUTPUT_STRIP_TRAILING_WHITESPACE)
	install(TARGETS _orsoqs DESTINATION ${PYTHON_SITE_PACKAGES})
	install(FILES ${CMAKE_BINARY_DIR}/orsoqs.py DESTINATION ${PYTHON_SITE_PACKAGES})
endif()

##########################################################################################
# UNINSTALL
##########################################################################################
add_custom_target(uninstall COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/uninstall.cmake)
