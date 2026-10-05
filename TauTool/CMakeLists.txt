cmake_minimum_required(VERSION 3.15)
project(TauTool VERSION 0.1 LANGUAGES CXX)


include(GNUInstallDirs)
# For std c++20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(podio REQUIRED)
find_package(EDM4HEP REQUIRED)

PODIO_GENERATE_DATAMODEL(TauTool tautool.yaml headers sources
  UPSTREAM_EDM edm4hep:${EDM4HEP_DATA_DIR}/edm4hep.yaml
  OUTPUT_FOLDER ${CMAKE_CURRENT_BINARY_DIR}
)
PODIO_ADD_DATAMODEL_CORE_LIB(TauTool "${headers}" "${sources}"
  OUTPUT_FOLDER ${CMAKE_CURRENT_BINARY_DIR}
)
target_link_libraries(TauTool PUBLIC EDM4HEP::edm4hep)
target_include_directories(TauTool PUBLIC
   $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/TauTool>
)
PODIO_ADD_ROOT_IO_DICT(TauToolDict TauTool "${headers}" src/selection.xml
 OUTPUT_FOLDER ${CMAKE_CURRENT_BINARY_DIR}
)

add_executable(smoke_test smoke_test.cpp)
target_link_libraries(smoke_test PRIVATE TauTool TauToolDict podio::podioRootIO)

add_executable(smoke_read smoke_reader.cpp)
target_link_libraries(smoke_read PRIVATE TauTool TauToolDict podio::podioRootIO)

install(TARGETS TauTool
  EXPORT TauToolTargets
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
  PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/TauTool
)
# Does not neet headers
install(TARGETS TauToolDict
  EXPORT TauToolTargets
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
install(FILES
  ${CMAKE_CURRENT_BINARY_DIR}/TauToolDictDict.rootmap
  ${CMAKE_CURRENT_BINARY_DIR}/libTauToolDict_rdict.pcm
  DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

install(EXPORT TauToolTargets
  FILE TauToolTargets.cmake
  NAMESPACE TauTool::
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/TauTool
)

include(CMakePackageConfigHelpers)

configure_package_config_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/TauToolConfig.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/TauToolConfig.cmake
  INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/TauTool
)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/TauToolConfig.cmake
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/TauTool
)