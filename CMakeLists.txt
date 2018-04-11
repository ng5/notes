cmake_minimum_required(VERSION 3.0)
project(ExternalProjects)
include(ExternalProject)
set(EXTERNAL_INSTALL_LOCATION ${CMAKE_BINARY_DIR}/external)
ExternalProject_Add(spdlog
    GIT_REPOSITORY https://github.com/gabime/spdlog.git
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${EXTERNAL_INSTALL_LOCATION}
)
ExternalProject_Add(StaticJSON
    GIT_REPOSITORY https://github.com/ng5/StaticJSON.git
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${EXTERNAL_INSTALL_LOCATION}
)
ExternalProject_Add(LucenePlusPlus
    GIT_REPOSITORY https://github.com/luceneplusplus/LucenePlusPlus.git
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${EXTERNAL_INSTALL_LOCATION}
)
