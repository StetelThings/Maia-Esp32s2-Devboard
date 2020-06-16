# The following lines of boilerplate have to be in your project's CMakeLists
# in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.5)

# Add RainMaker components and other common application components
set(EXTRA_COMPONENT_DIRS ${CMAKE_CURRENT_LIST_DIR}/../../../esp-rainmaker/components ${CMAKE_CURRENT_LIST_DIR}/../../../esp-rainmaker/examples/common)

set(PROJECT_VER "1.1")
include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(maia_raimaker_template)
