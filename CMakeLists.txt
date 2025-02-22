cmake_minimum_required(VERSION 3.31)
project(torinify)

set(CMAKE_POSITION_INDEPENDENT_CODE OFF)

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

set(SQL_DIR ${CMAKE_SOURCE_DIR}/sql)
set(BUILD_SQL_DIR ${CMAKE_BINARY_DIR}/sql)

file(MAKE_DIRECTORY ${BUILD_SQL_DIR})
file(GLOB SQL_FILES ${SQL_DIR}/*.sql)

set(SQL_OBJECT_FILES "")

foreach(SQL_FILE ${SQL_FILES})
    get_filename_component(SQL_NAME ${SQL_FILE} NAME_WE)  # Extract file name without extension

    # Define the output C file for the SQL data
    set(OUTPUT_C_FILE ${BUILD_SQL_DIR}/${SQL_NAME}.c)

    # Convert the SQL file to a C source with xxd and store it with a clean name
    add_custom_command(
        OUTPUT ${OUTPUT_C_FILE}
        COMMAND xxd -i -n "_sql_${SQL_NAME}" ${SQL_FILE} > ${OUTPUT_C_FILE}
        DEPENDS ${SQL_FILE}
        COMMENT "Converting ${SQL_FILE} into C source"
    )

    # Add the generated C file to the list of source files
    list(APPEND SQL_OBJECT_FILES ${OUTPUT_C_FILE})
endforeach()

message(STATUS "SQL Object Files: ${SQL_OBJECT_FILES}")

add_library(${PROJECT_NAME} STATIC ${lib_sources} ${SQL_OBJECT_FILES})

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

find_package(TagLib REQUIRED)
target_link_libraries(${PROJECT_NAME} PUBLIC tag_c)

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
