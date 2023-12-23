#include "Conversation/Components/DialogueComponentConfig.h"

#include "AzCore/Asset/AssetSerializer.h"
#include "AzCore/Component/ComponentBus.h"
#include "AzCore/Memory/SystemAllocator.h"
#include "AzCore/RTTI/ReflectContext.h"
#include "AzCore/Serialization/EditContext.h"
#include "AzCore/Serialization/EditContextConstants.inl"
#include "AzCore/Serialization/SerializeContext.h"

#include "Conversation/ConversationTypeIds.h"

namespace Conversation
{

    AZ_RTTI_NO_TYPE_INFO_IMPL(DialogueComponentConfig, AZ::ComponentConfig); // NOLINT
    AZ_TYPE_INFO_WITH_NAME_IMPL(DialogueComponentConfig, "DialogueComponentConfig", DialogueComponentConfigTypeId); // NOLINT
    AZ_CLASS_ALLOCATOR_IMPL(DialogueComponentConfig, AZ::SystemAllocator); // NOLINT

    void DialogueComponentConfig::Reflect(AZ::ReflectContext* context)
    {
        if (auto serialize = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serialize->Class<DialogueComponentConfig, AZ::ComponentConfig>()
                ->Version(4)
                ->Field("Display Name", &DialogueComponentConfig::m_displayName)
                ->Field("Speaker Icon", &DialogueComponentConfig::m_speakerIconPath)
                ->Field("Speaker Tag", &DialogueComponentConfig::m_speakerTag);

            if (auto editContext = serialize->GetEditContext())
            {
                editContext->Class<DialogueComponentConfig>("Dialogue Configuration", "")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                    ->Attribute(AZ::Edit::Attributes::Category, "BoP/Conversation")
                    ->Attribute(AZ::Edit::Attributes::AutoExpand, true)
                    ->DataElement(
                        AZ::Edit::UIHandlers::Default,
                        &DialogueComponentConfig::m_speakerTag,
                        "Speaker Tag",
                        "Identifies this entity as the owner of any dialogue containing this speaker tag.")
                    ->DataElement(AZ::Edit::UIHandlers::Default, &DialogueComponentConfig::m_displayName, "Display Name", "")
                    ->DataElement(AZ::Edit::UIHandlers::Default, &DialogueComponentConfig::m_speakerIconPath, "SpeakerIconPath", "")
                    ->Attribute(AZ::Edit::Attributes::Visibility, true);
            }
        }
    }

} // namespace Conversation
