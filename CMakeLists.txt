cmake_minimum_required(VERSION 3.14)
project(KanbanBoard LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

# 1. UI lib (FTXUI) fetch
FetchContent_Declare(
  ftxui
  GIT_REPOSITORY https://github.com/ArthurSonzogni/FTXUI.git
  GIT_TAG v5.0.0
)
FetchContent_MakeAvailable(ftxui)

# 2. JSON lib (nlohmann/json) fetch
FetchContent_Declare(
  json
  GIT_REPOSITORY https://github.com/nlohmann/json.git
  GIT_TAG v3.11.3
)
FetchContent_MakeAvailable(json)

# 3. Testing lib (Catch2) fetch
FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG v3.4.0
)
FetchContent_MakeAvailable(Catch2)

add_executable(kanban_board 
    ./src/main.cpp 
    ./src/KanbanBoard.cpp 
    ./src/KanbanBoard.hpp 
    ./src/Task.hpp
    ./src/StorageManager.cpp
    ./src/StorageManager.hpp
)

target_link_libraries(kanban_board PRIVATE 
    ftxui::screen 
    ftxui::dom 
    ftxui::component 
    nlohmann_json::nlohmann_json
)

add_executable(run_tests 
    ./tests/tests.cpp 
    ./src/KanbanBoard.cpp
    ./src/KanbanBoard.hpp 
    ./src/StorageManager.cpp
    ./src/StorageManager.hpp
)

target_link_libraries(run_tests PRIVATE 
    Catch2::Catch2WithMain
    nlohmann_json::nlohmann_json
)
