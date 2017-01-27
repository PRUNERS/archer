file(COPY clang-archer clang-archer++
  DESTINATION ${LLVM_TOOLS_BINARY_DIR})
install(PROGRAMS clang-archer clang-archer++ DESTINATION bin)
