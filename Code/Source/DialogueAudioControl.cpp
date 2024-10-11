#include "DialogueAudioControl.h"

#include "AzCore/RTTI/BehaviorContext.h"
#include "AzCore/Serialization/EditContext.h"
#include "AzCore/Serialization/EditContextConstants.inl"
#include "AzCore/Serialization/SerializeContext.h"
#include "Conversation/Constants.h"

namespace Conversation
{
    void DialogueAudioControl::Reflect(AZ::ReflectContext* context)
    {
        if (auto* serialize{ azrtti_cast<AZ::SerializeContext*>(context) })
        {
            serialize->Class<DialogueAudioControl>()->Version(0)->Field(
                "Control", &DialogueAudioControl::m_control);

            if (AZ::EditContext * edit{ serialize->GetEditContext() })
            {
                edit->Class<DialogueAudioControl>("Dialogue Audio Control", "")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                    ->Attribute(
                        AZ::Edit::Attributes::Visibility,
                        AZ::Edit::PropertyVisibility::ShowChildrenOnly)
                    ->DataElement(
                        AZ::Edit::UIHandlers::Default,
                        &DialogueAudioControl::m_control,
                        "AudioControlName",
                        "");
            }
        }

        if (auto* const behavior{ azrtti_cast<AZ::BehaviorContext*>(context) })
        {
            behavior
                ->Class<DialogueAudioControl>(
                    DialogueAudioControl::TYPEINFO_Name())
                ->Attribute(
                    AZ::Script::Attributes::Category, DialogueSystemCategory)
                ->Attribute(
                    AZ::Script::Attributes::Scope,
                    AZ::Script::Attributes::ScopeFlags::Common)
                ->Attribute(
                    AZ::Script::Attributes::Module, DialogueSystemModule)
                ->Attribute(AZ::Script::Attributes::ConstructibleFromNil, true)
                ->Attribute(
                    AZ::Script::Attributes::EnableAsScriptEventParamType, true)
                ->Attribute(
                    AZ::Script::Attributes::EnableAsScriptEventReturnType, true)
                ->Property(
                    "Name",
                    BehaviorValueGetter(&DialogueAudioControl::m_control),
                    BehaviorValueSetter(&DialogueAudioControl::m_control));
        }
    }
} // namespace Conversation
