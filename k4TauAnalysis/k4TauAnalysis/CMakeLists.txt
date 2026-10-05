#[[
Copyright (c) 2020-2024 Key4hep-Project.

This file is part of Key4hep.
See https://key4hep.github.io/key4hep-doc/ for further info.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
]]

file(GLOB _plugin_sources src/components/*.cpp)

add_library(TauClassification SHARED src/GenTauUtils.cpp)

target_include_directories(TauClassification PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

target_link_libraries(TauClassification PUBLIC EDM4HEP::edm4hep)

gaudi_add_module(k4TauAnalysisPlugins
                 SOURCES ${_plugin_sources}
                 LINK Gaudi::GaudiKernel
                      k4FWCore::k4FWCore
                      EDM4HEP::edm4hep
                      TauTool::TauTool
                      TauTool::TauToolDict
                      TauClassification
                      )

install(TARGETS k4TauAnalysisPlugins
  EXPORT ${CMAKE_PROJECT_NAME}Targets
  RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}" COMPONENT bin
  LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}" COMPONENT shlib
  PUBLIC_HEADER DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/${CMAKE_PROJECT_NAME}"
  COMPONENT dev)

install(TARGETS TauClassification
  EXPORT ${CMAKE_PROJECT_NAME}Targets
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})