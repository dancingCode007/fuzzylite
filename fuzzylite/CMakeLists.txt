cmake_minimum_required(VERSION 2.8)

project(fuzzylite CXX)


if (APPLE)
	cmake_policy(SET CMP0042 NEW)
endif()
if (MSVC)
	cmake_policy(SET CMP0054 NEW)
endif()


###DEFINES SECTION
if(NOT CMAKE_VERBOSE_MAKEFILE)
    set(CMAKE_VERBOSE_MAKEFILE false)
endif()

if( NOT CMAKE_BUILD_TYPE )
    set( CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build, options are: None Debug Release RelWithDebInfo
MinSizeRel." FORCE )
endif()

set(FL_VERSION 5.1)
add_definitions(-DFL_VERSION="${FL_VERSION}")

set(FL_DATE "1507")
add_definitions(-DFL_DATE="${FL_DATE}")

add_definitions(-DFL_BUILD_PATH="${CMAKE_SOURCE_DIR}") #used to determine FL__FILE__

option(FL_BUILD_SHARED "Build shared library" ON)
option(FL_BUILD_STATIC "Build static library" ON)
if(FL_BUILD_SHARED)
    option(FL_BUILD_BINARY "Build fuzzylite binary" ON)
endif()

option(FL_USE_FLOAT "Use fl::scalar as float" OFF)
option(FL_BACKTRACE "Provide backtrace information in case of errors" OFF)
option(FL_CPP11 "Builds utilizing C++11, i.e., passing -std=c++11" ON)

if(FL_USE_FLOAT)
    add_definitions(-DFL_USE_FLOAT)
endif(FL_USE_FLOAT)

if(NOT FL_BACKTRACE)
    add_definitions(-DFL_BACKTRACE_OFF)
endif()

if (MSVC AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS 19)
#C++11 not available before Visual Studio 2015
    if (FL_CPP11)
        set(FL_CPP11 OFF)
    endif()
endif()

if(FL_CPP11)
    add_definitions(-DFL_CPP11)
    if(NOT MSVC)
        add_definitions(-std=c++11)
    endif(NOT MSVC)
endif(FL_CPP11)

if(NOT FL_CPP11 AND NOT MSVC)
    add_definitions(-std=c++98)
endif()

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY bin)

if(NOT MSVC)
    set(CMAKE_CXX_FLAGS "-pedantic -Wall -Wextra ${CMAKE_CXX_FLAGS}")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE}")

    if(NOT APPLE)
        set(CMAKE_SHARED_LINKER_FLAGS "-Wl,--no-undefined ${CMAKE_SHARED_LINKER_FLAGS}") #To avoid undefined methods in library
    endif()
endif()

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-non-literal-null-conversion")
endif()

if ("${CMAKE_CXX_COMPILER_ID}"  STREQUAL "GNU")
    if (CMAKE_CXX_COMPILER_VERSION VERSION_EQUAL 6)
        if (NOT FL_CPP11)
            #set the default to C++98
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++98")
        endif()
    endif()
endif()


set(FL_LIBS)

if(MSVC)
    set(CMAKE_CXX_FLAGS "/W4 /EHsc")
    #Wx: Treat warnings as errors. W4: All warnings
    #http://msdn.microsoft.com/en-us/library/thxezb7y.aspx
    #EHsc: call destructors on __try __catch, and to ignore C4530: C++ exception handler used. Note, unwind semantics are not enabled
    if (FL_BACKTRACE)
        set(FL_LIBS dbghelp)
    endif()
endif()

if(APPLE)
    #Fix ld: symbol(s) not found for architecture x86_64 on mac
    set(FL_LIBS stdc++)
endif()


###BUILD SECTION

include_directories(.)

file(STRINGS FL_HEADERS fl-headers)
file(STRINGS FL_SOURCES fl-sources)

string(REGEX REPLACE "\n" " " ${fl-headers} ${fl-headers})
string(REGEX REPLACE "\n" " " ${fl-sources} ${fl-sources})

message("${exepath}")


set(CMAKE_DEBUG_POSTFIX d)

if (MSVC OR CMAKE_GENERATOR STREQUAL Xcode)
    if(FL_BUILD_SHARED)
        add_library(fl-shared SHARED ${fl-headers} ${fl-sources})
    endif()

    if(FL_BUILD_STATIC)
        add_library(fl-static STATIC ${fl-headers} ${fl-sources})
    endif()
else()
    if(FL_BUILD_SHARED OR FL_BUILD_STATIC)
        add_library(fl-obj OBJECT ${fl-headers} ${fl-sources})
        if(NOT MINGW)
            set_target_properties(fl-obj PROPERTIES COMPILE_FLAGS "-fPIC")
        endif()
    endif()

    if(FL_BUILD_SHARED)
        add_library(fl-shared SHARED $<TARGET_OBJECTS:fl-obj>)
    endif(FL_BUILD_SHARED)

    if(FL_BUILD_STATIC)
        add_library(fl-static STATIC $<TARGET_OBJECTS:fl-obj>)
    endif(FL_BUILD_STATIC)
endif()

if(FL_BUILD_SHARED)
    set_target_properties(fl-shared PROPERTIES OUTPUT_NAME fuzzylite)
    set_target_properties(fl-shared PROPERTIES DEBUG_POSTFIX d)
    set_target_properties(fl-shared PROPERTIES COMPILE_DEFINITIONS "FL_EXPORT_LIBRARY")
    set_target_properties(fl-shared PROPERTIES VERSION 5.1)
    target_link_libraries(fl-shared ${FL_LIBS})
endif()

if(FL_BUILD_STATIC)
    set_target_properties(fl-static PROPERTIES OUTPUT_NAME fuzzylite-static)
    set_target_properties(fl-static PROPERTIES DEBUG_POSTFIX d)
    set_target_properties(fl-static PROPERTIES VERSION 5.1)
    target_link_libraries(fl-static ${FL_LIBS})
endif()

if(FL_BUILD_BINARY)
    add_executable(fl-bin src/main.cpp)
    set_target_properties(fl-bin PROPERTIES OUTPUT_NAME fuzzylite)
    set_target_properties(fl-bin PROPERTIES OUTPUT_NAME fuzzylite IMPORT_PREFIX tmp-) #To prevent LNK1149 in Windows
    set_target_properties(fl-bin PROPERTIES DEBUG_POSTFIX d)

    set_target_properties(fl-bin PROPERTIES COMPILE_DEFINITIONS "FL_IMPORT_LIBRARY")
    target_link_libraries(fl-bin fl-shared ${FL_LIBS})
endif(FL_BUILD_BINARY)

###INSTALL SECTION
if(NOT FL_INSTALL_BINDIR)
    set(FL_INSTALL_BINDIR bin)
endif()

if(NOT FL_INSTALL_LIBDIR)
    if(NOT CMAKE_INSTALL_LIBDIR)
        set(FL_INSTALL_LIBDIR lib)
    else()
        set(FL_INSTALL_LIBDIR ${CMAKE_INSTALL_LIBDIR})
    endif()
endif()

if(FL_BUILD_BINARY)
    install(TARGETS fl-bin 
            RUNTIME DESTINATION ${FL_INSTALL_BINDIR}
            LIBRARY DESTINATION ${FL_INSTALL_LIBDIR}
            ARCHIVE DESTINATION ${FL_INSTALL_LIBDIR}
    )
endif()

if(FL_BUILD_SHARED)
    install(TARGETS fl-shared 
            RUNTIME DESTINATION ${FL_INSTALL_BINDIR}
            LIBRARY DESTINATION ${FL_INSTALL_LIBDIR}
            ARCHIVE DESTINATION ${FL_INSTALL_LIBDIR}
    )
endif()

if(FL_BUILD_STATIC)
    install(TARGETS fl-static
            RUNTIME DESTINATION ${FL_INSTALL_BINDIR}
            LIBRARY DESTINATION ${FL_INSTALL_LIBDIR}
            ARCHIVE DESTINATION ${FL_INSTALL_LIBDIR}
    )
endif()

install(DIRECTORY fl/ DESTINATION include/fl)

#pkg-config
configure_file(${CMAKE_SOURCE_DIR}/fuzzylite.pc.in ${CMAKE_BINARY_DIR}/fuzzylite.pc @ONLY)
install(FILES ${CMAKE_BINARY_DIR}/fuzzylite.pc DESTINATION ${FL_INSTALL_LIBDIR}/pkgconfig)

message("=====================================")
message("fuzzylite v${FL_VERSION}b${FL_DATE}\n")
message("FL_CPP11=${FL_CPP11}")
message("FL_USE_FLOAT=${FL_USE_FLOAT}")
message("FL_BACKTRACE=${FL_BACKTRACE}")
message("FL_LIBS=${FL_LIBS}")
message("FL_INSTALL_BINDIR=${FL_INSTALL_BINDIR}")
message("FL_INSTALL_LIBDIR=${FL_INSTALL_LIBDIR}")
message("FL_BUILD_TESTS=${FL_BUILD_TESTS}")
message("")
message("CMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}")
message("CMAKE_CXX_COMPILER_ID=${CMAKE_CXX_COMPILER_ID}")
message("CMAKE_CXX_COMPILER_VERSION=${CMAKE_CXX_COMPILER_VERSION}")
message("CMAKE_CXX_FLAGS=${CMAKE_CXX_FLAGS}")
message("COMPILE_DEFINITIONS:")
get_directory_property(fl-definitions DIRECTORY ${CMAKE_SOURCE_DIR} COMPILE_DEFINITIONS )
foreach(d ${fl-definitions})
    message( STATUS "Defined: " ${d} )
endforeach()

message("=====================================\n")

###UNINSTALL SECTION
#configure_file(
    #"${CMAKE_CURRENT_SOURCE_DIR}/cmake_uninstall.cmake.in"
    #"${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
    #IMMEDIATE @ONLY)

#add_custom_target(uninstall
    #COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/uninstall.cmake)

#unix uninstall
#xargs rm < install_manifest.txt
