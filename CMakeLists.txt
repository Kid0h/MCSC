cmake_minimum_required(VERSION 3.15.0)
project(MCSC VERSION 1.0.0)

# C++ Standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_EXTENSIONS OFF)

# WIN32 Runtime
if (WIN32)
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd")
endif()

# Fix Ubuntu no-pie
if (NOT WIN32 AND (UNIX AND NOT APPLE))
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -no-pie")
endif()

# Sources
file(GLOB SOURCES
    "src/*.cpp"
    "src/*.c"

    "vendor/glad/src/glad.c"

    "vendor/imgui/*.cpp"

    "vendor/base64/base64.c"
)

# Executable
if(WIN32)
    add_executable(MCSC WIN32 "src/main.cpp")
else()
    add_executable(MCSC "src/main.cpp")
endif()
target_sources(MCSC PRIVATE ${SOURCES})

# glad
set(GLAD_INCLUDE_DIR "vendor/glad/include")

# glfw
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(USE_MSVC_RUNTIME_LIBRARY_DLL OFF CACHE BOOL "" FORCE)
add_subdirectory("vendor/glfw")

# imgui
set(IMGUI_INCLUDE_DIR "vendor/imgui")

# libcurl
find_package(CURL REQUIRED)
OPTION(CURL_STATICLIB "Use static libcurl" ON)
if(CURL_STATICLIB)
    add_compile_definitions("CURL_STATICLIB")
    if (WIN32)
        set(CURL_LIBRARY "${CURL_LIBRARY};Normaliz.lib;Ws2_32.lib;Wldap32.lib;Crypt32.lib;advapi32.lib")
    else()
        set(CURL_LIBRARY "${CURL_LIBRARY};ssl;crypto;idn2;z;pthread")
    endif()
endif()
message("CURL_INCLUDE_DIR: ${CURL_INCLUDE_DIR}")
message("CURL_LIBRARY: ${CURL_LIBRARY}")

# kidCurl
set(KIDCURL_INCLUDE_DIR "vendor/kidCurl/include")

# json
set(JSON_INCLUDE_DIR "vendor/json/include")

# base64
set(BASE64_INCLUDE_DIR "vendor/base64")

# nativefiledialog-extended
add_subdirectory("vendor/nfde")

# Includes
target_include_directories(MCSC PRIVATE ${GLAD_INCLUDE_DIR} ${IMGUI_INCLUDE_DIR} ${CURL_INCLUDE_DIR} ${KIDCURL_INCLUDE_DIR} ${JSON_INCLUDE_DIR} ${BASE64_INCLUDE_DIR})

# Links
target_link_libraries(MCSC PRIVATE glfw ${CURL_LIBRARY} nfd)