include_directories(.)

# file(GLOB sources *.cpp)
set (sources
  stringext.cpp
  evdevjoy.cpp
  bitext.cpp
  sdlxboxmap.cpp
  platform.cpp
  logging.cpp
)

add_executable(${PROJECT_NAME} ${sources})

target_include_directories(${PROJECT_NAME}
    PRIVATE ${CMAKE_CURRENT_LIST_DIR}
)

target_link_libraries(${PROJECT_NAME} PRIVATE
        PkgConfig::LIBEVDEV
        cxxopts
        easyloggingpp
)

target_precompile_headers(${PROJECT_NAME}
  PUBLIC
    stringext.h
  PRIVATE
    <string>
    <iostream>
    <unordered_map>
    <map>
    <vector>
)

install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin)
