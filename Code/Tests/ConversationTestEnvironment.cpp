#include "ConversationTestEnvironment.h"

#include "AzCore/Asset/AssetManagerComponent.h"
#include "AzCore/Component/ComponentApplication.h"
#include "AzCore/IO/Streamer/StreamerComponent.h"
#include "AzCore/Jobs/JobManagerComponent.h"
#include "AzCore/RTTI/RTTIMacros.h"
#include "AzFramework/Application/Application.h"
#include "AzFramework/Asset/AssetCatalogComponent.h"
#include "AzFramework/Asset/AssetSystemComponent.h"
#include "AzFramework/Entity/GameEntityContextComponent.h"
#include "Components/ConversationAssetRefComponent.h"
#include "Conversation/DialogueComponent.h"
#include "ConversationSystemComponent.h"

namespace ConversationTest
{
    class ConversationTestApplication : public AzFramework::Application
    {
    public:
        auto GetRequiredSystemComponents() const
            -> AZ::ComponentTypeList override
        {
            using namespace Conversation;

            AZ::ComponentTypeList components =
                AzFramework::Application::GetRequiredSystemComponents();

            components.insert(
                components.end(),
                {
                    azrtti_typeid<AZ::StreamerComponent>(),
                    azrtti_typeid<AZ::AssetManagerComponent>(),
                    azrtti_typeid<AZ::JobManagerComponent>(),

                    azrtti_typeid<AzFramework::AssetCatalogComponent>(),
                    azrtti_typeid<AzFramework::GameEntityContextComponent>(),
                    azrtti_typeid<
                        AzFramework::AssetSystem::AssetSystemComponent>(),
                });

            return components;
        }
    };

    void ConversationTestEnvironment::AddGemsAndComponents()
    {
        using namespace Conversation;

        AddDynamicModulePaths({ "LmbrCentral" });

        AddComponentDescriptors(
            { ConversationSystemComponent::CreateDescriptor() });
        AddComponentDescriptors({ DialogueComponent::CreateDescriptor() });
        AddComponentDescriptors(
            { ConversationAssetRefComponent::CreateDescriptor() });

        AddRequiredComponents({ azrtti_typeid<ConversationSystemComponent>() });
    }

    auto ConversationTestEnvironment::CreateApplicationInstance()
        -> AZ::ComponentApplication*
    {
        return aznew ConversationTestApplication;
    }

} // namespace ConversationTest
