add_library(archer MODULE ompt-tsan.cpp)
add_library(archer_static STATIC ompt-tsan.cpp)
add_library(archer_static_report STATIC archer_report.cpp)

install(TARGETS archer archer_static
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib)

install(TARGETS archer archer_static_report
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib)

install(FILES suppressions.txt DESTINATION share/archer)
