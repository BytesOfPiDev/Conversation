
set(CONVERSATION_BUILD_DOCS OFF CACHE BOOL "If true, builds the Conversation gem's documentation")

set(gem_path ${CMAKE_CURRENT_LIST_DIR})
set(gem_json ${gem_path}/gem.json)
o3de_restricted_path(${gem_json} gem_restricted_path gem_parent_relative_path)

o3de_pal_dir(pal_dir ${CMAKE_CURRENT_LIST_DIR}/Platform/${PAL_PLATFORM_NAME} "${gem_restricted_path}" "${gem_path}" "${gem_parent_relative_path}")

# Now that we have the platform abstraction layer (PAL) folder for this folder, thats where we will find the
# project cmake for this platform.
include(${pal_dir}/${PAL_PLATFORM_NAME_LOWERCASE}_gem.cmake)

ly_add_external_target_path(${CMAKE_CURRENT_LIST_DIR}/3rdParty)

add_subdirectory(Code)
add_subdirectory(Tools)

if(${CONVERSATION_BUILD_DOCS})
  add_subdirectory(docs)
endif()

set(CONVERSATION_VERSION_MAJOR 0)
set(CONVERSATION_VERSION_MINOR 0)
set(CONVERSATION_VERSION_PATCH 0)

