cmake_minimum_required (VERSION 3.4)
project(AsynchronousCache CXX)

find_package(Doxygen)
if(DOXYGEN_FOUND)
    add_custom_target(doxygen ALL
        ${DOXYGEN_EXECUTABLE}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMENT "Generating API documentation with Doxygen" VERBATIM
        SOURCES Doxyfile
    )
endif(DOXYGEN_FOUND)

add_definitions(
    -DNOMINMAX
    -DWIN32_LEAN_AND_MEAN
    -DVC_EXTRALEAN
    -D_CRT_SECURE_NO_WARNINGS
    -D_SECURE_SCL=0
    -D_SCL_SECURE_NO_WARNINGS
)

option(BUILD_SHARED_LIBS "Build libraries as DLLs" FALSE)

#set(AsynchronousCache_VERSION_MAJOR 0)
#set(AsynchronousCache_VERSION_MINOR 1)
#configure_file("${PROJECT_SOURCE_DIR}/Version.h.in" "${PROJECT_BINARY_DIR}/Version.h")

#set(JSON_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/json/src)
#
#add_subdirectory(unit-test)         # VS unit test library

set(PUBLIC_INCLUDE_PATHS
    include/AsynchronousCache
)

set(PRIVATE_INCLUDE_PATHS
)

#set(SOURCES
#    main.cpp
#)
#
#add_executable(main ${SOURCES})
##set_target_properties(main PROPERTIES OUTPUT_NAME main)
##target_compile_definitions(main PRIVATE ${DEFINES})
#target_include_directories(main PUBLIC ${PUBLIC_INCLUDE_PATHS} PRIVATE ${PRIVATE_INCLUDE_PATHS})
#target_link_libraries(main AsynchronousCache)
#source_group(Sources FILES ${SOURCES})
