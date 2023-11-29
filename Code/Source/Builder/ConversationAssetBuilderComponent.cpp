#include <AzCore/Serialization/EditContextConstants.inl>
#include <AzCore/Serialization/SerializeContext.h>
#include <Builder/ConversationAssetBuilderComponent.h>
#include <Conversation/ConversationAsset.h>

#include <Builder/ConversationAssetBuilderComponent.h>

namespace ConversationEditor
{
    void DialogueAssetBuilderComponent::Reflect(AZ::ReflectContext* context)
    {
        if (auto* serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<DialogueAssetBuilderComponent, AZ::Component>()->Version(0)->Attribute(
                AZ::Edit::Attributes::SystemComponentTags,
                AZStd::vector<AZ::Crc32>({
                    AssetBuilderSDK::ComponentTags::AssetBuilder,
                }));
        }
    }

    DialogueAssetBuilderComponent::DialogueAssetBuilderComponent() = default;

    DialogueAssetBuilderComponent::~DialogueAssetBuilderComponent() = default;

    void DialogueAssetBuilderComponent::Init()
    {
    }

    void DialogueAssetBuilderComponent::Activate()
    {
        AssetBuilderSDK::AssetBuilderDesc builderDescriptor;
        builderDescriptor.m_name = "Dialogue Asset Worker Builder";
        builderDescriptor.m_patterns.emplace_back(
            Conversation::ConversationAsset::ProductExtensionPattern, AssetBuilderSDK::AssetBuilderPattern::PatternType::Wildcard);
        builderDescriptor.m_patterns.emplace_back(
            Conversation::ConversationAsset::SourceExtensionPattern, AssetBuilderSDK::AssetBuilderPattern::PatternType::Wildcard);
        builderDescriptor.m_busId = azrtti_typeid<ConversationAssetBuilderWorker>();
        builderDescriptor.m_version = 1; // if you change this, all assets will automatically rebuild
        builderDescriptor.m_analysisFingerprint = ""; // if you change this, all assets will re-analyze but not necessarily rebuild.
        builderDescriptor.m_createJobFunction = [ObjectPtr = &m_dialogueAssetBuilder](auto&& PH1, auto&& PH2)
        {
            ObjectPtr->CreateJobs(AZStd::forward<decltype(PH1)>(PH1), AZStd::forward<decltype(PH2)>(PH2));
        };
        builderDescriptor.m_processJobFunction = [ObjectPtr = &m_dialogueAssetBuilder](auto&& PH1, auto&& PH2)
        {
            ObjectPtr->ProcessJob(AZStd::forward<decltype(PH1)>(PH1), AZStd::forward<decltype(PH2)>(PH2));
        };

        // note that this particular builder does in fact emit various kinds of dependencies (as an example).
        // if your builder is simple and emits no dependencies (for example, it just processes a single file and that file
        // doesn't really depend on any other files or jobs), setting the BF_EmitsNoDependencies flag
        // will improve "fast analysis" scan performance.
        builderDescriptor.m_flags = AssetBuilderSDK::AssetBuilderDesc::BF_None;

        m_dialogueAssetBuilder.BusConnect(builderDescriptor.m_busId);

        AssetBuilderSDK::AssetBuilderBus::Broadcast(&AssetBuilderSDK::AssetBuilderBusTraits::RegisterBuilderInformation, builderDescriptor);
    }

    void DialogueAssetBuilderComponent::Deactivate()
    {
        m_dialogueAssetBuilder.BusDisconnect();
    }

    void DialogueAssetBuilderComponent::GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided)
    {
        provided.push_back(AZ_CRC_CE("ConversationAssetBuilderService"));
    }

    void DialogueAssetBuilderComponent::GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible)
    {
        incompatible.push_back(AZ_CRC_CE("ConversationAssetBuilderService"));
    }

    void DialogueAssetBuilderComponent::GetRequiredServices([[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& required)
    {
        // required.push_back(AZ_CRC("ConversationEditorService", 0x11a7a5d5));
    }

    void DialogueAssetBuilderComponent::GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent)
    {
        AZ_UNUSED(dependent);
    }
} // namespace ConversationEditor
