# CMake 最低版本号要求
cmake_minimum_required (VERSION 3.2)
# 项目信息
project (lasviewer)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

#--------- QT ----------------
set(CMAKE_PREFIX_PATH $ENV{QTDIR})
find_package(Qt5 REQUIRED Core Gui Widgets OpenGL Charts)

#SET(QT_USE_QTOPENGL TRUE)
link_libraries(Qt5::Core Qt5::Gui Qt5::Widgets Qt5::OpenGL Qt5::Charts)

# WinMain入口, 不需要console 
SET(QT_USE_QTMAIN TRUE) 
#----------- QT end  --------------------

#-------- OSG ----------------
set(CMAKE_PREFIX_PATH $ENV{OSG_DIR})
#find_package(OpenSceneGraph 3.0.0 REQUIRED 
#	osgDB osgGA osgUtil osgText osgViewer)
find_package(OpenSceneGraph 3.0.0 REQUIRED 
	 osgGA  osgViewer)
include_directories(${OSG_INCLUDE_DIR})
link_libraries(${OPENSCENEGRAPH_LIBRARIES})

message(STATUS "OPENSCENEGRAPH_LIBRARIES: ${OPENSCENEGRAPH_LIBRARIES}")

#--------  OSG end ---------------

file(GLOB Sources src/*.cpp)
file(GLOB Headers include/*.h)
file(GLOB Forms ui/*.ui)

QT5_WRAP_UI(UISrcs ${Forms})
QT5_WRAP_CPP(MocSrcs ${Headers})
include_directories (${PROJECT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ./include)

source_group("Header Files" FILES ${Headers}) 
source_group("Generator Files" FILES ${UISrcs} ${MocSrcs}) 

#SET(GUI_TYPE $<$<CONFIG:Release>:WIN32> $<$<CONFIG:Debug>:>)
IF (CMAKE_BUILD_TYPE MATCHES "Debug")
	SET(GUI_TYPE)
ELSE()
	SET(GUI_TYPE WIN32)
ENDIF()

add_executable(${PROJECT_NAME} ${GUI_TYPE} ${Headers} ${Sources} ${UISrcs} ${MocSrcs})
