cmake_minimum_required(VERSION 2.8)
project(oscar-cmd)

find_package(LIBRT REQUIRED)

include_directories(${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_SOURCE_DIR})

set(OSCAR_SOURCES_CPP
oscarmain.cpp
Workfunctions.cpp
Config.cpp
Benchmark.cpp
LiveCompleter.cpp
GeoHierarchyPrinter.cpp
ConsistencyCheckers.cpp
)

SET(OSCAR_LINK_LIBS
	${liboscar_BINARY_DIR}/libliboscar.so
	${sserialize_BINARY_DIR}/libsserialize.a
	${LIBRT_LIBRARIES}
)

add_executable(${PROJECT_NAME} ${OSCAR_SOURCES_CPP})
add_dependencies(${PROJECT_NAME} liboscar sserialize)
target_link_libraries(${PROJECT_NAME} ${OSCAR_LINK_LIBS})

