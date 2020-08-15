project(metashell_pch)

aux_source_directory(src SOURCES)
add_library(${PROJECT_NAME} STATIC ${SOURCES})
add_library(metashell::pch ALIAS ${PROJECT_NAME})

enable_warnings(${PROJECT_NAME})

target_precompile_headers(${PROJECT_NAME} PUBLIC include/metashell/pch/pch.hpp)

set(METASHELL_PRECOMPILED_HEADER
  "${CMAKE_CURRENT_SOURCE_DIR}/include/metashell/pch/pch.hpp" CACHE INTERNAL
  "Precompiled header"
)

target_link_libraries(
  ${PROJECT_NAME}

  boost::algorithm
  boost::container
  boost::core
  boost::filesystem
  boost::iterator
  boost::optional
  boost::preprocessor
  boost::range
  boost::thread
  boost::tuple
  boost::utility
  boost::wave
)
