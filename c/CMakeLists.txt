cmake_minimum_required(VERSION 3.10)

project(tstorage-client
    VERSION 1.0.0
    LANGUAGES C
    DESCRIPTION "Interface to the TStorage database"
)

option(BUILD_EXAMPLE "Build the example" ON)
option(BUILD_SHARED_LIBS "Build shared instead of static libraries" OFF)

include(GNUInstallDirs)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED True)

set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -s")

add_library(tstorage-client
    src/BufferedIStream.c
    src/BufferedOStream.c
    src/Channel.c
    src/DynamicArray.c
    src/DynamicBuffer.c
    src/Key.c
    src/KeyRange.c
    src/Record.c
    src/RecordsSet.c
    src/RequestGet.c
    src/RequestHeader.c
    src/RequestPut.c
    src/ResponseGet.c
    src/ResponseGetConfirmation.c
    src/ResponseGetWithCallback.c
    src/ResponseHeader.c
    src/ResponsePut.c
    src/Socket.c
    src/timestamp.c
)

if(BUILD_EXAMPLE)
    add_subdirectory("example")
endif()

set_target_properties(
	tstorage-client PROPERTIES
	VERSION "${PROJECT_VERSION}"
	SOVERSION "${PROJECT_VERSION_MAJOR}"
	PUBLIC_HEADER include/tstorage-client/client.h
	C_VISIBILITY_PRESET hidden
)

target_include_directories(tstorage-client PRIVATE include)
target_include_directories(tstorage-client PRIVATE src)

target_compile_definitions(tstorage-client PRIVATE TSTORAGE_CLIENT_BUILD)

install(TARGETS tstorage-client
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/tstorage-client
)
