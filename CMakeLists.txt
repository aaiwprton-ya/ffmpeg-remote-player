cmake_minimum_required(VERSION 3.0)

# project properties
set(PROJECT ffmpeg-remote-player)
project(${PROJECT} VERSION 1.0.0 LANGUAGES CXX)

# compiler and linker flags
set(GCC_COVERAGE_COMPILE_FLAGS )
set(GCC_COVERAGE_LINK_FLAGS )

# user pathes
set(LOCAL_BUILDS /home/aaiwprton/.local_builds)

set(OPENCV_INCLUDES ${LOCAL_BUILDS}/opencv/include/opencv4)
set(OPENCV_LIBS ${LOCAL_BUILDS}/opencv/lib)

set(FFMPEG_INCLUDES /${LOCAL_BUILDS}/ffmpeg/include)
set(FFMPEG_LIBS ${LOCAL_BUILDS}/ffmpeg/lib)

set(LOCAL_INCLUDES ../include)
set(LOCAL_LIBS ../lib)

# finding packages
# --

# dependencis
set(SHARED_INCLUDE_LIST ${OPENCV_INCLUDES} ${FFMPEG_INCLUDES})
set(SHARED_LINK_LIST ${OPENCV_LIBS} ${FFMPEG_LIBS})
set(SHARED_LIBS 
    -lopencv_core
    -lopencv_imgproc
    -lopencv_imgcodecs
    -lopencv_highgui
    -lopencv_objdetect
    -lopencv_videoio
    -lavutil
	-lavformat
	-lavcodec
    -lavfilter
    -lavdevice
    -lswresample
    -lswscale)
set(SHARED_LIBS_CLIENT
    -lopencv_core
    -lopencv_imgproc
    -lopencv_imgcodecs
    -lopencv_highgui
    -lopencv_objdetect
    -lopencv_videoio)
set(SHARED_LIBS_SERVER
    -lopencv_core
    -lopencv_imgproc
    -lopencv_imgcodecs
    -lopencv_highgui
    -lopencv_objdetect
    -lopencv_videoio)

set(STATIC_INCLUDE_LIST ${LOCAL_INCLUDES})
set(STATIC_LINK_LIST ${LOCAL_LIBS})
set(STATIC_LIBS
    argparser
    inet_module
    serializer)
set(STATIC_LIBS_CLIENT
    argparser
    inet_module)
set(STATIC_LIBS_SERVER
    argparser
    inet_module)

# sources
set(SOURCES main.cpp remote_player.cpp)
set(HEADERS remote_player.h)

set(SOURCES_CLIENT client.cpp)
set(HEADERS_CLIENT)

set(SOURCES_SERVER server.cpp)
set(HEADERS_SERVER)

# init functions
function(find_static_lib lib_dir lib_name)
	if(EXISTS "${lib_dir}/${lib_name}.a")
		add_library(${lib_name} STATIC IMPORTED)
		set_property(TARGET ${lib_name} PROPERTY
				IMPORTED_LOCATION ${lib_dir}/${lib_name}.a)
	endif()
endfunction()

function(find_static_libs lib_dir lib_names)
	foreach(name ${${lib_names}})
		find_static_lib(${lib_dir} ${name})
	endforeach()
endfunction()

# setup pathes
foreach(include_dir ${SHARED_INCLUDE_LIST})
	include_directories(${include_dir})
endforeach()

foreach(link_dir ${SHARED_LINK_LIST})
	link_directories(${link_dir})
endforeach()

foreach(include_dir ${STATIC_INCLUDE_LIST})
	include_directories(${include_dir})
endforeach()

foreach(link_dir ${STATIC_LINK_LIST})
	find_static_libs(${link_dir} STATIC_LIBS)
endforeach()

# setup compiler and linker flags
set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} ${GCC_COVERAGE_COMPILE_FLAGS}")
set(CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} ${GCC_COVERAGE_LINK_FLAGS}")
 
# build
add_executable(${PROJECT}.out ${SOURCES} ${HEADERS})
add_executable(${PROJECT}-client.out ${SOURCES_CLIENT} ${HEADERS_CLIENT})
add_executable(${PROJECT}-server.out ${SOURCES_SERVER} ${HEADERS_SERVER})

target_link_libraries(
	${PROJECT}.out 
	${STATIC_LIBS}
	${SHARED_LIBS})

target_link_libraries(
	${PROJECT}-client.out 
	${STATIC_LIBS_CLIENT}
	${SHARED_LIBS_CLIENT})

target_link_libraries(
	${PROJECT}-server.out 
	${STATIC_LIBS_SERVER}
	${SHARED_LIBS_SERVER})

# testing
#enable_testing()

