add_library(archer MODULE ompt-tsan.cpp counter.cpp)
add_library(archer_static STATIC ompt-tsan.cpp counter.cpp)
add_library(farcher MODULE ftsan.c)
add_library(farcher_static STATIC ftsan.c)

install(TARGETS archer archer_static farcher farcher_static
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib)

install(FILES suppressions.txt DESTINATION share/archer)
