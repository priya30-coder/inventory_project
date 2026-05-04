cmake_minimum_required(VERSION 3.14)
project(HybridInventoryManager LANGUAGES C CXX)

set(CMAKE_C_STANDARD   11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED   ON)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Compiler warnings
if(MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -Wextra -pedantic)
endif()

include_directories(include)

add_executable(inventory_manager
    src/inventory.c
    src/InventoryManager.cpp
    src/main.cpp
)

# Make sure the C file is compiled as C and CPP files as C++
set_source_files_properties(src/inventory.c          PROPERTIES LANGUAGE C)
set_source_files_properties(src/InventoryManager.cpp PROPERTIES LANGUAGE CXX)
set_source_files_properties(src/main.cpp             PROPERTIES LANGUAGE CXX)
