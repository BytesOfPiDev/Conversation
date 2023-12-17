#pragma once

#include "AzCore/std/optional.h"
#include "Conversation/DialogueData.h"
#include "GraphModel/Model/Common.h"

namespace ConversationEditor
{
    struct LinkData
    {
        GraphModel::ConstNodePtr m_from;
        GraphModel::ConstNodePtr m_to;
    };

    struct NodeData
    {
        AZStd::optional<Conversation::DialogueData> m_dialogue;
        AZStd::vector<AZ::Name> m_conditions;
        AZStd::vector<Conversation::DialogueId> m_responseIds;
        LinkData m_linkData;
    };
} // namespace ConversationEditor
