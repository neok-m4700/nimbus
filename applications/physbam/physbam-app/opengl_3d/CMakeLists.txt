CMAKE_MINIMUM_REQUIRED( VERSION 2.8.7 )
MARK_AS_ADVANCED(CMAKE_BACKWARDS_COMPATIBILITY)
###########################################################################
#Project-specific Configuration
###########################################################################

#Set a project name
SET(PROJECT_NAME opengl_3d)
PROJECT(${PROJECT_NAME})

#Manually list your cpp and h files here - do NOT use GLOB/GLOB_RECURSE
SET(${PROJECT_NAME}_cpp
        ${PROJECT_SOURCE_DIR}/main.cpp)
SET(${PROJECT_NAME}_h
        "")
        
#List the libraries you want to link against - including a higher-level
#library automatically includes the lower-level ones
SET(LIBRARIES_TO_LINK PhysBAM_Rendering-OpenGL)
  
# Added to fix the compile issue on Ubuntu 16.04 - omidm
LIST(APPEND LIBRARIES_TO_LINK X11)
        
#you can force cache variables to be on or off here
#SET(PHYSBAM_USE_CUDA ON CACHE BOOL ON FORCE)
        
INCLUDE(${PROJECT_SOURCE_DIR}/../../physbam-lib/Scripts/CMake/Nimbus_Application.cmake)
