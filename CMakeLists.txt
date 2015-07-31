project(roskhepera)

find_package(PkgConfig REQUIRED)
pkg_check_modules(ROSCPP roscpp REQUIRED)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_executable(roskhepera main.cpp)
target_link_libraries(roskhepera
    ${ROSCPP_LDFLAGS}
)
