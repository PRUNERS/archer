configure_file(clang-archer.in clang-archer)
configure_file(clang-archer++.in clang-archer++)
install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/clang-archer ${CMAKE_CURRENT_BINARY_DIR}/clang-archer++ DESTINATION bin)
