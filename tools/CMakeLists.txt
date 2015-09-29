find_package(Boost 1.41 COMPONENTS filesystem unit_test_framework REQUIRED)
message("Boost Include: ${Boost_INCLUDE_DIR}")
message("Boost Libraries: ${Boost_LIBRARY_DIRS}")
message("Boost Libraries: ${Boost_LIBRARIES}")

INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIR})
LINK_DIRECTORIES(${Boost_LIBRARY_DIRS})

set(CMAKE_CXX_FLAGS "-frtti")

file(MAKE_DIRECTORY ${LLVM_TOOLS_BINARY_DIR}/archer)
add_executable(archer-bl archer-bl.cpp)
file(COPY clang-archer DESTINATION ${LLVM_TOOLS_BINARY_DIR}/archer)
file(COPY clang-archer++ DESTINATION ${LLVM_TOOLS_BINARY_DIR}/archer)
file(COPY ar DESTINATION ${LLVM_TOOLS_BINARY_DIR}/archer)
file(COPY ranlib DESTINATION ${LLVM_TOOLS_BINARY_DIR}/archer)
install(TARGETS archer-bl
        RUNTIME DESTINATION bin/archer)
install(FILES clang-archer clang-archer++ ar ranlib DESTINATION bin/archer)
