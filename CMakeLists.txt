cmake_minimum_required(VERSION 3.22)
project(cppfeatures)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_C_STANDARD 23)
IF (UNIX)
    if (NOT DEFINED CONDA_ENV)
        message(FATAL_ERROR "You must set CONDA_ENV variable")
    endif ()
ENDIF ()
include_directories(${PROJECT_SOURCE_DIR}/include)
if (UNIX)
    include_directories(${PROJECT_SOURCE_DIR}/include ${CONDA_ENV}/include ${CONDA_ENV}/include/benchmarks/include)
    set(CMAKE_C_FLAGS " -Wall -Wextra -Wpedantic")
    set(CMAKE_C_FLAGS_DEBUG " -g")
    set(CMAKE_C_FLAGS_RELEASE "-fmodules-ts -flto -O3")
    set(CMAKE_CXX_FLAGS " -Wall -Wextra -Wpedantic")
    set(CMAKE_CXX_FLAGS_DEBUG "-g")
    set(CMAKE_CXX_FLAGS_RELEASE "-flto -O3")
ENDIF ()
set(SOURCES c coderpad codility codinggame concurrency gotw1 leetcode oop standard stl ub)
IF (UNIX)
    list(APPEND SOURCES networking)
ENDIF ()
foreach (X IN LISTS SOURCES)
    message(STATUS "generating sources for ${X}")
    file(GLOB files "${X}/*.cpp" "${X}/*.c" "${X}/.cxx")
    foreach (file ${files})
        get_filename_component(x ${file} NAME_WLE)
        add_executable("${X}_${x}" ${file})
        if (UNIX)
            target_link_libraries("${X}_${x}" -static-libgcc -static-libstdc++ rt dl pthread)
        endif ()
    endforeach ()
endforeach ()

if (UNIX)
    file(GLOB benchmarks "benchmarks/*.cpp" "benchmarks/*.c" "benchmarks/.cxx")
    file(GLOB benchmarks_src "benchmarks/src/*.cpp")
    find_package(benchmark REQUIRED)
    find_library(INTEL_TBB tbb)
    foreach (file ${benchmarks})
        get_filename_component(x ${file} NAME_WLE)
        add_executable("benchmarks_${x}" ${file}
                ${benchmark_src}
        )
        include_directories(benchmarks/include)
        target_link_libraries("benchmarks_${x}" -static-libgcc -static-libstdc++ benchmark dl pthread)
    endforeach ()
endif ()
list(APPEND NON_DISTRIBUTABLES_PRE
        [[api-ms-win-.*]]
        [[ext-ms-.*]]
        [[kernel32\.dll]]
        [[.*/system32/.*\.dll]]
        [[libc\.so\..*]]
        [[libm\.so\..*]]
        [[librt\.so\..*]]
        [[libdl\.so\..*]]
        [[libpthread\.so\..*]]
        [[^/lib.*]]
        [[^ld-linux.*]]
        [[^/lib64.*]]
        [[^/usr/lib.*]]
        [[^/usr/lib64.*]]
)
list(APPEND NON_DISTRIBUTABLES_POST
        [[.*/system32/.*\.dll]]
)
install(RUNTIME_DEPENDENCY_SET APP_DEPS PRE_EXCLUDE_REGEXES ${NON_DISTRIBUTABLES_PRE} POST_EXCLUDE_REGEXES ${NON_DISTRIBUTABLES_POST})
