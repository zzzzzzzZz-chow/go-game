cmake_minimum_required (VERSION 3.11)
# add_compile_options(-g)
# --- Fetch FTXUI --------------------------------------------------------------
include(FetchContent)

FetchContent_Declare(ftxui
  GIT_REPOSITORY https://github.com/ArthurSonzogni/ftxui
  GIT_TAG v5.0.0
)

FetchContent_GetProperties(ftxui)
if(NOT ftxui_POPULATED)
  FetchContent_Populate(ftxui)
  add_subdirectory(${ftxui_SOURCE_DIR} ${ftxui_BINARY_DIR} EXCLUDE_FROM_ALL)
endif()
# ------------------------------------------------------------------------------
 
project(go-game
  LANGUAGES CXX
  VERSION 1.0.0
)
 
add_executable(go-game main.cpp)
target_include_directories(go-game PRIVATE src)
 
target_link_libraries(go-game
  PRIVATE ftxui::screen
  PRIVATE ftxui::dom
  PRIVATE ftxui::component # Not needed for this example.
)

