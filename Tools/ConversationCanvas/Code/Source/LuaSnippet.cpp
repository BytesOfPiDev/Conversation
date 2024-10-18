#include "LuaSnippet.h"

#include "AzCore/Serialization/EditContext.h"
#include "AzCore/Serialization/SerializeContext.h"

namespace ConversationCanvas
{
    void LuaSnippet::Reflect(AZ::ReflectContext* context)
    {
        if (auto* const serialize{
                azrtti_cast<AZ::SerializeContext*>(context) })
        {
            serialize->Class<LuaSnippet>()->Version(0)->Field(
                "SourceCode", &LuaSnippet::m_code);

            if (AZ::EditContext* const edit{ serialize->GetEditContext() })
            {
                edit->Class<LuaSnippet>("Lua Snippet", "Lines of lua code")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                    ->DataElement(
                        AZ::Edit::UIHandlers::MultiLineEdit,
                        &LuaSnippet::m_code,
                        "Code",
                        "");
            }
        }
    }
} // namespace ConversationCanvas