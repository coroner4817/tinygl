# CMake entry point
cmake_minimum_required (VERSION 3.1)
project (Tutorials)

# set(CMAKE_VERBOSE_MAKEFILE ON)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
# set(CMAKE_CXX_FLAGS "-Wall -Wextra")

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug)
endif()

if(CMAKE_BUILD_TYPE MATCHES Debug)
  message(STATUS "Build debug mode")
  set(CMAKE_CXX_FLAGS_DEBUG "-g")
endif()

if(CMAKE_BUILD_TYPE MATCHES Release)
  message(STATUS "Build release mode")
  set(CMAKE_CXX_FLAGS_RELEASE "-O3")
endif()

if( CMAKE_BINARY_DIR STREQUAL CMAKE_SOURCE_DIR )
    message( FATAL_ERROR "Please select another Build Directory ! (and give it a clever name, like bin_Visual2012_64bits/)" )
endif()
if( CMAKE_SOURCE_DIR MATCHES " " )
	message( "Your Source Directory contains spaces. If you experience problems when compiling, this can be the cause." )
endif()
if( CMAKE_BINARY_DIR MATCHES " " )
	message( "Your Build Directory contains spaces. If you experience problems when compiling, this can be the cause." )
endif()

if(NO_EXEC_TARGET)
  message(STATUS "Not execute the target")
endif()

##############################################
find_package(OpenGL REQUIRED)
find_package(glm REQUIRED)

find_package(PkgConfig REQUIRED)
pkg_search_module(GLFW REQUIRED glfw3)

# Compile external dependencies
add_subdirectory (external)
add_subdirectory (external/glad-ogl-4.5.0)

include_directories(
  ${OPENGL_INCLUDE_DIRS}
  ${GLM_INCLUDE_DIRS}
  external/AntTweakBar-1.16/include/
  external/glad-ogl-4.5.0/include/
  external/glew-1.13.0/include/
  external/assimp-3.0.1270/include/
  external/bullet-2.81-rev2613/src/
  external/freetype-2.10.1/include/
  /usr/include/eigen3
  .
)

set(ALL_LIBS
  ${OPENGL_LIBRARY}
  ${GLFW_LIBRARIES}
  GLEW_1130
  glad
)

MESSAGE(STATUS "-----: " ${GLFW_LIBRARIES})
MESSAGE(STATUS "-----: " ${GLM_INCLUDE_DIRS})

set(MODEL_LIBS
  assimp
)

find_package(ZLIB)
set(FONT_LIBS
  ${CMAKE_SOURCE_DIR}/external/freetype-2.10.1/libfreetype.a
  ${ZLIB_LIBRARIES}
)

add_definitions(
  -DTW_STATIC
  -DTW_NO_LIB_PRAGMA
  -DTW_NO_DIRECT3D
  -DGLEW_STATIC
  -D_CRT_SECURE_NO_WARNINGS
  # -DASSIMP_BUILD_BOOST_WORKAROUND
)

add_custom_command(OUTPUT clean-but-external-cmd
  COMMAND rm -rf ./CMakeCache.txt ./CMakeFiles ./cmake_install.cmake ./Makefile
  COMMENT "Clean all but the external build folder"
)
add_custom_target(clean-but-external ALL
  DEPENDS clean-but-external-cmd
  VERBATIM
)

##############################################
function(POSTHANDLER)
  if(NOT NO_EXEC_TARGET)
    add_custom_command(
      TARGET ${SUB_TARGET_NAME} POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_CURRENT_BINARY_DIR}/${SUB_TARGET_NAME}
        ${CMAKE_SOURCE_DIR}/examples/${SUB_TARGET_NAME}/${SUB_TARGET_NAME})
    add_custom_command(
      TARGET ${SUB_TARGET_NAME} POST_BUILD
      COMMAND ${CMAKE_SOURCE_DIR}/examples/${SUB_TARGET_NAME}/${SUB_TARGET_NAME})
    add_custom_command(
      TARGET ${SUB_TARGET_NAME} POST_BUILD
      COMMAND rm ${CMAKE_SOURCE_DIR}/examples/${SUB_TARGET_NAME}/${SUB_TARGET_NAME})
    add_custom_command(
      TARGET ${SUB_TARGET_NAME} POST_BUILD
      COMMAND rm ${CMAKE_CURRENT_BINARY_DIR}/${SUB_TARGET_NAME})
  endif()
endfunction()

##############################################
set(SUB_TARGET_NAME gltest)
add_executable(${SUB_TARGET_NAME}
  examples/${SUB_TARGET_NAME}/${SUB_TARGET_NAME}.cpp
)
target_link_libraries(${SUB_TARGET_NAME}
  ${ALL_LIBS}
)
posthandler()

##############################################
set(SUB_TARGET_NAME playground)
add_executable(${SUB_TARGET_NAME}
  examples/${SUB_TARGET_NAME}/${SUB_TARGET_NAME}.cpp
)
target_link_libraries(${SUB_TARGET_NAME}
  ${ALL_LIBS}
)
posthandler()

##############################################
set(SUB_TARGET_NAME hello_triangle)
add_executable(${SUB_TARGET_NAME}
  examples/${SUB_TARGET_NAME}/${SUB_TARGET_NAME}.cpp
)
target_link_libraries(${SUB_TARGET_NAME}
  ${ALL_LIBS}
)
posthandler()

##############################################
set(SUB_TARGET_NAME shaders)
add_executable(${SUB_TARGET_NAME}
  examples/${SUB_TARGET_NAME}/${SUB_TARGET_NAME}.cpp
)
target_link_libraries(${SUB_TARGET_NAME}
  ${ALL_LIBS}
)
posthandler()

##############################################
set(SUB_TARGET_NAME texture)
add_executable(${SUB_TARGET_NAME}
  examples/${SUB_TARGET_NAME}/${SUB_TARGET_NAME}.cpp
)
target_link_libraries(${SUB_TARGET_NAME}
  ${ALL_LIBS}
)
posthandler()

##############################################
set(SUB_TARGET_NAME multi_texture)
add_executable(${SUB_TARGET_NAME}
  examples/${SUB_TARGET_NAME}/${SUB_TARGET_NAME}.cpp
)
target_link_libraries(${SUB_TARGET_NAME}
  ${ALL_LIBS}
)
posthandler()

##############################################
set(SUB_TARGET_NAME transform)
add_executable(${SUB_TARGET_NAME}
  examples/${SUB_TARGET_NAME}/${SUB_TARGET_NAME}.cpp
)
target_link_libraries(${SUB_TARGET_NAME}
  ${ALL_LIBS}
)
posthandler()

##############################################
set(SUB_TARGET_NAME coordinate_system)
add_executable(${SUB_TARGET_NAME}
  examples/${SUB_TARGET_NAME}/${SUB_TARGET_NAME}.cpp
)
target_link_libraries(${SUB_TARGET_NAME}
  ${ALL_LIBS}
)
posthandler()

##############################################
set(SUB_TARGET_NAME camera)
add_executable(${SUB_TARGET_NAME}
  examples/${SUB_TARGET_NAME}/${SUB_TARGET_NAME}.cpp
)
target_link_libraries(${SUB_TARGET_NAME}
  ${ALL_LIBS}
)
posthandler()

##############################################
set(SUB_TARGET_NAME color)
add_executable(${SUB_TARGET_NAME}
  examples/${SUB_TARGET_NAME}/${SUB_TARGET_NAME}.cpp
)
target_link_libraries(${SUB_TARGET_NAME}
  ${ALL_LIBS}
)
posthandler()

##############################################
set(SUB_TARGET_NAME basic_lighting)
add_executable(${SUB_TARGET_NAME}
  examples/${SUB_TARGET_NAME}/${SUB_TARGET_NAME}.cpp
)
target_link_libraries(${SUB_TARGET_NAME}
  ${ALL_LIBS}
)
posthandler()

##############################################
set(SUB_TARGET_NAME materials)
add_executable(${SUB_TARGET_NAME}
  examples/${SUB_TARGET_NAME}/${SUB_TARGET_NAME}.cpp
)
target_link_libraries(${SUB_TARGET_NAME}
  ${ALL_LIBS}
)
posthandler()

##############################################
set(SUB_TARGET_NAME lightingmaps)
add_executable(${SUB_TARGET_NAME}
  examples/${SUB_TARGET_NAME}/${SUB_TARGET_NAME}.cpp
)
target_link_libraries(${SUB_TARGET_NAME}
  ${ALL_LIBS}
)
posthandler()

##############################################
set(SUB_TARGET_NAME lightcaster)
add_executable(${SUB_TARGET_NAME}
  examples/${SUB_TARGET_NAME}/${SUB_TARGET_NAME}.cpp
)
target_link_libraries(${SUB_TARGET_NAME}
  ${ALL_LIBS}
)
posthandler()

##############################################
set(SUB_TARGET_NAME multiplelights)
add_executable(${SUB_TARGET_NAME}
  examples/${SUB_TARGET_NAME}/${SUB_TARGET_NAME}.cpp
)
target_link_libraries(${SUB_TARGET_NAME}
  ${ALL_LIBS}
)
posthandler()

##############################################
set(SUB_TARGET_NAME mesh)
add_executable(${SUB_TARGET_NAME}
  examples/${SUB_TARGET_NAME}/${SUB_TARGET_NAME}.cpp
)
target_link_libraries(${SUB_TARGET_NAME}
  ${ALL_LIBS} ${MODEL_LIBS}
)
posthandler()

##############################################
set(SUB_TARGET_NAME cubemaps)
add_executable(${SUB_TARGET_NAME}
  examples/${SUB_TARGET_NAME}/${SUB_TARGET_NAME}.cpp
)
target_link_libraries(${SUB_TARGET_NAME}
  ${ALL_LIBS}
)
posthandler()

##############################################
set(SUB_TARGET_NAME stroke)
add_executable(${SUB_TARGET_NAME}
  examples/${SUB_TARGET_NAME}/${SUB_TARGET_NAME}.cpp
)
target_link_libraries(${SUB_TARGET_NAME}
  ${ALL_LIBS}
)
posthandler()

##############################################
set(SUB_TARGET_NAME depth_testing)
add_executable(${SUB_TARGET_NAME}
  examples/${SUB_TARGET_NAME}/${SUB_TARGET_NAME}.cpp
)
target_link_libraries(${SUB_TARGET_NAME}
  ${ALL_LIBS}
)
posthandler()

##############################################
set(SUB_TARGET_NAME stencil_buffer)
add_executable(${SUB_TARGET_NAME}
  examples/${SUB_TARGET_NAME}/${SUB_TARGET_NAME}.cpp
)
target_link_libraries(${SUB_TARGET_NAME}
  ${ALL_LIBS}
)
posthandler()

##############################################
set(SUB_TARGET_NAME blending)
add_executable(${SUB_TARGET_NAME}
  examples/${SUB_TARGET_NAME}/${SUB_TARGET_NAME}.cpp
)
target_link_libraries(${SUB_TARGET_NAME}
  ${ALL_LIBS}
)
posthandler()

##############################################
set(SUB_TARGET_NAME framebuffer)
add_executable(${SUB_TARGET_NAME}
  examples/${SUB_TARGET_NAME}/${SUB_TARGET_NAME}.cpp
)
target_link_libraries(${SUB_TARGET_NAME}
  ${ALL_LIBS}
)
posthandler()

##############################################
set(SUB_TARGET_NAME advanced_glsl)
add_executable(${SUB_TARGET_NAME}
  examples/${SUB_TARGET_NAME}/${SUB_TARGET_NAME}.cpp
)
target_link_libraries(${SUB_TARGET_NAME}
  ${ALL_LIBS}
)
posthandler()

##############################################
set(SUB_TARGET_NAME geometry_shader)
add_executable(${SUB_TARGET_NAME}
  examples/${SUB_TARGET_NAME}/${SUB_TARGET_NAME}.cpp
)
target_link_libraries(${SUB_TARGET_NAME}
  ${ALL_LIBS}
)
posthandler()

##############################################
set(SUB_TARGET_NAME geometry_model)
add_executable(${SUB_TARGET_NAME}
  examples/${SUB_TARGET_NAME}/${SUB_TARGET_NAME}.cpp
)
target_link_libraries(${SUB_TARGET_NAME}
  ${ALL_LIBS} ${MODEL_LIBS}
)
posthandler()

##############################################
set(SUB_TARGET_NAME anti_aliasing)
add_executable(${SUB_TARGET_NAME}
  examples/${SUB_TARGET_NAME}/${SUB_TARGET_NAME}.cpp
)
target_link_libraries(${SUB_TARGET_NAME}
  ${ALL_LIBS}
)
posthandler()

##############################################
set(SUB_TARGET_NAME hello_world)
add_executable(${SUB_TARGET_NAME}
  examples/${SUB_TARGET_NAME}/${SUB_TARGET_NAME}.cpp
)
target_link_libraries(${SUB_TARGET_NAME}
  ${ALL_LIBS}
)
posthandler()

##############################################
set(SUB_TARGET_NAME instancing)
add_executable(${SUB_TARGET_NAME}
  examples/${SUB_TARGET_NAME}/${SUB_TARGET_NAME}.cpp
)
target_link_libraries(${SUB_TARGET_NAME}
  ${ALL_LIBS}
)
posthandler()

##############################################
set(SUB_TARGET_NAME asteroid)
add_executable(${SUB_TARGET_NAME}
  examples/${SUB_TARGET_NAME}/${SUB_TARGET_NAME}.cpp
)
target_link_libraries(${SUB_TARGET_NAME}
  ${ALL_LIBS} ${MODEL_LIBS}
)
posthandler()

##############################################
set(SUB_TARGET_NAME advanced_lighting)
add_executable(${SUB_TARGET_NAME}
  examples/${SUB_TARGET_NAME}/${SUB_TARGET_NAME}.cpp
)
target_link_libraries(${SUB_TARGET_NAME}
  ${ALL_LIBS}
)
posthandler()

##############################################
set(SUB_TARGET_NAME gamma_correction)
add_executable(${SUB_TARGET_NAME}
  examples/${SUB_TARGET_NAME}/${SUB_TARGET_NAME}.cpp
)
target_link_libraries(${SUB_TARGET_NAME}
  ${ALL_LIBS}
)
posthandler()

##############################################
set(SUB_TARGET_NAME shadow_mapping)
add_executable(${SUB_TARGET_NAME}
  examples/${SUB_TARGET_NAME}/${SUB_TARGET_NAME}.cpp
)
target_link_libraries(${SUB_TARGET_NAME}
  ${ALL_LIBS}
)
posthandler()

##############################################
set(SUB_TARGET_NAME hello_mesh)
add_executable(${SUB_TARGET_NAME}
  examples/${SUB_TARGET_NAME}/${SUB_TARGET_NAME}.cpp
)
target_link_libraries(${SUB_TARGET_NAME}
  ${ALL_LIBS} ${MODEL_LIBS}
)
posthandler()

##############################################
set(SUB_TARGET_NAME point_shadow)
add_executable(${SUB_TARGET_NAME}
  examples/${SUB_TARGET_NAME}/${SUB_TARGET_NAME}.cpp
)
target_link_libraries(${SUB_TARGET_NAME}
  ${ALL_LIBS}
)
posthandler()

##############################################
set(SUB_TARGET_NAME hello_shadow)
add_executable(${SUB_TARGET_NAME}
  examples/${SUB_TARGET_NAME}/${SUB_TARGET_NAME}.cpp
)
target_link_libraries(${SUB_TARGET_NAME}
  ${ALL_LIBS} ${MODEL_LIBS}
)
posthandler()

##############################################
set(SUB_TARGET_NAME normal_map)
add_executable(${SUB_TARGET_NAME}
  examples/${SUB_TARGET_NAME}/${SUB_TARGET_NAME}.cpp
)
target_link_libraries(${SUB_TARGET_NAME}
  ${ALL_LIBS}
)
posthandler()

##############################################
set(SUB_TARGET_NAME parallax_map)
add_executable(${SUB_TARGET_NAME}
  examples/${SUB_TARGET_NAME}/${SUB_TARGET_NAME}.cpp
)
target_link_libraries(${SUB_TARGET_NAME}
  ${ALL_LIBS}
)
posthandler()

##############################################
set(SUB_TARGET_NAME hdr)
add_executable(${SUB_TARGET_NAME}
  examples/${SUB_TARGET_NAME}/${SUB_TARGET_NAME}.cpp
)
target_link_libraries(${SUB_TARGET_NAME}
  ${ALL_LIBS}
)
posthandler()

##############################################
set(SUB_TARGET_NAME bloom)
add_executable(${SUB_TARGET_NAME}
  examples/${SUB_TARGET_NAME}/${SUB_TARGET_NAME}.cpp
)
target_link_libraries(${SUB_TARGET_NAME}
  ${ALL_LIBS}
)
posthandler()

##############################################
set(SUB_TARGET_NAME deferred_shading)
add_executable(${SUB_TARGET_NAME}
  examples/${SUB_TARGET_NAME}/${SUB_TARGET_NAME}.cpp
)
target_link_libraries(${SUB_TARGET_NAME}
  ${ALL_LIBS} ${MODEL_LIBS}
)
posthandler()

##############################################
set(SUB_TARGET_NAME ssao)
add_executable(${SUB_TARGET_NAME}
  examples/${SUB_TARGET_NAME}/${SUB_TARGET_NAME}.cpp
)
target_link_libraries(${SUB_TARGET_NAME}
  ${ALL_LIBS} ${MODEL_LIBS}
)
posthandler()

##############################################
set(SUB_TARGET_NAME pbr_lighting)
add_executable(${SUB_TARGET_NAME}
  examples/${SUB_TARGET_NAME}/${SUB_TARGET_NAME}.cpp
)
target_link_libraries(${SUB_TARGET_NAME}
  ${ALL_LIBS}
)
posthandler()

##############################################
set(SUB_TARGET_NAME pbr_texture)
add_executable(${SUB_TARGET_NAME}
  examples/${SUB_TARGET_NAME}/${SUB_TARGET_NAME}.cpp
)
target_link_libraries(${SUB_TARGET_NAME}
  ${ALL_LIBS}
)
posthandler()

##############################################
set(SUB_TARGET_NAME pbr_diffuse_irradiance)
add_executable(${SUB_TARGET_NAME}
  examples/${SUB_TARGET_NAME}/${SUB_TARGET_NAME}.cpp
)
target_link_libraries(${SUB_TARGET_NAME}
  ${ALL_LIBS}
)
posthandler()

##############################################
set(SUB_TARGET_NAME pbr_specular_ibl)
add_executable(${SUB_TARGET_NAME}
  examples/${SUB_TARGET_NAME}/${SUB_TARGET_NAME}.cpp
)
target_link_libraries(${SUB_TARGET_NAME}
  ${ALL_LIBS}
)
posthandler()

##############################################
set(SUB_TARGET_NAME pbr_specular_ibl_texture)
add_executable(${SUB_TARGET_NAME}
  examples/${SUB_TARGET_NAME}/${SUB_TARGET_NAME}.cpp
)
target_link_libraries(${SUB_TARGET_NAME}
  ${ALL_LIBS}
)
posthandler()

##############################################
set(SUB_TARGET_NAME text_rendering)
add_executable(${SUB_TARGET_NAME}
  examples/${SUB_TARGET_NAME}/${SUB_TARGET_NAME}.cpp
)
target_link_libraries(${SUB_TARGET_NAME}
  ${ALL_LIBS} ${FONT_LIBS}
)
posthandler()

##############################################
set(SUB_TARGET_NAME hello_sprite)
add_executable(${SUB_TARGET_NAME}
  examples/${SUB_TARGET_NAME}/${SUB_TARGET_NAME}.cpp
)
target_link_libraries(${SUB_TARGET_NAME}
  ${ALL_LIBS}
)
posthandler()

##############################################
set(SUB_TARGET_NAME bouncing_ball)
add_executable(${SUB_TARGET_NAME}
  examples/${SUB_TARGET_NAME}/${SUB_TARGET_NAME}.cpp
)
target_link_libraries(${SUB_TARGET_NAME}
  ${ALL_LIBS}
)
posthandler()

##############################################
set(SUB_TARGET_NAME cloth_animation)
add_executable(${SUB_TARGET_NAME}
  examples/${SUB_TARGET_NAME}/${SUB_TARGET_NAME}.cpp
)
target_link_libraries(${SUB_TARGET_NAME}
  ${ALL_LIBS}
)
posthandler()

##############################################
MESSAGE(STATUS "Build type: " ${CMAKE_BUILD_TYPE})
MESSAGE(STATUS "Library Type: " ${LIB_TYPE})
MESSAGE(STATUS "Compiler flags:" ${CMAKE_CXX_COMPILE_FLAGS})
MESSAGE(STATUS "Compiler cxx debug flags:" ${CMAKE_CXX_FLAGS_DEBUG})
MESSAGE(STATUS "Compiler cxx release flags:" ${CMAKE_CXX_FLAGS_RELEASE})
MESSAGE(STATUS "Compiler cxx min size flags:" ${CMAKE_CXX_FLAGS_MINSIZEREL})
MESSAGE(STATUS "Compiler cxx flags:" ${CMAKE_CXX_FLAGS})
