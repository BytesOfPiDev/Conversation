#include "ConversationEditorTestEnvironment.h"

#include "AzCore/Asset/AssetManagerComponent.h"
#include "AzCore/IO/Streamer/StreamerComponent.h"
#include "AzCore/Jobs/JobManagerComponent.h"
#include "AzFramework/Application/Application.h"
#include "AzFramework/Asset/AssetCatalogComponent.h"
#include "AzFramework/Asset/AssetSystemComponent.h"
#include "AzFramework/Entity/GameEntityContextComponent.h"

#include "Conversation/DialogueComponent.h"
#include "ConversationEditorSystemComponent.h"

namespace ConversationEditorTest
{

    class ConversationEditorTestApplication : public AzFramework::Application
    {
    public:
        AZ_DISABLE_COPY_MOVE(ConversationEditorTestApplication); // NOLINT

        ConversationEditorTestApplication() = default;
        ~ConversationEditorTestApplication() override = default;

        auto GetRequiredSystemComponents() const -> AZ::ComponentTypeList override
        {
            AZ::ComponentTypeList components = AzFramework::Application::GetRequiredSystemComponents();

            components.insert(
                components.end(),
                {
                    azrtti_typeid<AZ::StreamerComponent>(),
                    azrtti_typeid<AZ::AssetManagerComponent>(),
                    azrtti_typeid<AZ::JobManagerComponent>(),

                    azrtti_typeid<AzFramework::AssetCatalogComponent>(),
                    azrtti_typeid<AzFramework::GameEntityContextComponent>(),
                    azrtti_typeid<AzFramework::AssetSystem::AssetSystemComponent>(),
                });

            return components;
        }
    };

    void ConversationEditorTestEnvironment::AddGemsAndComponents()
    {
        AddDynamicModulePaths({ "LmbrCentral" });

        AddComponentDescriptors({ ConversationEditor::ConversationEditorSystemComponent::CreateDescriptor() });
        AddComponentDescriptors({ Conversation::DialogueComponent::CreateDescriptor() });

        AddRequiredComponents({ azrtti_typeid<ConversationEditor::ConversationEditorSystemComponent>() });
    }

    auto ConversationEditorTestEnvironment::CreateApplicationInstance() -> AZ::ComponentApplication*
    {
        return aznew ConversationEditorTestApplication;
    }
} // namespace ConversationEditorTest
