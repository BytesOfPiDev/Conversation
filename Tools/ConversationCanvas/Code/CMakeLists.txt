#
# Copyright (c) Contributors to the Open 3D Engine Project.
# For complete copyright and license terms please see the LICENSE at the root of this distribution.
#
# SPDX-License-Identifier: Apache-2.0 OR MIT
#
#

if(NOT PAL_TRAIT_BUILD_HOST_TOOLS)
  return()
endif()

o3de_pal_dir(pal_source_dir ${CMAKE_CURRENT_LIST_DIR}/Source/Platform/${PAL_PLATFORM_NAME} "${gem_restricted_path}" "${gem_path}" "${gem_parent_relative_path}")

include(${pal_source_dir}/PAL_${PAL_PLATFORM_NAME_LOWERCASE}.cmake)

if(NOT PAL_TRAIT_ATOM_CONVERSATION_CANVAS_APPLICATION_SUPPORTED)
  message(FATAL_ERROR "Conversation canvas not supported")
  return()
endif()

ly_add_target(
    NAME ConversationCanvas APPLICATION
    NAMESPACE Gem
    AUTOMOC
    AUTORCC
    FILES_CMAKE
        conversationcanvas_files.cmake
        ${pal_source_dir}/platform_${PAL_PLATFORM_NAME_LOWERCASE}_files.cmake
    PLATFORM_INCLUDE_FILES
        ${pal_source_dir}/tool_dependencies_${PAL_PLATFORM_NAME_LOWERCASE}.cmake
    INCLUDE_DIRECTORIES
        PRIVATE
            .
            Source
            ${pal_source_dir}
        PUBLIC
            Include
    BUILD_DEPENDENCIES
        PRIVATE
            Gem::CommonFeaturesAtom.Static
            Gem::AtomToolsFramework.Editor
            Gem::AtomToolsFramework.Static
            Gem::Conversation.API
            Gem::Conversation
            Gem::Conversation.Static
            Gem::GraphModel.Editor
            Gem::GraphModel.Editor.Static
    RUNTIME_DEPENDENCIES
        Gem::AtomToolsFramework.Editor
)

ly_set_gem_variant_to_load(TARGETS ConversationCanvas VARIANTS Tools)

# Add a 'builders' alias to allow the ConversationCanvas root gem path to be added to the generated
# cmake_dependencies.<project>.assetprocessor.setreg to allow the asset scan folder for it to be added
ly_create_alias(NAME ConversationCanvas.Builders NAMESPACE Gem)

# Add build dependency to Editor for the ConversationCanvas application since
# Editor opens up the ConversationCanvas
ly_add_dependencies(Editor Gem::ConversationCanvas)

# Inject the project path into the ConversationCanvas VS debugger command arguments if the build system being invoked
# in a project centric view
if(NOT PROJECT_NAME STREQUAL "O3DE")
  set_property(TARGET ConversationCanvas APPEND PROPERTY VS_DEBUGGER_COMMAND_ARGUMENTS "--project-path=\"${CMAKE_SOURCE_DIR}\"")
endif()

# Adds the ConversationCanvas target as a C preprocessor define so that it can be used as a Settings Registry
# specialization in order to look up the generated .setreg which contains the dependencies
# specified for the target.
if(TARGET ConversationCanvas)
  set_source_files_properties(
        Source/ConversationCanvasApplication.cpp
        PROPERTIES
            COMPILE_DEFINITIONS
                LY_CMAKE_TARGET="ConversationCanvas"
    )
else()
  message(FATAL_ERROR "Cannot set LY_CMAKE_TARGET define to ConversationCanvas as the target doesn't exist anymore."
        " Perhaps it has been renamed")
endif()
