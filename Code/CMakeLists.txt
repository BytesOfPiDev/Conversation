
# Currently we are in the Code folder: ${CMAKE_CURRENT_LIST_DIR}
# Get the platform specific folder ${pal_dir} for the current folder: ${CMAKE_CURRENT_LIST_DIR}/Platform/${PAL_PLATFORM_NAME}
# Note: ly_get_list_relative_pal_filename will take care of the details for us, as this may be a restricted platform
#       in which case it will see if that platform is present here or in the restricted folder.
#       i.e. It could here in our gem : Gems/Conversation/Code/Platform/<platorm_name>  or
#            <restricted_folder>/<platform_name>/Gems/Conversation/Code
ly_get_list_relative_pal_filename(pal_dir ${CMAKE_CURRENT_LIST_DIR}/Platform/${PAL_PLATFORM_NAME} ${o3de_gem_restricted_path} ${o3de_gem_path} ${o3de_gem_name})

# Now that we have the platform abstraction layer (PAL) folder for this folder, thats where we will find the
# traits for this platform. Traits for a platform are defines for things like whether or not something in this gem
# is supported by this platform.
include(${pal_dir}/PAL_${PAL_PLATFORM_NAME_LOWERCASE}.cmake)

# Add the Conversation.Static target
# Note: We include the common files and the platform specific files which are set in conversation_common_files.cmake
# and in ${pal_dir}/conversation_${PAL_PLATFORM_NAME_LOWERCASE}_files.cmake
ly_add_target(
    NAME Conversation.Static STATIC
    NAMESPACE Gem
    FILES_CMAKE
        conversation_files.cmake
        ${pal_dir}/conversation_${PAL_PLATFORM_NAME_LOWERCASE}_files.cmake
    INCLUDE_DIRECTORIES
        PUBLIC
            Include
        PRIVATE
            Source
    BUILD_DEPENDENCIES
        PUBLIC
            AZ::AzCore
            AZ::AzFramework
)

# Here add Conversation target, it depends on the Conversation.Static
ly_add_target(
    NAME Conversation ${PAL_TRAIT_MONOLITHIC_DRIVEN_MODULE_TYPE}
    NAMESPACE Gem
    FILES_CMAKE
        conversation_shared_files.cmake
        ${pal_dir}/conversation_shared_${PAL_PLATFORM_NAME_LOWERCASE}_files.cmake
    INCLUDE_DIRECTORIES
        PUBLIC
            Include
        PRIVATE
            Source
    BUILD_DEPENDENCIES
        PRIVATE
            Gem::Conversation.Static
)

# By default, we will specify that the above target Conversation would be used by
# Client and Server type targets when this gem is enabled.  If you don't want it
# active in Clients or Servers by default, delete one of both of the following lines:
ly_create_alias(NAME Conversation.Clients NAMESPACE Gem TARGETS Gem::Conversation)
ly_create_alias(NAME Conversation.Servers NAMESPACE Gem TARGETS Gem::Conversation)

# If we are on a host platform, we want to add the host tools targets like the Conversation.Editor target which
# will also depend on Conversation.Static
if(PAL_TRAIT_BUILD_HOST_TOOLS)
    ly_add_target(
        NAME Conversation.Editor.Static STATIC
        NAMESPACE Gem
        AUTOMOC
        AUTORCC
        FILES_CMAKE
            conversation_editor_files.cmake
        INCLUDE_DIRECTORIES
            PRIVATE
                Source
            PUBLIC
                Include
        BUILD_DEPENDENCIES
            PUBLIC
                AZ::AzToolsFramework
                Gem::Conversation.Static
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
                Gem::Conversation.Editor.Static
    )

    # By default, we will specify that the above target Conversation would be used by
    # Tool and Builder type targets when this gem is enabled.  If you don't want it
    # active in Tools or Builders by default, delete one of both of the following lines:
    ly_create_alias(NAME Conversation.Tools    NAMESPACE Gem TARGETS Gem::Conversation.Editor)
    ly_create_alias(NAME Conversation.Builders NAMESPACE Gem TARGETS Gem::Conversation.Editor)


endif()

################################################################################
# Tests
################################################################################
# See if globally, tests are supported
if(PAL_TRAIT_BUILD_TESTS_SUPPORTED)
    # We globally support tests, see if we support tests on this platform for Conversation.Static
    if(PAL_TRAIT_CONVERSATION_TEST_SUPPORTED)
        # We support Conversation.Tests on this platform, add Conversation.Tests target which depends on Conversation.Static
        ly_add_target(
            NAME Conversation.Tests ${PAL_TRAIT_TEST_TARGET_TYPE}
            NAMESPACE Gem
            FILES_CMAKE
                conversation_files.cmake
                conversation_tests_files.cmake
            INCLUDE_DIRECTORIES
                PRIVATE
                    Tests
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
    endif()

    # If we are a host platform we want to add tools test like editor tests here
    if(PAL_TRAIT_BUILD_HOST_TOOLS)
        # We are a host platform, see if Editor tests are supported on this platform
        if(PAL_TRAIT_CONVERSATION_EDITOR_TEST_SUPPORTED)
            # We support Conversation.Editor.Tests on this platform, add Conversation.Editor.Tests target which depends on Conversation.Editor
            ly_add_target(
                NAME Conversation.Editor.Tests ${PAL_TRAIT_TEST_TARGET_TYPE}
                NAMESPACE Gem
                FILES_CMAKE
                    conversation_editor_tests_files.cmake
                INCLUDE_DIRECTORIES
                    PRIVATE
                        Tests
                        Source
                BUILD_DEPENDENCIES
                    PRIVATE
                        AZ::AzTest
                        Gem::Conversation.Editor
            )

            # Add Conversation.Editor.Tests to googletest
            ly_add_googletest(
                NAME Gem::Conversation.Editor.Tests
            )
        endif()
    endif()
endif()
