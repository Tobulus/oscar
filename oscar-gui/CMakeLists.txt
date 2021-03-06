project(oscar-gui)
cmake_minimum_required(VERSION 2.8)
include_directories(${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_SOURCE_DIR})
find_package(Qt4)

IF (QT4_FOUND)

include_directories(${QT_INCLUDES} ${CMAKE_CURRENT_BINARY_DIR})

set(SOURCES_MOC_H
OscarQtMainWindow.h
ItemIndexTableModel.h
CompletionDispatcher.h
SubSetModel.h
SubSetNodeFlattener.h
MarbleMap.h)

set(SOURCES_CPP
SubSetModel.cpp
CompletionDispatcher.cpp
ItemIndexTableModel.cpp
OscarQtMainWindow.cpp
MarbleMap.cpp
SubSetNodeFlattener.cpp
main.cpp)

qt4_wrap_cpp(SOURCES_MOC_CPP ${SOURCES_MOC_H})


SET(OSCAR_LINK_LIBS
${liboscar_BINARY_DIR}/libliboscar.so
${OSCAR_LINK_LIBS}
${QT_QTCORE_LIBRARY}
${QT_QTGUI_LIBRARY}
"marblewidget"
)

add_executable(${PROJECT_NAME} ${SOURCES_CPP} ${SOURCES_MOC_CPP})
add_dependencies(${PROJECT_NAME} liboscar sserialize)
target_link_libraries(${PROJECT_NAME} ${OSCAR_LINK_LIBS})


ENDIF (QT4_FOUND)
