# CMake 最低版本号要求
cmake_minimum_required (VERSION 3.2)
# 项目信息
project (lasviewer)

#--------- QT ----------------
find_package(Qt5 REQUIRED Core Gui Widgets OpenGL Charts)

#SET(QT_USE_QTOPENGL TRUE)
link_libraries(Qt5::Core Qt5::Gui Qt5::Widgets Qt5::OpenGL Qt5::Charts)

# WinMain入口, 不需要console 
SET(QT_USE_QTMAIN TRUE) 
#----------- QT end  --------------------

#-------- OSG ----------------
find_path(OSG_INCLUDE osg/Referenced
    PATHS
    $ENV{OSGHOME}/include
    /usr/include
    /usr/local/include
)

find_path(OSG_LIBDIR osg.lib
    PATHS
    $ENV{OSGHOME}/lib
    /usr/lib
    /usr/local/lib
)

include_directories (${OSG_INCLUDE})
link_directories(${OSG_LIBDIR})

SET(OSG_DEBUG_POSTFIX "d" CACHE STRING "add a postfix, usually d on windows")
SET(OSG_RELEASE_POSTFIX "" CACHE STRING "add a postfix, usually empty on windows")

SET(OSG_LIB_MODULES 
   osg osgDB osgGA osgText osgUtil osgViewer OpenThreads
)
#    osg 
#    OpenThreads  osgAnimation osgFx osgDB osgGA 
#    osgManipulator osgParticle osgQt5 osgShadow osgSim 
#    osgTerrain osgText osgUtil osgViewer osgVolume osgWidget

SET(OSG_LIBS "")
foreach(m ${OSG_LIB_MODULES})
    link_libraries(debug ${m}${OSG_DEBUG_POSTFIX}  optimized ${m}${OSG_RELEASE_POSTFIX})
endforeach(m)
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
#SET(GUI_TYPE WIN32)

add_executable(${PROJECT_NAME} ${GUI_TYPE} ${Headers} ${Sources} ${UISrcs} ${MocSrcs})
