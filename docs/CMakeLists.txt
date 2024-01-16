find_package(Doxygen REQUIRED)
find_package(Sphinx REQUIRED)

set(DOXYGEN_INPUT_DIR ${gem_path}/Code)
set(DOXYGEN_OUTPUT_DIR ${CMAKE_CURRENT_LIST_DIR}/doxygen)
set(DOXYGEN_INDEX_FILE ${DOXYGEN_OUTPUT_DIR}/html/index.html)
set(DOXYFILE_IN ${CMAKE_CURRENT_LIST_DIR}/Doxyfile.in)
set(DOXYFILE_OUT ${CMAKE_CURRENT_LIST_DIR}/Doxyfile)

configure_file(${DOXYFILE_IN} ${DOXYFILE_OUT} @ONLY)

file(MAKE_DIRECTORY ${DOXYGEN_OUTPUT_DIR})

add_custom_command(OUTPUT ${DOXYGEN_INDEX_FILE}
                   DEPENDS Conversation
                   COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYFILE_OUT}
                   MAIN_DEPENDENCY ${DOXYFILE_OUT} ${DOXYFILE_IN}
                   COMMENT "Generating docs"
                   VERBATIM)

add_custom_target(Doxygen ALL DEPENDS ${DOXYGEN_INDEX_FILE})

set(SPHINX_SOURCE ${CMAKE_CURRENT_LIST_DIR})
set(SPHINX_BUILD ${SPHINX_SOURCE}/sphinx)
set(SPHINX_INDEX_FILE ${SPHINX_BUILD}/index.html)

# Only regenerate Sphinx when:
# - Doxygen has rerun
# - Our doc files have been updated
# - The Sphinx config has been updated
add_custom_command(OUTPUT ${SPHINX_INDEX_FILE}
                   COMMAND
                      ${SPHINX_EXECUTABLE} -b html
                      # Tell Breathe where to find the Doxygen output
                      -Dbreathe_projects.Conversation=${DOXYGEN_OUTPUT_DIR}/xml
                      ${SPHINX_SOURCE} ${SPHINX_BUILD}
                   WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
                   DEPENDS
                      # Other docs files you want to track should go here (or in some variable)
                      ${DOXYGEN_INDEX_FILE}
                      ${CMAKE_CURRENT_LIST_DIR}/index.rst
                      ${CMAKE_CURRENT_LIST_DIR}/gettingstarted.rst
                      ${CMAKE_CURRENT_LIST_DIR}/installation.rst
                      api.rst
                   MAIN_DEPENDENCY ${SPHINX_SOURCE}/conf.py
                   COMMENT "Generating documentation with Sphinx")

# Nice named target so we can run the job easily
add_custom_target(Sphinx ALL DEPENDS ${SPHINX_INDEX_FILE})

# Add an install target to install the docs
include(GNUInstallDirs)
install(DIRECTORY ${SPHINX_BUILD}
DESTINATION ${CMAKE_INSTALL_DOCDIR})
