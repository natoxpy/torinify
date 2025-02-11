cmake_minimum_required(VERSION 3.31)
project(torinify)

# Enable AddressSanitizer and debugging symbols
if (WIN32)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")
endif()

if (UNIX)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -fsanitize=address")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -fsanitize=address")
endif()

file(GLOB lib_sources
     "src/*/*.c"
)

list(REMOVE_ITEM lib_sources "src/main.c")

add_library(${PROJECT_NAME} STATIC ${lib_sources})
target_include_directories(${PROJECT_NAME} PUBLIC include)

if (WIN32)
    set(SQLite3_INCLUDE_DIR $ENV{SQLite3_INCLUDE_DIR})
    set(SQLite3_LIBRARY $ENV{SQLite3_LIBRARY})
    set(TagLib_DIR $ENV{TagLib_DIR})
endif()

# Find and link SQLite3 correctly
find_package(SQLite3 REQUIRED)
target_link_libraries(${PROJECT_NAME} PUBLIC SQLite::SQLite3)

# Find and link FFmpeg libraries

set(FFMPEG_INCLUDE_DIR $ENV{FFMPEG_INCLUDE_DIR})
set(FFMPEG_LIBRARY $ENV{FFMPEG_LIBRARY})

include_directories(${FFMPEG_INCLUDE_DIR})
link_directories(${FFMPEG_LIBRARY})

target_link_libraries(${PROJECT_NAME}
    PUBLIC
    avcodec avformat avutil swresample swscale)

# Find and link Taglib correctly
# Try to find TagLib via vcpkg first
find_package(TagLib CONFIG)
target_link_libraries(${PROJECT_NAME} PUBLIC TagLib::TagLib)

# Link against math library
target_link_libraries(${PROJECT_NAME} PUBLIC m)

# Define which headers to expose
set(EXPOSED_HEADERS
    ${PROJECT_SOURCE_DIR}/include/torinify/core.h
)

install(FILES ${EXPOSED_HEADERS} DESTINATION ${CMAKE_BINARY_DIR}/include)

# Install the library (optional)
install(TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_BINARY_DIR}/lib)

# ------- MAIN EXECUTABLE --------
# Define the executable
add_executable(main "src/main.c")

# Link the executable to the library
target_include_directories(main PUBLIC include)
target_link_libraries(main PUBLIC ${PROJECT_NAME})
target_link_libraries(main PRIVATE
    m        # math library
    z        # zlib
)
