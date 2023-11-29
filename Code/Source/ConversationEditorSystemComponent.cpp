#include "Tools/EditorActorText.h"
#include <ConversationEditorSystemComponent.h>

#include <AssetBuilderSDK/AssetBuilderSDK.h>
#include <AzCore/Serialization/SerializeContext.h>
#include <AzToolsFramework/API/ViewPaneOptions.h>
#include <Tools/ConversationGraphContext.h>

namespace ConversationEditor
{
    void ConversationEditorSystemComponent::Reflect(AZ::ReflectContext* context)
    {
        if (auto serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<ConversationEditorSystemComponent, ConversationSystemComponent>()->Version(0)->Attribute(
                AZ::Edit::Attributes::SystemComponentTags,
                AZStd::vector<AZ::Crc32>({
                    AssetBuilderSDK::ComponentTags::AssetBuilder,
                }));

            if (AZ::EditContext* editContext = serializeContext->GetEditContext())
            {
                editContext->Class<ConversationEditorSystemComponent>("ConversationEditor", "Handles editing conversation files.")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                    ->Attribute(AZ::Edit::Attributes::AppearsInAddComponentMenu, AZ_CRC("System"))
                    ->Attribute(AZ::Edit::Attributes::AutoExpand, true);
            }
        }
    }

    ConversationEditorSystemComponent::ConversationEditorSystemComponent()
        : m_conversationEditorNodeManager(AZ_CRC_CE("ConversationEditor"))
    {
    }

    ConversationEditorSystemComponent::~ConversationEditorSystemComponent() = default;

    void ConversationEditorSystemComponent::GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided)
    {
        BaseSystemComponent::GetProvidedServices(provided);
        provided.push_back(AZ_CRC_CE("ConversationEditorService"));
    }

    void ConversationEditorSystemComponent::GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible)
    {
        BaseSystemComponent::GetIncompatibleServices(incompatible);
        incompatible.push_back(AZ_CRC_CE("ConversationEditorService"));
    }

    void ConversationEditorSystemComponent::GetRequiredServices([[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& required)
    {
        BaseSystemComponent::GetRequiredServices(required);
    }

    void ConversationEditorSystemComponent::GetDependentServices([[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& dependent)
    {
        BaseSystemComponent::GetDependentServices(dependent);
        dependent.push_back(AZ_CRC_CE("ConversationAssetBuilderService"));
    }

    void ConversationEditorSystemComponent::Init()
    {
        ConversationSystemComponent::Init();
    }

    void ConversationEditorSystemComponent::Activate()
    {
        ConversationSystemComponent::Activate();
        AzToolsFramework::EditorEvents::Bus::Handler::BusConnect();
    }

    void ConversationEditorSystemComponent::Deactivate()
    {
        AzToolsFramework::EditorEvents::Bus::Handler::BusDisconnect();
        ConversationSystemComponent::Deactivate();
    }
} // namespace ConversationEditor
