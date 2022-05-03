add_library(easyloggingpp src/easylogging++.cc)

target_include_directories(easyloggingpp
          INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/src
          )

# add_compile_definitions(ELPP_NO_DEFAULT_LOG_FILE)
