cmake_minimum_required(VERSION 3.8)
project(Hair_generate_convdata_and_imgs)

set(CMAKE_CXX_STANDARD 11)

file(GLOB Hair_Code hair/*.h hair/*.cpp)

set(SOURCE_FILES main.cpp Renderer.h SimpleVertexShader.vertexshader SimpleFragmentShader.fragmentshader
        TrainingDataGenerator.h ${Hair_Code} HairHelper.h hair/cnpy.h hair/cnpy.cpp)

find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(glfw3 REQUIRED)

find_package( OpenCV REQUIRED )

#find_package(PCL 1.3 REQUIRED COMPONENTS common io)

# Ask the user to give the paths to GLFW, GLEW and GLM.
# On Linux this can be found automatically with pkg-config;
# see http://stackoverflow.com/questions/6889264/cmake-flags-for-opengl-using-glew-and-glfw, for instance.

#set(GLFW_INCLUDE_PATH "" CACHE PATH "The directory that contains GL/glfw.h" )
#set(GLFW_LIBRARY "" CACHE FILEPATH "libglfw.a or glfw.lib" )

#set(GLEW_INCLUDE_PATH "" CACHE PATH "The directory that contains GL/glew.h" )
#set(GLEW_LIBRARY "" CACHE FILEPATH "libglew.a or glew.lib" )

#set(GLM_INCLUDE_PATH "" CACHE PATH "The directory that contains glm/glm.hpp" )

include_directories(
        ${GLFW3_INCLUDE_PATH}
        ${GLEW_INCLUDE_PATH}
        ${GLM_INCLUDE_PATH}
        ${OPENGL_INCLUDE_DIR}
        #        ${PCL_INCLUDE_DIRS}
        /home/yi/OpenMesh-6.3/src
)

#link_directories(${PCL_LIBRARY_DIRS})
#add_definitions(${PCL_DEFINITIONS})


add_executable(Hair_generate_convdata_and_imgs ${SOURCE_FILES} )

target_link_libraries(Hair_generate_convdata_and_imgs
        ${OPENGL_LIBRARY} # filled by "find_package(OpenGL REQUIRED)"
        ${GLFW3_LIBRARY}
        ${GLEW_LIBRARY}
        /home/yi/OpenMesh-6.3/build/Build/lib/libOpenMeshCore.so
        /home/yi/OpenMesh-6.3/build/Build/lib/libOpenMeshTools.so
        ${OpenCV_LIBS}
        #        ${PCL_COMMON_LIBRARIES}
        #        ${PCL_IO_LIBRARIES}
        )
