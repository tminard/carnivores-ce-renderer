cmake_minimum_required(VERSION 3.5 FATAL_ERROR)
project(CarnivoresRenderer)

# Add .lib files
link_directories(${CMAKE_SOURCE_DIR}/lib)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)

FetchContent_Declare(json URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz)
FetchContent_MakeAvailable(json)

# Add source files
file(GLOB_RECURSE SOURCE_FILES 
	${CMAKE_SOURCE_DIR}/src/*.c
	${CMAKE_SOURCE_DIR}/src/*.cpp)
	
# Add header files
file(GLOB_RECURSE HEADER_FILES 
	${CMAKE_SOURCE_DIR}/src/*.h
	${CMAKE_SOURCE_DIR}/src/*.hpp)

# Configure assets header file
configure_file(src/helpers/RootDir.h.in src/helpers/RootDir.h)
include_directories(${CMAKE_BINARY_DIR}/src)
	
# Define the executable
add_executable(${PROJECT_NAME} ${HEADER_FILES} ${SOURCE_FILES})

# We need a CMAKE_DIR with some code to find external dependencies
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")

#######################################
# LOOK for the packages that we need! #
#######################################

# GLM
find_package(GLM REQUIRED)
message(STATUS "GLM included at ${GLM_INCLUDE_DIR}")

# GLFW
find_package(GLFW3 REQUIRED)
message(STATUS "Found GLFW3 in ${GLFW3_INCLUDE_DIR}")


# Put all libraries into a variable
set(LIBS glfw3 nlohmann_json::nlohmann_json)

# Manually add GLAD
set(GLAD_SRC_DIR "${CMAKE_SOURCE_DIR}/thirdparty/glad/src")
set(GLAD_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/thirdparty/glad/include")

add_library(glad STATIC "${GLAD_SRC_DIR}/glad.c")
target_include_directories(glad PUBLIC "${GLAD_INCLUDE_DIR}")

# Define the include DIRs
include_directories(
	"${CMAKE_SOURCE_DIR}/src"
	"${CMAKE_SOURCE_DIR}/include"
	${GLAD_INCLUDE_DIR}
)


# Define the link libraries
if(APPLE)
    target_link_libraries(${PROJECT_NAME} glad ${GLFW3_LIBRARY} "-framework OpenAL" "-framework OpenGL" "-framework CoreFoundation" "-framework IOKit" "-framework CoreGraphics" "-framework AppKit" ${LIBS})
else()
    target_link_libraries(${PROJECT_NAME} glad ${GLFW3_LIBRARY} OpenAL32.lib OpenGL32.lib ${LIBS})
endif()

if(APPLE)
    find_library(GLFW3_LIBRARY glfw3 PATHS /usr/local/lib /opt/homebrew/lib)
    if (NOT GLFW3_LIBRARY)
        message(FATAL_ERROR "Could not find glfw3 library")
    endif()
    message(STATUS "GLFW3 found: ${GLFW3_LIBRARY}")
    link_directories("/usr/local/lib" "/opt/homebrew/lib")
endif()

# Create virtual folders to make it look nicer in VS
if(MSVC_IDE)
	# Macro to preserve source files hierarchy in the IDE
	macro(GroupSources curdir)
		file(GLOB children RELATIVE ${PROJECT_SOURCE_DIR}/${curdir} ${PROJECT_SOURCE_DIR}/${curdir}/*)

		foreach(child ${children})
			if(IS_DIRECTORY ${PROJECT_SOURCE_DIR}/${curdir}/${child})
				GroupSources(${curdir}/${child})
			else()
				string(REPLACE "/" "\\" groupname ${curdir})
				string(REPLACE "src" "Sources" groupname ${groupname})
				source_group(${groupname} FILES ${PROJECT_SOURCE_DIR}/${curdir}/${child})
			endif()
		endforeach()
	endmacro()

	# Run macro
	GroupSources(src)
endif()

# Copy dlls
if(WIN32)
	set_property(TARGET ${PROJECT_NAME} PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "$<TARGET_FILE_DIR:${PROJECT_NAME}>")
	add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy_directory
		"${PROJECT_SOURCE_DIR}/dlls"
		$<TARGET_FILE_DIR:${PROJECT_NAME}>)
endif()

# Specify the source and destination for config.json
set(CONFIG_SOURCE "${CMAKE_SOURCE_DIR}/runtime/config.json")
set(CONFIG_DESTINATION "$<TARGET_FILE_DIR:${PROJECT_NAME}>/config.json")

# Copy config.json to the executable directory
add_custom_command(
    TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    ${CONFIG_SOURCE} ${CONFIG_DESTINATION}
)

