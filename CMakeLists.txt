cmake_minimum_required(VERSION 3.16)
project(Whacky)

set(CMAKE_CXX_STANDARD 20)

if(MSVC) 
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Release>:Release>")
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
	add_compile_options(/arch:AVX2) #make sure SIMD optimizations take place
endif()

set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE) #link time optimization


file(GLOB_RECURSE MY_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")

add_executable(${CMAKE_PROJECT_NAME} ${MY_SOURCES})


#DELETE THE OUT FOLDER AFTER CHANGING THIS BECAUSE VISUAL STUDIO DOESN'T SEEM TO RECOGNIZE THIS CHANGE AND REBUILD!
option(PRODUCTION_BUILD "Make this a production build!" OFF)
#DELETE THE OUT FOLDER AFTER CHANGING THIS BECAUSE VISUAL STUDIO DOESN'T SEEM TO RECOGNIZE THIS CHANGE AND REBUILD!

if(PRODUCTION_BUILD)
    target_compile_definitions(${CMAKE_PROJECT_NAME} PUBLIC
        RESOURCES_PATH="./resources/"
        PRODUCTION_BUILD=1
    )
else()
    target_compile_definitions(${CMAKE_PROJECT_NAME} PUBLIC
        RESOURCES_PATH="${CMAKE_CURRENT_SOURCE_DIR}/resources/"
        PRODUCTION_BUILD=0
    )
endif()

target_include_directories(${CMAKE_PROJECT_NAME} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include/")