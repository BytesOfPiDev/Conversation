#include "Tools/Document/ConversationGraphCompiler.h"

#include "AtomToolsFramework/Graph/DynamicNode/DynamicNode.h"
#include "AtomToolsFramework/Graph/DynamicNode/DynamicNodeUtil.h"
#include "AtomToolsFramework/Graph/GraphCompiler.h"
#include "AtomToolsFramework/Graph/GraphTemplateFileDataCacheRequestBus.h"
#include "AtomToolsFramework/Graph/GraphUtil.h"
#include "AtomToolsFramework/Util/Util.h"
#include "AzCore/Console/ILogger.h"
#include "AzCore/Debug/Trace.h"
#include "AzCore/Script/ScriptAsset.h"
#include "AzCore/Serialization/SerializeContext.h"
#include "AzCore/Utils/Utils.h"
#include "AzCore/std/ranges/ranges.h"
#include "AzCore/std/smart_ptr/shared_ptr.h"
#include "AzCore/std/string/regex.h"
#include "AzFramework/Asset/AssetSystemBus.h"
#include "AzToolsFramework/API/EditorAssetSystemAPI.h"
#include "Conversation/Constants.h"
#include "GraphModel/Model/Common.h"
#include "GraphModel/Model/Node.h"
#include "GraphModel/Model/Slot.h"
#include "cstdlib"

#include "Conversation/ConversationAsset.h"
#include "Conversation/ConversationTypeIds.h"
#include "Conversation/DialogueData_incl.h"
#include "Tools/DataTypes.h"
#include <optional>

namespace ConversationEditor::Nodes
{
    // We use forward declarations to avoid including the header files of each node.
    // Instead, I use a combination of templates and the type ID.
    // @todo Find a way to use just the type ID.

    class ActorDialogue;
    class RootNode;
} // namespace ConversationEditor::Nodes

namespace ConversationEditor
{

    [[maybe_unused]] constexpr auto SetActorText = [](Conversation::DialogueData& dialogueData, GraphModel::ConstSlotPtr const& slot)
    {
        if (!slot->GetParentNode()->RTTI_IsTypeOf(AZ::TypeId(ActorDialogueNodeTypeId)))
        {
            return;
        }

        if (slot->GetName() == ToString(NodeAndSlotNames::ActorDialogueProperty_Text))
        {
            SetDialogueActorText(dialogueData, slot->GetValue<AZStd::string>());
        }
    };

    ConversationGraphCompiler::ConversationGraphCompiler(AZ::Crc32 const& toolId)
        : AtomToolsFramework::GraphCompiler(toolId)
    {
    }

    void ConversationGraphCompiler::Reflect(AZ::ReflectContext* context)
    {
        if (auto* serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<ConversationGraphCompiler, AtomToolsFramework::GraphCompiler>()->Version(0);
        }
    }

    ConversationGraphCompiler::~ConversationGraphCompiler() = default;

    auto ConversationGraphCompiler::CompileGraph(GraphModel::GraphPtr graph, AZStd::string const& graphName, AZStd::string const& graphPath)
        -> bool
    {
        ClearData();

        if (!AtomToolsFramework::GraphCompiler::CompileGraph(graph, graphName, graphPath))
        {
            return false;
        }

        BuildSlotValueTable();
        AZLOG_INFO("Built slot value table."); // NOLINT

        BuildDependencyTables();
        AZLOG_INFO("Built dependency tables."); // NOLINT

        AZStd::vector<GraphModel::ConstNodePtr> const nodesInExecutionOrder = GetAllNodesInExecutionOrder();

        // Traverse all graph nodes and slots searching for settings to generate files from templates
        for (auto const& currentNode : nodesInExecutionOrder)
        {
            m_currentNode = currentNode;
            if (!m_currentNode)
            {
                AZLOG_FATAL("Current node is null!"); // NOLINT

                continue;
            }

            // Search this node for any template path settings that describe files that need to be generated from the graph.
            BuildTemplatePathsForCurrentNode(currentNode);

            // If no template files were specified for this node then skip additional processing and continue to the next one.
            if (GetTemplatePathsForCurrentNode().empty())
            {
                continue;
            }

            // Attempt to load all of the template files referenced by this node. All of the template data will be tokenized into
            // individual lines and stored in a container so then multiple passes can be made on each file, substituting tokens and
            // filling in details provided by the graph. None of the files generated from this node will be saved until they have all
            // been processed. Template files for material types will be processed in their own pass Because they require special
            // handling and need to be saved before material file templates to not trigger asset processor dependency errors.
            if (!LoadTemplatesForCurrentNode())
            {
                AZ_Error("ConversationGraphCompiler", false, "Compilation failed while loading templates for current node."); // NOLINT
                SetState(AtomToolsFramework::GraphCompiler::State::Failed);
                return false;
            }

            DeleteExistingFilesForCurrentNode();
            PreprocessTemplatesForCurrentNode();
            BuildInstructionsForCurrentNode(currentNode);

            if (!ExportTemplatesMatchingRegex(".*\\.lua\\b"))
            {
                AZ_Error("ConversationGraphCompiler", false, "Compilation failed. while trying to export '.lua' files."); // NOLINT
                SetState(AtomToolsFramework::GraphCompiler::State::Failed);
                return false;
            }
        };

        for (auto const& currentNode : nodesInExecutionOrder)
        {
            BuildNode(currentNode);
        }

        auto conversationAsset = AZStd::make_unique<Conversation::ConversationAsset>();

        AZStd::ranges::for_each(
            m_startingIds,
            [&conversationAsset](auto const& startingId) -> void
            {
                conversationAsset->AddStartingId(startingId);
            });

        for (auto& [node, nodeData] : m_nodeDataTable)
        {
            if (nodeData.m_dialogue.has_value())
            {
                AZStd::ranges::for_each(
                    nodeData.m_responseIds,
                    [&nodeData = nodeData](Conversation::DialogueId const& responseId) -> void
                    {
                        nodeData.m_dialogue->m_responseIds.push_back(responseId);
                    });
                AZ_Info("ConversationGraphCompiler", "Adding... %s'.\n", ToString(*nodeData.m_dialogue).c_str());
                conversationAsset->AddDialogue(*nodeData.m_dialogue);
            }
        }

        auto const generatedPath = AZStd::string::format( // NOLINT
            "%s/Assets/Conversations/Generated/%s.%s", AZ::Utils::GetProjectPath().c_str(), GetGraphName().data(),
            Conversation::ConversationAsset::SourceExtension);

        AZ_Info("ConversationGraphCompiler", "Saving compiled conversation graph to %s.\n", generatedPath.c_str()); // NOLINT
        AZ::Utils::SaveObjectToFile<Conversation::ConversationAsset>(generatedPath, AZ::DataStream::ST_JSON, conversationAsset.get());
        AZ_Info("ConversationGraphCompiler", "Saved compiled conversation graph to %s.\n", generatedPath.c_str()); // NOLINT

        if (!ExportTemplatesMatchingRegex(".*\\.conversation\\b"))
        {
            AZ_Error("ConversationGraphCompiler", false, "Compilation failed while exporting '.conversation' files."); // NOLINT
            SetState(AtomToolsFramework::GraphCompiler::State::Failed);
            return false;
        }

        if (!ReportGeneratedFileStatus())
        {
            AZ_Error("ConversationGraphCompiler", false, "Compilation failed while reporting generated file status."); // NOLINT
            SetState(AtomToolsFramework::GraphCompiler::State::Failed);
            return false;
        }

        SetState(AtomToolsFramework::GraphCompiler::State::Complete);

        AZLOG_INFO("CompileGraph success.\n"); // NOLINT
        return true;
    }

    auto ConversationGraphCompiler::GetSlotValueTable() const -> AZStd::map<GraphModel::ConstSlotPtr, AZStd::any> const&
    {
        return m_slotValueTable;
    }

    auto ConversationGraphCompiler::ModifySlotValueTable() -> AZStd::map<GraphModel::ConstSlotPtr, AZStd::any>&
    {
        return m_slotValueTable;
    }

    auto ConversationGraphCompiler::GetStartingIds() -> StartingIdContainer const&
    {
        return m_startingIds;
    }

    auto ConversationGraphCompiler::ModifyStartingIds() -> StartingIdContainer&
    {
        return m_startingIds;
    }

    auto ConversationGraphCompiler::GetTemplatePathsForCurrentNode() -> AZStd::set<AZStd::string> const&
    {
        return m_templatePathsForCurrentNode;
    }

    auto ConversationGraphCompiler::ModifyTemplatePathsForCurrentNode() -> AZStd::set<AZStd::string>&
    {
        return m_templatePathsForCurrentNode;
    }

    auto ConversationGraphCompiler::GetTemplateDataForCurrentNode() -> AZStd::list<AtomToolsFramework::GraphTemplateFileData> const&
    {
        return m_templateFileDataVecForCurrentNode;
    }

    auto ConversationGraphCompiler::ModifyTemplateDataForCurrentNode() -> AZStd::list<AtomToolsFramework::GraphTemplateFileData>&
    {
        return m_templateFileDataVecForCurrentNode;
    }

    auto ConversationGraphCompiler::ModifyInstructionNodesForCurrentNode() -> AZStd::vector<GraphModel::ConstNodePtr>&
    {
        return m_instructionNodesForCurrentNode;
    }

    void ConversationGraphCompiler::ClearInstructionsForCurrentNode(size_t reserveAmount)
    {
        ModifyInstructionNodesForCurrentNode().clear();
        m_instructionNodesForCurrentNode.reserve(reserveAmount);
    }

    void ConversationGraphCompiler::BuildDependencyTables()
    {
        if (!m_graph)
        {
            AZ_Error( // NOLINT
                "ConversationGraphCompiler", false, "Attempting to generate data from invalid graph object.");
            return;
        }

        for (auto const& nodePair : m_graph->GetNodes())
        {
            auto const& currentNode = nodePair.second;

            if (auto dynamicNode = azrtti_cast<AtomToolsFramework::DynamicNode const*>(currentNode.get()))
            {
                if (!m_configIdsVisited.contains(dynamicNode->GetConfig().m_id))
                {
                    m_configIdsVisited.insert(dynamicNode->GetConfig().m_id);
                    AtomToolsFramework::VisitDynamicNodeSettings(
                        dynamicNode->GetConfig(),
                        [&](AtomToolsFramework::DynamicNodeSettingsMap const& settings)
                        {
                            AtomToolsFramework::CollectDynamicNodeSettings(settings, "includePaths", m_includePaths);
                            AtomToolsFramework::CollectDynamicNodeSettings(settings, "classDefinitions", m_classDefinitions);
                            AtomToolsFramework::CollectDynamicNodeSettings(settings, "functionDefinitions", m_functionDefinitions);
                            AtomToolsFramework::CollectDynamicNodeSettings(settings, "conditionInstructions", m_conditionInstructions);
                        });
                }
            }
        }
    }

    [[nodiscard]] auto ConversationGraphCompiler::ShouldUseInstructionsFromInputNode(
        GraphModel::ConstNodePtr const& outputNode,
        GraphModel::ConstNodePtr const& inputNode,
        AZStd::vector<AZStd::string> const& inputSlotNames) const -> bool
    {
        if (inputNode == outputNode)
        {
            return true;
        }

        for (const auto& inputSlotName : inputSlotNames)
        {
            if (const auto slot = outputNode->GetSlot(inputSlotName))
            {
                if (slot->GetSlotDirection() == GraphModel::SlotDirection::Input)
                {
                    for (const auto& connection : slot->GetConnections())
                    {
                        AZ_Assert( // NOLINT
                            connection->GetSourceNode() != outputNode, "This should never be the source node on an input connection.");
                        AZ_Assert( // NOLINT
                            connection->GetTargetNode() == outputNode, "This should always be the target node on an input connection.");
                        if (connection->GetSourceNode() == inputNode || connection->GetSourceNode()->HasInputConnectionFromNode(inputNode))
                        {
                            return true;
                        }
                    }
                }
            }
        }

        return false;
    }

    [[nodiscard]] auto ConversationGraphCompiler::GetInstructionsFromSlot(
        GraphModel::ConstNodePtr const& node,
        const AtomToolsFramework::DynamicNodeSlotConfig& slotConfig,
        const AZStd::vector<AZStd::pair<AZStd::string, AZStd::string>>& substitutionSymbols) const -> AZStd::vector<AZStd::string>
    {
        AZStd::vector<AZStd::string> instructionsForSlot;

        auto slot = node->GetSlot(slotConfig.m_name);

        if (slot && (slot->GetSlotDirection() != GraphModel::SlotDirection::Output || !slot->GetConnections().empty()))
        {
            AtomToolsFramework::CollectDynamicNodeSettings(slotConfig.m_settings, "instructions", instructionsForSlot);

            AtomToolsFramework::ReplaceSymbolsInContainer(substitutionSymbols, instructionsForSlot);
            AtomToolsFramework::ReplaceSymbolsInContainer("SLOTNAME", GetSymbolNameFromSlot(slot), instructionsForSlot);
            AtomToolsFramework::ReplaceSymbolsInContainer("SLOTTYPE", GetLuaTypeFromSlot(slot), instructionsForSlot);
            AtomToolsFramework::ReplaceSymbolsInContainer("SLOTVALUE", GetLuaValueFromSlot(slot), instructionsForSlot);
        }

        return instructionsForSlot;
    }

    [[nodiscard]] auto ConversationGraphCompiler::GetInstructionNodesInExecutionOrder(
        GraphModel::ConstNodePtr const& outputNode, AZStd::vector<AZStd::string> const& inputSlotNames) const
        -> AZStd::vector<GraphModel::ConstNodePtr>
    {
        AZStd::vector<GraphModel::ConstNodePtr> nodes = GetAllNodesInExecutionOrder();
        AZStd::erase_if(
            nodes,
            [this, &outputNode, &inputSlotNames](const auto& node)
            {
                return !ShouldUseInstructionsFromInputNode(outputNode, node, inputSlotNames);
            });
        return nodes;
    }

    [[nodiscard]] auto ConversationGraphCompiler::GetSymbolNameFromNode(GraphModel::ConstNodePtr const& node) const -> AZStd::string
    {
        return AtomToolsFramework::GetSymbolNameFromText(AZStd::string::format("node%u_%s", node->GetId(), node->GetTitle()));
    }

    [[nodiscard]] auto ConversationGraphCompiler::GetSymbolNameFromSlot(GraphModel::ConstSlotPtr slot) const -> AZStd::string
    {
        bool allowNameSubstitution = true;
        if (auto dynamicNode = azrtti_cast<const AtomToolsFramework::DynamicNode*>(slot->GetParentNode().get()))
        {
            const auto& nodeConfig = dynamicNode->GetConfig();
            AtomToolsFramework::VisitDynamicNodeSlotConfigs(
                nodeConfig,
                [&](const AtomToolsFramework::DynamicNodeSlotConfig& slotConfig)
                {
                    if (slot->GetName() == slotConfig.m_name)
                    {
                        allowNameSubstitution = slotConfig.m_allowNameSubstitution;
                    }
                });
        }

        if (!allowNameSubstitution)
        {
            return slot->GetName();
        }

        if (slot->SupportsExtendability())
        {
            return AZStd::string::format(
                "%s_%s_%d", GetSymbolNameFromNode(slot->GetParentNode()).c_str(), slot->GetName().c_str(), slot->GetSlotSubId());
        }

        return AZStd::string::format("%s_%s", GetSymbolNameFromNode(slot->GetParentNode()).c_str(), slot->GetName().c_str());
    }

    [[nodiscard]] auto ConversationGraphCompiler::GetLuaTypeFromSlot(GraphModel::ConstSlotPtr const& slot) const -> AZStd::string
    {
        auto const& slotValue = GetValueFromSlot(slot);
        auto const& slotDataType = slot->GetGraphContext()->GetDataTypeForValue(slotValue);
        auto const& slotDataTypeName = slotDataType ? slotDataType->GetDisplayName() : AZStd::string{};

        return AZ::StringFunc::Equal(slotDataTypeName, "color") ? "float4" : slotDataTypeName;
    }

    [[nodiscard]] auto ConversationGraphCompiler::GetLuaValueFromSlot(GraphModel::ConstSlotPtr const& slot) const -> AZStd::string
    {
        const auto& slotValue = GetValueFromSlot(slot);

        // This code and some of these rules will be refactored and generalized after splitting this class into a document and builder
        // or compiler class. Once that is done, it will be easier to register types, conversions, substitutions with the system.
        for (const auto& connection : slot->GetConnections())
        {
            auto sourceSlot = connection->GetSourceSlot();
            auto targetSlot = connection->GetTargetSlot();
            if (targetSlot == slot)
            {
                // If there is an incoming connection to this slot, the name of the source slot from the incoming connection will be
                // used as part of the value for the slot. It must be cast to the correct vector type for generated code. These
                // conversions will be extended once the code generator is separated from the document class.
                const auto& sourceSlotValue = GetValueFromSlot(sourceSlot);
                const auto& sourceSlotSymbolName = GetSymbolNameFromSlot(sourceSlot);
                if (slotValue.is<AZ::Vector2>())
                {
                    if (sourceSlotValue.is<AZ::Vector3>() || sourceSlotValue.is<AZ::Vector4>() || sourceSlotValue.is<AZ::Color>())
                    {
                        return AZStd::string::format("(float2)%s", sourceSlotSymbolName.c_str());
                    }
                }
                if (slotValue.is<AZ::Vector3>())
                {
                    if (sourceSlotValue.is<AZ::Vector2>())
                    {
                        return AZStd::string::format("float3(%s, 0)", sourceSlotSymbolName.c_str());
                    }
                    if (sourceSlotValue.is<AZ::Vector4>() || sourceSlotValue.is<AZ::Color>())
                    {
                        return AZStd::string::format("(float3)%s", sourceSlotSymbolName.c_str());
                    }
                }
                if (slotValue.is<AZ::Vector4>() || slotValue.is<AZ::Color>())
                {
                    if (sourceSlotValue.is<AZ::Vector2>())
                    {
                        return AZStd::string::format("float4(%s, 0, 1)", sourceSlotSymbolName.c_str());
                    }
                    if (sourceSlotValue.is<AZ::Vector3>())
                    {
                        return AZStd::string::format("float4(%s, 1)", sourceSlotSymbolName.c_str());
                    }
                }
                return sourceSlotSymbolName;
            }
        }

        // If the slot's embedded value is being used then generate shader code to represent it. More generic options will be explored
        // to clean this code up, possibly storing numeric values in a two-dimensional floating point array with the layout
        // corresponding to most vector and matrix types.
        if (auto v = AZStd::any_cast<const AZ::Color>(&slotValue))
        {
            return AZStd::string::format("{%g, %g, %g, %g}", v->GetR(), v->GetG(), v->GetB(), v->GetA());
        }
        if (auto v = AZStd::any_cast<const AZ::Vector4>(&slotValue))
        {
            return AZStd::string::format("{%g, %g, %g, %g}", v->GetX(), v->GetY(), v->GetZ(), v->GetW());
        }
        if (auto v = AZStd::any_cast<const AZ::Vector3>(&slotValue))
        {
            return AZStd::string::format("{%g, %g, %g}", v->GetX(), v->GetY(), v->GetZ());
        }
        if (auto v = AZStd::any_cast<const AZ::Vector2>(&slotValue))
        {
            return AZStd::string::format("{%g, %g}", v->GetX(), v->GetY());
        }
        if (auto v = AZStd::any_cast<const AZStd::array<AZ::Vector2, 2>>(&slotValue))
        {
            const auto& value = *v;
            return AZStd::string::format("{%g, %g, %g, %g}", value[0].GetX(), value[0].GetY(), value[1].GetX(), value[1].GetY());
        }
        if (auto v = AZStd::any_cast<const AZStd::array<AZ::Vector3, 3>>(&slotValue))
        {
            const auto& value = *v;
            return AZStd::string::format(
                "{%g, %g, %g, %g, %g, %g, %g, %g, %g}", value[0].GetX(), value[0].GetY(), value[0].GetZ(), value[1].GetX(), value[1].GetY(),
                value[1].GetZ(), value[2].GetX(), value[2].GetY(), value[2].GetZ());
        }
        if (auto v = AZStd::any_cast<const AZStd::array<AZ::Vector4, 3>>(&slotValue))
        {
            const auto& value = *v;
            return AZStd::string::format(
                "{%g, %g, %g, %g, %g, %g, %g, %g, %g, %g, %g, %g}", value[0].GetX(), value[0].GetY(), value[0].GetZ(), value[0].GetW(),
                value[1].GetX(), value[1].GetY(), value[1].GetZ(), value[1].GetW(), value[2].GetX(), value[2].GetY(), value[2].GetZ(),
                value[2].GetW());
        }
        if (auto v = AZStd::any_cast<const AZStd::array<AZ::Vector4, 4>>(&slotValue))
        {
            const auto& value = *v;
            return AZStd::string::format(
                "{%g, %g, %g, %g, %g, %g, %g, %g, %g, %g, %g, %g, %g, %g, %g, %g}", value[0].GetX(), value[0].GetY(), value[0].GetZ(),
                value[0].GetW(), value[1].GetX(), value[1].GetY(), value[1].GetZ(), value[1].GetW(), value[2].GetX(), value[2].GetY(),
                value[2].GetZ(), value[2].GetW(), value[3].GetX(), value[3].GetY(), value[3].GetZ(), value[3].GetW());
        }
        if (auto v = AZStd::any_cast<const float>(&slotValue))
        {
            return AZStd::string::format("%g", *v);
        }
        if (auto v = AZStd::any_cast<const int>(&slotValue))
        {
            return AZStd::string::format("%i", *v);
        }
        if (auto v = AZStd::any_cast<const unsigned int>(&slotValue))
        {
            return AZStd::string::format("%u", *v);
        }
        if (auto v = AZStd::any_cast<const bool>(&slotValue))
        {
            return AZStd::string::format("%u", *v ? 1 : 0);
        }
        if (auto v = AZStd::any_cast<AZStd::string const>(&slotValue))
        {
            return AZStd::string::format("%s", (*v).c_str());
        }
        if (auto const& v = AZStd::any_cast<Conversation::DialogueId const>(&slotValue))
        {
            return AZStd::string::format("\"%s\"", ToString(Conversation::DialogueId{ GetSymbolNameFromSlot(slot) }).c_str());
        }
        if (auto v = AZStd::any_cast<AZ::Data::Asset<AZ::ScriptAsset> const>(&slotValue))
        {
            return AZStd::string::format("\"%s\"", (*v).GetId().ToFixedString().c_str());
        }
        if (auto const& v = AZStd::any_cast<Conversation::DialogueData const>(&slotValue))
        {
            return AZStd::string::format("DialogueData()");
        }

        return {};
    }

    [[nodiscard]] auto ConversationGraphCompiler::GetSubstitutionSymbolsFromNode(GraphModel::ConstNodePtr const& node) const
        -> AZStd::vector<AZStd::pair<AZStd::string, AZStd::string>>
    {
        AZStd::vector<AZStd::pair<AZStd::string, AZStd::string>> substitutionSymbols;

        // Reserving space for the number of elements added in this function.
        substitutionSymbols.reserve(node->GetSlots().size() * 4 + 1);
        substitutionSymbols.emplace_back("NODEID", GetSymbolNameFromNode(node));

        for (const auto& slotPair : node->GetSlots())
        {
            const auto& slot = slotPair.second;

            // These substitutions will allow accessing the slot ID, type, value from anywhere in the node's shader code.
            substitutionSymbols.emplace_back(AZStd::string::format("SLOTTYPE\\(%s\\)", slot->GetName().c_str()), GetLuaTypeFromSlot(slot));
            substitutionSymbols.emplace_back(
                AZStd::string::format("SLOTVALUE\\(%s\\)", slot->GetName().c_str()), GetLuaValueFromSlot(slot));
            substitutionSymbols.emplace_back(
                AZStd::string::format("SLOTNAME\\(%s\\)", slot->GetName().c_str()), GetSymbolNameFromSlot(slot));

            // This expression will allow direct substitution of node variable names in node configurations with the decorated
            // symbol name. It will match whole words only. No additional decoration should be required on the node configuration
            // side. However, support for the older slot type, name, value substitutions are still supported as a convenience.
            substitutionSymbols.emplace_back(AZStd::string::format("\\b%s\\b", slot->GetName().c_str()), GetSymbolNameFromSlot(slot));
        }

        return substitutionSymbols;
    }

    auto ConversationGraphCompiler::GetInstructionsFromConnectedNodes(
        GraphModel::ConstNodePtr const& outputNode,
        AZStd::vector<AZStd::string> const& inputSlotNames,
        AZStd::vector<GraphModel::ConstNodePtr>& instructionNodes) const -> AZStd::vector<AZStd::string>
    {
        AZStd::vector<AZStd::string> instructions;

        // TODO: Remove once regular GraphModel::Node's are supported.
        if (!azrtti_istypeof<AtomToolsFramework::DynamicNode>(outputNode.get()))
        {
            return {};
        }

        for (auto const& inputNode : GetInstructionNodesInExecutionOrder(outputNode, inputSlotNames))
        {
            // Build a list of all nodes that will contribute instructions for the output node
            if (AZStd::find(instructionNodes.begin(), instructionNodes.end(), inputNode) == instructionNodes.end())
            {
                instructionNodes.push_back(inputNode);
            }

            if (auto const* const dynamicNode = azrtti_cast<AtomToolsFramework::DynamicNode const*>(inputNode.get()))
            {
                auto const& nodeConfig = dynamicNode->GetConfig();
                auto const& substitutionSymbols = GetSubstitutionSymbolsFromNode(inputNode);

                // Instructions are gathered separately for all of the slot categories because they need to be added in a specific
                // order.

                // Gather and perform substitutions on instructions embedded directly in the node.
                AZStd::vector<AZStd::string> instructionsForNode;
                AtomToolsFramework::CollectDynamicNodeSettings(nodeConfig.m_settings, NodeSettings::InstructionKey, instructionsForNode);

                AtomToolsFramework::ReplaceSymbolsInContainer(substitutionSymbols, instructionsForNode);

                // Gather and perform substitutions on instructions contained in property slots.
                AZStd::vector<AZStd::string> instructionsForPropertySlots;
                for (auto const& slotConfig : nodeConfig.m_propertySlots)
                {
                    auto const& instructionsForSlot = GetInstructionsFromSlot(inputNode, slotConfig, substitutionSymbols);
                    instructionsForPropertySlots.insert(
                        instructionsForPropertySlots.end(), instructionsForSlot.begin(), instructionsForSlot.end());
                }

                // Gather and perform substitutions on instructions contained in input slots.
                AZStd::vector<AZStd::string> instructionsForInputSlots;
                for (auto const& slotConfig : nodeConfig.m_inputSlots)
                {
                    // If this is the output node, only gather instructions for requested input slots.
                    if (inputNode == outputNode &&
                        AZStd::find(inputSlotNames.begin(), inputSlotNames.end(), slotConfig.m_name) == inputSlotNames.end())
                    {
                        continue;
                    }

                    auto const& instructionsForSlot = GetInstructionsFromSlot(inputNode, slotConfig, substitutionSymbols);
                    instructionsForInputSlots.insert(
                        instructionsForInputSlots.end(), instructionsForSlot.begin(), instructionsForSlot.end());
                }

                // Gather and perform substitutions on instructions contained in output slots.
                AZStd::vector<AZStd::string> instructionsForOutputSlots;
                for (auto const& slotConfig : nodeConfig.m_outputSlots)
                {
                    auto const instructionsForSlot = GetInstructionsFromSlot(inputNode, slotConfig, substitutionSymbols);
                    instructionsForOutputSlots.insert(
                        instructionsForOutputSlots.end(), instructionsForSlot.begin(), instructionsForSlot.end());
                }

                instructions.insert(instructions.end(), instructionsForPropertySlots.begin(), instructionsForPropertySlots.end());
                instructions.insert(instructions.end(), instructionsForInputSlots.begin(), instructionsForInputSlots.end());
                instructions.insert(instructions.end(), instructionsForNode.begin(), instructionsForNode.end());
                instructions.insert(instructions.end(), instructionsForOutputSlots.begin(), instructionsForOutputSlots.end());
            }
        }

        return instructions;
    }

    void ConversationGraphCompiler::BuildInstructionsForCurrentNode(GraphModel::ConstNodePtr const& currentNode)
    {
        AZLOG_INFO( // NOLINT
            "Building instructions for node '%s'.\n", GetSymbolNameFromNode(currentNode).c_str());

        ClearInstructionsForCurrentNode(m_graph->GetNodeCount());

        AZ::parallel_for_each(
            m_templateFileDataVecForCurrentNode.begin(), m_templateFileDataVecForCurrentNode.end(),
            [&](auto& templateFileData)
            {
                templateFileData.ReplaceLinesInBlock(
                    "BOP_GENERATED_INSTRUCTIONS_BEGIN", "BOP_GENERATED_INSTRUCTIONS_END",
                    [&]([[maybe_unused]] AZStd::string const& blockHeader)
                    {
                        AZStd::vector<AZStd::string> inputSlotNames;
                        AZ::StringFunc::Tokenize(blockHeader, inputSlotNames, ";:, \t\r\n\\/", false, false);

                        AZStd::vector<GraphModel::ConstNodePtr> instructionNodesForBlock;
                        instructionNodesForBlock.reserve(m_graph->GetNodeCount());
                        auto const& lines = GetInstructionsFromConnectedNodes(currentNode, inputSlotNames, instructionNodesForBlock);

                        // Adding all of the contributing notes from this block to the set of all nodes for all blocks.
                        AZStd::scoped_lock lock(m_instructionNodesForCurrentNodeMutex);
                        m_instructionNodesForCurrentNode.insert(
                            m_instructionNodesForCurrentNode.end(), instructionNodesForBlock.begin(), instructionNodesForBlock.end());

                        return lines;
                    });
            });
    }

    auto ConversationGraphCompiler::GetVectorSize(const AZStd::any& slotValue) const -> unsigned int
    {
        if (slotValue.is<AZ::Color>())
        {
            return 4;
        }
        if (slotValue.is<AZ::Vector4>())
        {
            return 4;
        }
        if (slotValue.is<AZ::Vector3>())
        {
            return 3;
        }
        if (slotValue.is<AZ::Vector2>())
        {
            return 2;
        }
        if (slotValue.is<bool>() || slotValue.is<int>() || slotValue.is<unsigned int>() || slotValue.is<float>())
        {
            return 1;
        }
        return 0;
    }

    [[nodiscard]] auto ConversationGraphCompiler::GetValueFromSlot(GraphModel::ConstSlotPtr const slot) const -> AZStd::any
    {
        const auto& slotItr = AZStd::ranges::find_if(
            GetSlotValueTable(),
            [slot](auto const& key) -> bool
            {
                return key.first == slot;
            });

        return slotItr != GetSlotValueTable().end() ? slotItr->second : slot->GetValue();
    }

    [[nodiscard]] auto ConversationGraphCompiler::GetValueFromSlotOrConnection(GraphModel::ConstSlotPtr const& slot) const -> AZStd::any
    {
        for (const auto& connection : slot->GetConnections())
        {
            auto sourceSlot = connection->GetSourceSlot();
            auto targetSlot = connection->GetTargetSlot();
            if (targetSlot == slot)
            {
                return GetValueFromSlotOrConnection(sourceSlot);
            }
        }

        return GetValueFromSlot(slot);
    }

    [[nodiscard]] auto ConversationGraphCompiler::GetAllNodesInExecutionOrder() const -> AZStd::vector<GraphModel::ConstNodePtr>
    {
        AZStd::vector<GraphModel::ConstNodePtr> nodes{};

        if (m_graph)
        {
            nodes.reserve(m_graph->GetNodes().size());
            for (const auto& nodePair : m_graph->GetNodes())
            {
                nodes.push_back(nodePair.second);
            }

            AtomToolsFramework::SortNodesInExecutionOrder(nodes);
        }

        return nodes;
    }

    void ConversationGraphCompiler::BuildNode(GraphModel::ConstNodePtr const& currentNode)
    {
        using namespace Conversation;

        if (!m_graph)
        {
            return;
        }
        if (auto const dynamicNode = azrtti_cast<AtomToolsFramework::DynamicNode const*>(currentNode.get()))
        {
            auto const& dynamicNodeSettings = dynamicNode->GetConfig().m_settings;

            bool const hasNodeTypeSetting = [&dynamicNodeSettings]() -> bool
            {
                return (dynamicNodeSettings.contains(NodeSettings::NodeTypeKey));
            }();

            bool const isDialogueNode = [&hasNodeTypeSetting, &dynamicNodeSettings]() -> bool
            {
                return hasNodeTypeSetting &&
                    AZStd::ranges::contains(dynamicNodeSettings.find(NodeSettings::NodeTypeKey)->second, "Dialogue");
            }();

            if (isDialogueNode)
            {
                BuildDialogueNode(currentNode);
            }
        }
    }

    void ConversationGraphCompiler::BuildDialogueNode(const GraphModel::ConstNodePtr& currentNode)
    {
        using namespace Conversation;

        if (!currentNode)
        {
            return;
        }

        DialogueId currentNodeDialogueId{ GetSymbolNameFromNode(currentNode) };
        if (!m_nodeDataTable[currentNode].m_dialogue.has_value())
        {
            m_nodeDataTable[currentNode].m_dialogue.emplace(currentNodeDialogueId);
        }

        auto& nodeDataDialogue = m_nodeDataTable[currentNode].m_dialogue;

        AZ_Error( // NOLINT
            "ConversationGraphCompiler", currentNodeDialogueId == nodeDataDialogue->m_id,
            "The Id we generated and the one stored in the dialogue should be the same!");

        nodeDataDialogue->m_availabilityId = AZ::Name{ GetSymbolNameFromNode(currentNode) };

        for (auto& [slotId, slot] : currentNode->GetSlots())
        {
            auto const value = m_slotValueTable[slot];

            switch (slot->GetDataType()->GetTypeEnum())
            {
            case AZ_CRC_CE("actor_text"):
                m_nodeDataTable[currentNode].m_dialogue->m_actorText = AZStd::any_cast<AZStd::string>(value);
                m_chunks.insert(DialogueChunk{ m_nodeDataTable[currentNode].m_dialogue->m_actorText });
                break;
            case AZ_CRC_CE("speaker_tag"):
                m_nodeDataTable[currentNode].m_dialogue->m_speaker = AZStd::any_cast<AZStd::string>(value);
                break;
            default:
                break;
            }
        }

        // Is this dialogue a starting dialogue?
        auto const isStartingDialogue = [&currentNode]() -> bool
        {
            auto const inputSlot_isStarter = currentNode->GetSlot(DialogueNodeIsStarterSlotName);
            return (inputSlot_isStarter && inputSlot_isStarter->GetValue().is<bool>()) //
                ? inputSlot_isStarter->GetValue<bool>()
                : false;
        }();

        if (isStartingDialogue)
        {
            m_startingIds.insert(m_nodeDataTable[currentNode].m_dialogue->m_id);
        }

        auto const currentNodeHasParent = [&currentNode]() -> bool
        {
            GraphModel::ConstSlotPtr currentNodeParentInputSlot = currentNode->GetSlot(DialogueNodeParentSlotName);
            return currentNodeParentInputSlot && !currentNodeParentInputSlot->GetConnections().empty();
        }();

        if (currentNodeHasParent)
        {
            auto const parentNode = [&currentNode]() -> GraphModel::ConstNodePtr
            {
                auto const connections = currentNode->GetSlot(DialogueNodeParentSlotName)->GetConnections();
                if (connections.size() != 1)
                {
                    AZLOG_ERROR( // NOLINT
                        "There should be exactly one connection on the current node's parent input. We found: %lu.", connections.size());
                    return nullptr;
                }

                GraphModel::ConstConnectionPtr const connection = *connections.begin();
                if (connection->GetSourceNode() != currentNode && connection->GetTargetNode() == currentNode)
                {
                    return connection->GetSourceNode();
                }

                AZLOG_ERROR( // NOLINT
                    "Something is wrong with the connection between the current node and the parent node. Returng nullptr.");

                return nullptr;
            }();

            if (parentNode)
            {
                m_nodeDataTable[parentNode].m_responseIds.push_back(currentNodeDialogueId);
            }
        }

        /*
                // If the parent slot exists, then we need to get the
                // DialogueData of the parent dialogue so we can add the current
                // node's DialogueId as a response.
                AZLOG_INFO("Checking if we're connected to a parent node ...");
                if (GraphModel::ConstSlotPtr const currentNodeParentSlot = currentNode->GetSlot(DialogueNodeParentSlotName))
                {
                    if (currentNodeParentSlot->GetConnections().empty())
                    {
                        AZLOG_INFO("Yes, we're connected to a parent node.");
                    }
                    else
                    {
                        AZLOG_INFO("No, we're not connected to a parent node.");
                    }
                    auto count = 0;
                    // FIXME: Doesn't add responses
                    AZStd::ranges::for_each(
                        currentNodeParentSlot->GetConnections(),
                        [this, &currentNode, &currentNodeParentSlot, &count](auto const& connection) -> void
                        {
                            ++count;
                            AZLOG_INFO("InParent. Connection Count: %i.\n", count); // NOLINT
                            // Exit if the source node is the current node.
                            if (!(connection->GetSourceNode() != currentNode))
                            {
                                AZLOG_ERROR("The source node should not also be the current node.\n"); // NOLINT
                                AZLOG_INFO("GetSourceNode error.");
                                return;
                            }
                            // Exit if the target slot is not the current node's parent slot.
                            if (!(connection->GetTargetSlot() == currentNodeParentSlot))
                            {
                                AZLOG_ERROR("The target slot slot should be the same as the current node's 'parent' slot.\n"); // NOLINT
                                AZLOG_INFO("GetTargetSlot error.");
                                return;
                            }

                            auto& sourceNodeData = m_nodeDataTable[connection->GetSourceNode()];

                            // We create a response using the ID of each node.
                            if (auto& sourceOptionalParentDialogue = sourceNodeData.m_dialogue)
                            {
                                DialogueId const sourceParentDialogueId{ GetSymbolNameFromNode(connection->GetSourceNode()) };

                                if (sourceOptionalParentDialogue->m_id.IsNull())
                                {
                                    sourceOptionalParentDialogue->m_id = DialogueId{ GetSymbolNameFromNode(connection->GetSourceNode()) };
                                }
                                DialogueId const targetResponseDialogueId{ GetSymbolNameFromNode(currentNode) };
                                sourceOptionalParentDialogue = DialogueData{ sourceParentDialogueId };
                            }
                        });
                }
                else
                {
                    AZLOG_ERROR(
                        "... no, we're not connected to a parent node. CurrentNode '%s' does not have '%s' slot.",
                        GetSymbolNameFromNode(currentNode).c_str(), DialogueNodeParentSlotName);
                }
                */
    }

    void ConversationGraphCompiler::BuildSlotValueTable()
    {
        // Build a table of all values for every slot in the graph.
        ModifySlotValueTable().clear();

        AZStd::ranges::for_each(
            GetAllNodesInExecutionOrder(),
            [this](auto const& currentNode) -> void
            {
                AZStd::ranges::for_each(
                    currentNode->GetSlots(),
                    [this](auto const& currentSlotPair)
                    {
                        auto const& currentSlot = currentSlotPair.second;
                        ModifySlotValueTable()[currentSlot] = currentSlot->GetValue();
                    });

                // If this is a dynamic node with slot data type groups, we will search for the largest vector or other data type
                // and convert all of the values in the group to the same type.
                if (auto dynamicNode = azrtti_cast<const AtomToolsFramework::DynamicNode*>(currentNode.get()))
                {
                    const auto& nodeConfig = dynamicNode->GetConfig();
                    for (const auto& slotDataTypeGroup : nodeConfig.m_slotDataTypeGroups)
                    {
                        unsigned int vectorSize = 0;

                        // The slot data group string is separated by vertical bars and can be treated like a regular expression to
                        // compare against slot names. The largest vector size is recorded for each slot group.
                        const AZStd::regex slotDataTypeGroupRegex(slotDataTypeGroup, AZStd::regex::flag_type::icase);
                        for (const auto& currentSlotPair : currentNode->GetSlots())
                        {
                            const auto& currentSlot = currentSlotPair.second;
                            if (currentSlot->GetSlotDirection() == GraphModel::SlotDirection::Input &&
                                AZStd::regex_match(currentSlot->GetName(), slotDataTypeGroupRegex))
                            {
                                const auto& currentSlotValue = GetValueFromSlotOrConnection(currentSlot);
                                vectorSize = AZStd::max(vectorSize, GetVectorSize(currentSlotValue));
                            }
                        }

                        // Once all of the container sizes have been recorded for each slot data group, iterate over all of these
                        // slot values and upgrade entries in the map to the bigger type.
                        for (const auto& currentSlotPair : currentNode->GetSlots())
                        {
                            const auto& currentSlot = currentSlotPair.second;
                            if (AZStd::regex_match(currentSlot->GetName(), slotDataTypeGroupRegex))
                            {
                                const auto& currentSlotValue = GetValueFromSlot(currentSlot);
                                ModifySlotValueTable()[currentSlot] = ConvertToVector(currentSlotValue, vectorSize);
                            }
                        }
                    }
                }
            });
    }

    /*     void ConversationGraphCompiler::BuildDialogueNodeData()
        {
            AZStd::ranges::for_each(
                GetSlotValueTable(),
                [this](auto const& currentSlotPair)
                {
                    GraphModel::ConstSlotPtr currentSlot = currentSlotPair.first;
                    GraphModel::ConstNodePtr parentNode = currentSlot->GetParentNode();
                    if (!currentSlot)
                    {
                        // This should never happen. If it does, we're in a situation we don't want to recover from.
                        AZ_Fatal( // NOLINT
                            "ConversationGraphCompiler", "Unexpected null slot pointer while building dialogue node.\n");
                        exit(EXIT_FAILURE);
                    }

                    // We only handle actor dialogue nodes.
                    if (!parentNode->RTTI_IsTypeOf(AZ::TypeId(ActorDialogueNodeTypeId)))
                    {
                        return;
                    }

                    // The node's name is supposed to be the string version of one of the members in NodeAndSlotNames.
                    AZStd::optional<NodeAndSlotNames> const slotName =
                        NodeAndSlotNamesNamespace::FromStringToNodeAndSlotNames(currentSlot->GetName());

                    // If we didn't get anything back, or the value isn't in our helper list, then we have a slot we cannot currently
                    // handle.
                    if (!slotName || !m_dialogueNodeCompilerHelpers.contains(*slotName))
                    {
                        AZLOG_WARN( // NOLINT
                            "While consuming a dialogue node's slot, we encountered a slot named '%s' that we don't know how to "
                            "handle.\n",
                            currentSlot->GetName().c_str());
                        return;
                    }

                    Conversation::DialogueData& dialogueData = ModifyNodeIdToDialogueMap()[parentNode->GetId()];

                    m_dialogueNodeCompilerHelpers[*slotName](dialogueData, currentSlot);
                });
        } */

    auto ConversationGraphCompiler::ConvertToScalar(const AZStd::any& slotValue) const -> AZStd::any
    {
        if (auto v = AZStd::any_cast<const AZ::Color>(&slotValue))
        {
            return AZStd::any(v->GetR());
        }
        if (auto v = AZStd::any_cast<const AZ::Vector4>(&slotValue))
        {
            return AZStd::any(v->GetX());
        }
        if (auto v = AZStd::any_cast<const AZ::Vector3>(&slotValue))
        {
            return AZStd::any(v->GetX());
        }
        if (auto v = AZStd::any_cast<const AZ::Vector2>(&slotValue))
        {
            return AZStd::any(v->GetX());
        }
        return slotValue;
    }

    [[nodiscard]] auto ConversationGraphCompiler::ConvertToVector(const AZStd::any& slotValue, unsigned int score) const -> AZStd::any
    {
        switch (score)
        {
        case 4:
            // Skipping color to vector conversions so that they export as the correct type with the material type.
            return slotValue.is<AZ::Color>() ? slotValue : ConvertToVector<AZ::Vector4>(slotValue);
        case 3:
            // Skipping color to vector conversions so that they export as the correct type with the material type.
            return slotValue.is<AZ::Color>() ? slotValue : ConvertToVector<AZ::Vector3>(slotValue);
        case 2:
            return ConvertToVector<AZ::Vector2>(slotValue);
        case 1:
            return ConvertToScalar(slotValue);
        default:
            return slotValue;
        }
    }

    void ConversationGraphCompiler::BuildTemplatePathsForCurrentNode(GraphModel::ConstNodePtr const& currentNode)
    {
        ModifyTemplatePathsForCurrentNode().clear();

        auto dynamicNode = azrtti_cast<AtomToolsFramework::DynamicNode const*>(currentNode.get());
        if (!dynamicNode)
        {
            return;
        }

        AtomToolsFramework::VisitDynamicNodeSettings(
            dynamicNode->GetConfig(),
            [&](const AtomToolsFramework::DynamicNodeSettingsMap& settings)
            {
                AtomToolsFramework::CollectDynamicNodeSettings(settings, "templatePaths", ModifyTemplatePathsForCurrentNode());
            });
    }

    auto ConversationGraphCompiler::LoadTemplatesForCurrentNode() -> bool
    {
        m_templateFileDataVecForCurrentNode.clear();

        for (const auto& templatePath : m_templatePathsForCurrentNode)
        {
            AZLOG_INFO( // NOLINT
                "Template path: %s.\n", templatePath.data());

            if (templatePath.ends_with(".lua"))
            {
                // Load the unmodified, template source file data, which will be copied and used for insertions, substitutions, and
                // code generation.
                AtomToolsFramework::GraphTemplateFileData templateFileData;
                AtomToolsFramework::GraphTemplateFileDataCacheRequestBus::EventResult(
                    templateFileData, m_toolId, &AtomToolsFramework::GraphTemplateFileDataCacheRequestBus::Events::Load,
                    AtomToolsFramework::GetPathWithoutAlias(templatePath));

                if (!templateFileData.IsLoaded())
                {
                    m_templateFileDataVecForCurrentNode.clear();
                    return false;
                }

                m_templateFileDataVecForCurrentNode.emplace_back(AZStd::move(templateFileData));
            }
        };
        return true;
    }

    [[nodiscard]] auto ConversationGraphCompiler::GetOutputPathFromTemplatePath(AZStd::string const& templateInputPath) const
        -> AZStd::string
    {
        AZStd::string templateInputFileName;
        AZ::StringFunc::Path::GetFullFileName(templateInputPath.c_str(), templateInputFileName);

        AZStd::string templateOutputPath = GetGraphPath();
        AZ::StringFunc::Path::ReplaceFullName(templateOutputPath, templateInputFileName.c_str());

        AZ::StringFunc::Replace(templateOutputPath, "ConversationGraphName", GetUniqueGraphName().c_str());
        AZ::StringFunc::Replace(templateOutputPath, "ConversationGraphConditionNode", GetSymbolNameFromNode(m_currentNode).c_str());
        return templateOutputPath;
    }

    void ConversationGraphCompiler::DeleteExistingFilesForCurrentNode()
    {
        if (AtomToolsFramework::GetSettingsValue("/O3DE/Atom/ConversationCanvas/ForceDeleteGeneratedFiles", true))
        {
            AZLOG_INFO( // NOLINT
                "Deleting generated files.\n");
            AZ::parallel_for_each(
                ModifyTemplateDataForCurrentNode().begin(), ModifyTemplateDataForCurrentNode().end(),
                [this](const auto& templateFileData)
                {
                    auto const& templateInputPath = AtomToolsFramework::GetPathWithoutAlias(templateFileData.GetPath());
                    auto const& templateOutputPath = GetOutputPathFromTemplatePath(templateInputPath);

                    auto fileIO = AZ::IO::FileIOBase::GetInstance();
                    fileIO->Remove(templateOutputPath.c_str());
                });

            AZLOG_INFO( // NOLINT
                "Finished deleting generated files.\n");
        }
    }

    void ConversationGraphCompiler::PreprocessTemplatesForCurrentNode()
    {
        AZ::parallel_for_each(
            ModifyTemplateDataForCurrentNode().begin(), ModifyTemplateDataForCurrentNode().end(),

            [&](auto& templateFileData)
            {
                // Substitute all references to the placeholder graph name with one generated from the document name
                templateFileData.ReplaceSymbol("ConversationGraphName", GetUniqueGraphName());

                // Inject include files found while traversing the graph into any include file blocks in the template.
                templateFileData.ReplaceLinesInBlock(
                    "BOP_GENERATED_INCLUDES_BEGIN", "BOP_GENERATED_INCLUDES_END",
                    [&, this]([[maybe_unused]] const AZStd::string& blockHeader)
                    {
                        // Include file paths will need to be converted to include statements.
                        AZStd::vector<AZStd::string> includeStatements;
                        includeStatements.reserve(m_includePaths.size());

                        for (const auto& path : m_includePaths)
                        {
                            bool relativePathFound = false;
                            AZStd::string relativePath;
                            AZStd::string relativePathFolder;

                            AzToolsFramework::AssetSystemRequestBus::BroadcastResult(
                                relativePathFound, &AzToolsFramework::AssetSystem::AssetSystemRequest::GenerateRelativeSourcePath,
                                AtomToolsFramework::GetPathWithoutAlias(path), relativePath, relativePathFolder);

                            if (relativePathFound)
                            {
                                includeStatements.push_back(AZStd::string::format("require <%s>", relativePath.c_str()));
                            }
                        }
                        return includeStatements;
                    });

                // Inject class definitions found while traversing the graph.
                templateFileData.ReplaceLinesInBlock(
                    "BOP_GENERATED_CLASSES_BEGIN", "BOP_GENERATED_CLASSES_END",
                    [&]([[maybe_unused]] const AZStd::string& blockHeader)
                    {
                        return m_classDefinitions;
                    });

                // Inject function definitions found while traversing the graph.
                templateFileData.ReplaceLinesInBlock(
                    "BOP_GENERATED_FUNCTIONS_BEGIN", "BOP_GENERATED_FUNCTIONS_END",
                    [&]([[maybe_unused]] const AZStd::string& blockHeader)
                    {
                        return m_functionDefinitions;
                    });

                // Inject function definitions found while traversing the graph.
                templateFileData.ReplaceLinesInBlock(
                    "BOP_GENERATED_CONDITION_BEGIN", "BOP_GENERATED_CONDITION_END",
                    [&]([[maybe_unused]] const AZStd::string& blockHeader)
                    {
                        return m_conditionInstructions;
                    });
            });
    }

    [[nodiscard]] auto ConversationGraphCompiler::GetUniqueGraphName() const -> AZStd::string
    {
        return m_templateNodeCount <= 0 ? GetGraphName().data()
                                        : AZStd::string::format("%s_%03i", GetGraphName().data(), m_templateNodeCount);
    }

    auto ConversationGraphCompiler::ExportTemplatesMatchingRegex(AZStd::string const& pattern) -> bool
    {
        const AZStd::regex patternRegex(pattern, AZStd::regex::flag_type::icase);
        for (const auto& templateFileData : m_templateFileDataVecForCurrentNode)
        {
            if (AZStd::regex_match(templateFileData.GetPath(), patternRegex))
            {
                const auto& templateOutputPath = GetOutputPathFromTemplatePath(templateFileData.GetPath());

                if (!templateFileData.Save(templateOutputPath))
                {
                    AZLOG_ERROR( // NOLINT
                        "Export failed!\n");
                    return false;
                }

                AzFramework::AssetSystemRequestBus::Broadcast(
                    &AzFramework::AssetSystem::AssetSystemRequests::EscalateAssetBySearchTerm, templateOutputPath);
                m_generatedFiles.push_back(templateOutputPath);
            }
        }
        return true;
    }

    void ConversationGraphCompiler::ClearData()
    {
        m_instructionNodesForCurrentNode.clear();
        m_currentNode = nullptr;
        m_generatedFiles.clear();
        m_includePaths.clear();
        m_classDefinitions.clear();
        m_functionDefinitions.clear();
        m_conditionInstructions.clear();
        m_slotValueTable.clear();
        m_slotDialogueTable.clear();
        m_slotConditionTable.clear();
        m_startingIds.clear();
        m_nodeDataTable.clear();
        m_templateFileDataVecForCurrentNode.clear();
        m_configIdsVisited.clear();
        m_configIdsVisited.clear();
        m_templateNodeCount = 0;
    }

} // namespace ConversationEditor
