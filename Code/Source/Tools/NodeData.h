#pragma once

#include "AzCore/std/optional.h"
#include "Conversation/DialogueData.h"
#include "GraphModel/Model/Node.h"

namespace ConversationEditor
{
    struct LinkData
    {
        [[nodiscard]] auto IsValid() const
        {
            return (m_from && m_from->HasConnections()) &&
                (m_to && m_to->HasConnections());
        }

        GraphModel::ConstNodePtr m_from;
        GraphModel::ConstNodePtr m_to;
    };

    struct NodeData
    {
        AZStd::optional<Conversation::DialogueData> m_dialogue;
        AZStd::vector<AZ::Name> m_conditions;
        AZStd::vector<Conversation::UniqueId> m_responseIds;
        LinkData m_linkData;
    };
} // namespace ConversationEditor
