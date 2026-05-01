cmake_minimum_required(VERSION 3.23)
project(rla2 VERSION "1.0.0" LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
add_executable(${PROJECT_NAME} src/RLA_CL.cc src/MainRLA.cc src/cluster.cc
        src/Common.h src/performance.cc
        src/SortMSD.cc src/SortInsertion.cc
        src/util.cc src/util.h
        src/multi_attribute_blocking.cpp src/multi_attribute_blocking.h
        src/edit_distance.cpp src/edit_distance.h
        src/prefix_blocking.cpp src/prefix_blocking.h
        src/multi_blocking_scheme.cpp src/multi_blocking_scheme.h
        src/union_find.cpp src/union_find.h
        src/soundex.cpp src/soundex.h
        src/soundex_blocking.cpp src/soundex_blocking.h
        src/double_metaphone.cc src/double_metaphone.h
        src/doublemetaphone_blocking.cpp src/doublemetaphone_blocking.h
)
find_package(LibXml2 REQUIRED)
include_directories(${LIBXML2_INCLUDE_DIR})
find_package(Boost COMPONENTS log log_setup thread filesystem program_options regex REQUIRED)
include_directories(${boost_INCLUDE_DIR})
find_package(yaml-cpp REQUIRED)
find_library(YAML_LIB yaml-cpp)
include_directories(${YAML_CPP_INCLUDE_DIR})
target_link_libraries(${PROJECT_NAME} ${LIBXML2_LIBRARIES} ${Boost_FILESYSTEM_LIBRARY_RELEASE}
        ${Boost_LOG_LIBRARY_RELEASE} ${Boost_LOG_SETUP_LIBRARY_RELEASE}
        ${Boost_FILESYSTEM_LIBRARY_RELEASE} ${Boost_PROGRAM_OPTIONS_LIBRARY_RELEASE}
        ${Boost_THREAD_LIBRARY_RELEASE}
        ${Boost_REGEX_LIBRARY_RELEASE}
        ${YAML_LIB}
)
set(CMAKE_CXX_FLAGS_DEBUG "-g -DBOOST_LOG_DYN_LINK")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DBOOST_LOG_DYN_LINK")
install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION /usr/local/bin)

add_executable(test_edit_distance src/test_edit_distance.cpp src/edit_distance.cpp)

add_executable(test_soundex tests/test_soundex.cpp src/soundex.cpp src/util.cc)
set(CMAKE_CXX_FLAGS_DEBUG "-g -DBOOST_LOG_DYN_LINK")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DBOOST_LOG_DYN_LINK")
target_link_libraries(test_soundex ${Boost_FILESYSTEM_LIBRARY_RELEASE}
        ${Boost_LOG_LIBRARY_RELEASE} ${Boost_LOG_SETUP_LIBRARY_RELEASE}
        ${Boost_FILESYSTEM_LIBRARY_RELEASE} ${Boost_PROGRAM_OPTIONS_LIBRARY_RELEASE}
        ${Boost_THREAD_LIBRARY_RELEASE})
