set(Boost_NO_BOOST_CMAKE ON)
find_package(Boost 1.41 COMPONENTS REQUIRED)
message(STATUS "Boost Include: ${Boost_INCLUDE_DIR}")
message(STATUS "Boost Libraries: ${Boost_LIBRARY_DIRS}")
message(STATUS "Boost Libraries: ${Boost_LIBRARIES}")

INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIR})
LINK_DIRECTORIES(${Boost_LIBRARY_DIRS})

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -frtti")

add_executable(archer-bl archer-bl.cpp)
file(COPY clang-archer clang-archer++ ar ranlib
          DESTINATION ${LLVM_TOOLS_BINARY_DIR})
install(PROGRAMS clang-archer clang-archer++ ar ranlib DESTINATION bin/archer)
install(TARGETS archer-bl RUNTIME DESTINATION bin/archer)
