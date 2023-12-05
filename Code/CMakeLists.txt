
# Currently we are in the Code folder: ${CMAKE_CURRENT_LIST_DIR}
# Get the platform specific folder ${pal_dir} for the current folder: ${CMAKE_CURRENT_LIST_DIR}/Platform/${PAL_PLATFORM_NAME}
# Note: o3de_pal_dir will take care of the details for us, as this may be a restricted platform
#       in which case it will see if that platform is present here or in the restricted folder.
#       i.e. It could here in our gem : Gems/Conversation/Code/Platform/<platorm_name>  or
#            <restricted_folder>/<platform_name>/Gems/Conversation/Code
o3de_pal_dir(pal_dir ${CMAKE_CURRENT_LIST_DIR}/Platform/${PAL_PLATFORM_NAME} "${gem_restricted_path}" "${gem_path}" "${gem_parent_relative_path}")

# Now that we have the platform abstraction layer (PAL) folder for this folder, thats where we will find the
# traits for this platform. Traits for a platform are defines for things like whether or not something in this gem
# is supported by this platform.
include(${pal_dir}/PAL_${PAL_PLATFORM_NAME_LOWERCASE}.cmake)

ly_add_target(
    NAME ConversationCanvas APPLICATION
    NAMESPACE Gem
    AUTOMOC
    AUTORCC
    FILES_CMAKE
        conversationcanvas_files.cmake
    INCLUDE_DIRECTORIES
        PRIVATE
            .
            Source
            ${pal_source_dir}
        PUBLIC
            Include
    BUILD_DEPENDENCIES
        PRIVATE
            AZ::AzFramework
            AZ::AzToolsFramework
            Gem::AtomLyIntegration_CommonFeatures.Static
            Gem::AtomToolsFramework.Editor
            Gem::AtomToolsFramework.Static
            Gem::Conversation.Editor
    RUNTIME_DEPENDENCIES
        AZ::AzCore
        AZ::AzFramework
        Gem::AtomToolsFramework.Editor
)

ly_set_gem_variant_to_load(TARGETS ConversationCanvas VARIANTS Tools)

# Add a 'builders' alias to allow the ConversationCanvas root gem path to be added to the generated
# cmake_dependencies.<project>.assetprocessor.setreg to allow the asset scan folder for it to be added
ly_create_alias(NAME ConversationCanvas.Builders NAMESPACE Gem)

# Add build dependency to Editor for the ConversationCanvas application since
# Editor opens up the ConversationCanvas
# ly_add_dependencies(Editor Gem::ConversationCanvas)

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
        Source/Tools/ConversationCanvasApplication.cpp
        PROPERTIES
            COMPILE_DEFINITIONS
                LY_CMAKE_TARGET="ConversationCanvas"
    )
else()
  message(FATAL_ERROR "Cannot set LY_CMAKE_TARGET define to ConversationCanvas as the target doesn't exist anymore."
        " Perhaps it has been renamed")
endif()

# The Conversation.API target declares the common interface that users of this gem should depend on in their targets
ly_add_target(
    NAME Conversation.API INTERFACE
    NAMESPACE Gem
    FILES_CMAKE
        conversation_api_files.cmake
        ${pal_dir}/conversation_api_files.cmake
    INCLUDE_DIRECTORIES
        INTERFACE
            Include
    BUILD_DEPENDENCIES
        INTERFACE
           AZ::AzCore
)

ly_add_target(
    NAME Conversation.Static STATIC
    NAMESPACE Gem
    FILES_CMAKE
        conversation_files.cmake
    INCLUDE_DIRECTORIES
        PRIVATE
            Include
            Source
    BUILD_DEPENDENCIES
        PUBLIC
            AZ::AzCore
            AZ::AzFramework
            Gem::Atom_RPI.Public
            Gem::ScriptCanvas
            Gem::LmbrCentral
            Legacy::CryCommon
        PRIVATE
)

# Here add Conversation target, it depends on the Private Object library and Public API interface
ly_add_target(
    NAME Conversation ${PAL_TRAIT_MONOLITHIC_DRIVEN_MODULE_TYPE}
    NAMESPACE Gem
    FILES_CMAKE
        conversation_shared_files.cmake
        ${pal_dir}/conversation_shared_files.cmake
    INCLUDE_DIRECTORIES
        PUBLIC
            Include
        PRIVATE
            Source
    BUILD_DEPENDENCIES
        PUBLIC
            Gem::Conversation.API
        PRIVATE
            Gem::Conversation.Static
)

# By default, we will specify that the above target Conversation would be used by
# Client and Server type targets when this gem is enabled.  If you don't want it
# active in Clients or Servers by default, delete one of both of the following lines:
ly_create_alias(NAME Conversation.Clients NAMESPACE Gem TARGETS Gem::Conversation)

# For the Client and Server variants of Conversation Gem, an alias to the Conversation.API target will be made
ly_create_alias(NAME Conversation.Clients.API NAMESPACE Gem TARGETS Gem::Conversation.API)

# If we are on a host platform, we want to add the host tools targets like the Conversation.Editor MODULE target
if(PAL_TRAIT_BUILD_HOST_TOOLS)
  # The Conversation.Editor.API target can be used by other gems that want to interact with the Conversation.Editor module
  ly_add_target(
        NAME Conversation.Editor.API INTERFACE
        NAMESPACE Gem
        FILES_CMAKE
            conversation_editor_api_files.cmake
            ${pal_dir}/conversation_editor_api_files.cmake
        INCLUDE_DIRECTORIES
            INTERFACE
                Include
        BUILD_DEPENDENCIES
            INTERFACE
                AZ::AzToolsFramework
    )

  ly_add_target(
        NAME Conversation.Editor.Static STATIC
        NAMESPACE Gem
        AUTOMOC
        AUTORCC
        FILES_CMAKE
            conversation_editor_files.cmake
        INCLUDE_DIRECTORIES
            PRIVATE
                Include
                Source
        BUILD_DEPENDENCIES
            PUBLIC
                AZ::AssetBuilderSDK
                AZ::AzToolsFramework
                Gem::AtomToolsFramework.Static
                Gem::AtomToolsFramework.Editor
                Gem::Conversation.Static
                Gem::ScriptCanvas.Editor
                Gem::GraphCanvasWidgets
                Gem::GraphModel.Editor.Static
            PRIVATE
              Gem::ScriptCanvas.Editor.Static

    )

  ly_add_target(
        NAME Conversation.Editor GEM_MODULE
        NAMESPACE Gem
        AUTOMOC
        FILES_CMAKE
            conversation_editor_shared_files.cmake
        INCLUDE_DIRECTORIES
            PRIVATE
                Source
            PUBLIC
                Include
        BUILD_DEPENDENCIES
            PUBLIC
                Gem::AtomToolsFramework.Editor
                Gem::Conversation.Editor.API
                Gem::Conversation.Editor.Static
                Gem::ScriptCanvas.Editor
            PRIVATE
    )

  # By default, we will specify that the above target Conversation would be used by
  # Tool and Builder type targets when this gem is enabled.  If you don't want it
  # active in Tools or Builders by default, delete one of both of the following lines:
  ly_create_alias(NAME Conversation.Tools    NAMESPACE Gem TARGETS Gem::Conversation.Editor)
  ly_create_alias(NAME Conversation.Builders NAMESPACE Gem TARGETS Gem::Conversation.Editor)

  # For the Tools and Builders variants of Conversation Gem, an alias to the Conversation.Editor API target will be made
  ly_create_alias(NAME Conversation.Tools.API NAMESPACE Gem TARGETS Gem::Conversation.Editor.API)
  ly_create_alias(NAME Conversation.Builders.API NAMESPACE Gem TARGETS Gem::Conversation.Editor.API)

endif()

################################################################################
# Tests
################################################################################
# See if globally, tests are supported
if(PAL_TRAIT_BUILD_TESTS_SUPPORTED)
  # We globally support tests, see if we support tests on this platform for Conversation.Tests
  if(PAL_TRAIT_CONVERSATION_TEST_SUPPORTED)
    # We support Conversation.Tests on this platform, add dependency on the Static Object target
    ly_add_target(
            NAME Conversation.Tests ${PAL_TRAIT_TEST_TARGET_TYPE}
            NAMESPACE Gem
            FILES_CMAKE
                conversation_tests_files.cmake
            INCLUDE_DIRECTORIES
                PRIVATE
                    Tests
                    Include
                    Source
            BUILD_DEPENDENCIES
                PRIVATE
                    AZ::AzTest
                    AZ::AzFramework
                    Gem::Conversation.Static
        )

    # Add Conversation.Tests to googletest
    ly_add_googletest(
            NAME Gem::Conversation.Tests
        )

    list(APPEND convTestTargets Conversation.Tests)

    ly_add_target_files(
        TARGETS
            ${convTestTargets}
        FILES
            ${CMAKE_CURRENT_SOURCE_DIR}/Tests/whatever.txt
        OUTPUT_SUBDIRECTORY
            Test.Assets/Gems/Conversation
    )
  endif()

  # If we are a host platform we want to add tools test like editor tests here
  if(PAL_TRAIT_BUILD_HOST_TOOLS)
    # We are a host platform, see if Editor tests are supported on this platform
    if(PAL_TRAIT_CONVERSATION_EDITOR_TEST_SUPPORTED)
      # We support Conversation.Editor.Tests on this platform, add Conversation.Editor.Tests target which depends on
      # private Conversation.Editor.Private.Object target
      ly_add_target(
                NAME Conversation.Editor.Tests ${PAL_TRAIT_TEST_TARGET_TYPE}
                NAMESPACE Gem
                AUTOMOC
                AUTORCC
                FILES_CMAKE
                    conversation_editor_tests_files.cmake
                    conversationcanvas_files.cmake
                INCLUDE_DIRECTORIES
                    PRIVATE
                        Tests
                        Source
                BUILD_DEPENDENCIES
                    PRIVATE
                        AZ::AzTest
                        Gem::Conversation.Static
                        AZ::AzFramework
                        AZ::AzToolsFramework
                        Gem::AtomLyIntegration_CommonFeatures.Static
                        Gem::AtomToolsFramework.Editor
                        Gem::AtomToolsFramework.Static
                        Gem::Conversation.Editor
            )

      # Add Conversation.Editor.Tests to googletest
      ly_add_googletest(
                NAME Gem::Conversation.Editor.Tests
            )

    endif()
  endif()
endif()
