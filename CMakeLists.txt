cmake_minimum_required(VERSION 2.8.11)

# Name of project
project(ACME)

# Dependencies SeqAn - tested version 2.3.2
find_package(SeqAn REQUIRED)
find_package(Boost REQUIRED)

# Add include directories
include_directories (${SEQAN_INCLUDE_DIRS} ${Boost_INCLUDE_DIR} include)

# Add definitions
add_definitions (${SEQAN_DEFINITIONS} ${Boost_LIB_DIAGNOSTIC_DEFINITIONS})

# Set header files
set(HEADER_FILES src/cast.h src/acme.h src/occurrenceelement.h src/branchelement.h)

# Set source files
set(SOURCE_FILES src/cast.cpp src/acme.cpp src/main.cpp)

# Set c++ flags
SET ( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${SEQAN_CXX_FLAGS} -W -Wall -Wstrict-aliasing -pedantic -Wno-long-long -Wno-variadic-macros" )

# Add executable
add_executable(ACME ${SOURCE_FILES} ${HEADER_FILES})

# Link against SeqAn dependencies
target_link_libraries (ACME ${SEQAN_LIBRARIES} ${Boost_LIBRARIES})
