#include "Tools/Document/ConversationGraphCompiler.h"

#include "AtomToolsFramework/Graph/DynamicNode/DynamicNode.h"
#include "AtomToolsFramework/Graph/DynamicNode/DynamicNodeUtil.h"
#include "AtomToolsFramework/Graph/GraphCompiler.h"
#include "AtomToolsFramework/Graph/GraphTemplateFileData.h"
#include "AtomToolsFramework/Graph/GraphTemplateFileDataCacheRequestBus.h"
#include "AtomToolsFramework/Graph/GraphUtil.h"
#include "AtomToolsFramework/Util/Util.h"
#include "AzCore/Asset/AssetCommon.h"
#include "AzCore/Console/ILogger.h"
#include "AzCore/Debug/Trace.h"
#include "AzCore/IO/Path/Path_fwd.h"
#include "AzCore/RTTI/RTTIMacros.h"
#include "AzCore/Script/ScriptAsset.h"
#include "AzCore/Serialization/ObjectStream.h"
#include "AzCore/Serialization/SerializeContext.h"
#include "AzCore/StringFunc/StringFunc.h"
#include "AzCore/Utils/Utils.h"
#include "AzCore/std/smart_ptr/shared_ptr.h"
#include "AzCore/std/string/regex.h"
#include "AzFramework/Asset/AssetSystemBus.h"
#include "AzToolsFramework/API/EditorAssetSystemAPI.h"
#include "Conversation/DialogueData.h"
#include "GraphModel/Model/Common.h"
#include "GraphModel/Model/Node.h"
#include "GraphModel/Model/Slot.h"

#include "Conversation/Constants.h"
#include "Conversation/ConversationAsset.h"
#include "Conversation/ConversationTypeIds.h"
#include "Conversation/DialogueData_incl.h"
#include "Tools/DataTypes.h"
#include "Tools/Document/NodeRequestBus.h"

namespace ConversationEditor
{
    AZ_RTTI_NO_TYPE_INFO_IMPL(ConversationGraphCompiler, AtomToolsFramework::GraphCompiler); // NOLINT
    AZ_TYPE_INFO_WITH_NAME_IMPL(ConversationGraphCompiler, "ConversationGraphCompiler", ConversationGraphCompilerTypeId); // NOLINT
    AZ_CLASS_ALLOCATOR_IMPL(ConversationGraphCompiler, AZ::SystemAllocator); // NOLINT

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

    auto ConversationGraphCompiler::CompileGraph(GraphModel::GraphPtr graph, AZStd::string const& graphName, AZStd::string const& graphPath)
        -> bool
    {
        if (IsCompileLoggingEnabled())
        {
            AZLOG_INFO("Compiling conversation graph '%s'...", graphName.c_str()); // NOLINT
        }

        ClearData();

        if (!AtomToolsFramework::GraphCompiler::CompileGraph(graph, graphName, graphPath))
        {
            if (IsCompileLoggingEnabled())
            {
                AZLOG_INFO("Base graph compilation failed."); // NOLINT
            }
            return false;
        }

        BuildSlotValueTable();
        if (!BuildDependencyTables())
        {
            SetState(AtomToolsFramework::GraphCompiler::State::Failed);
            return false;
        }

        AZStd::vector<GraphModel::ConstNodePtr> const nodesInExecutionOrder = GetAllNodesInExecutionOrder();

        constexpr auto scriptTemplatePath =
            "@gemroot:Conversation@/Assets/ConversationCanvas/GraphData/ConversationOutputs/ConversationGraphName.lua";

        m_scriptFileDataTemplate.Load(scriptTemplatePath);
        ReplaceBasicSymbols(m_scriptFileDataTemplate);

        // Traverse all graph nodes and slots searching for settings to generate files from templates
        for (auto const& currentNode : nodesInExecutionOrder)
        {
            m_currentNode = currentNode;
            if (!m_currentNode)
            {
                AZ_Error("ConversationGraphCompiler", false, "Current node is null!"); // NOLINT
                SetState(AtomToolsFramework::GraphCompiler::State::Failed);
                return false;
            }

            // Get and store the data in node so it can be used while building a conversation asset later.
            BuildNode(currentNode);

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
                AZ_Error("ConversationGraphCompiler", false, "Compilation failed while trying to export '.lua' files."); // NOLINT
                SetState(AtomToolsFramework::GraphCompiler::State::Failed);
                return false;
            }
        };

        if (!BuildConversationAsset())
        {
            AZ_Error("ConversationGraphCompiler", false, "Compilation failed to build the conversation asset."); // NOLINT
            SetState(AtomToolsFramework::GraphCompiler::State::Failed);
        }

        m_scriptFileDataTemplate.ReplaceLinesInBlock(
            "BOP_GENERATED_FUNCTIONS_BEGIN",
            "BOP_GENERATED_FUNCTIONS_END",
            [&]([[maybe_unused]] AZStd::string const& blockHeader)
            {
                return m_functionDefinitions;
            });

        m_scriptFileDataTemplate.ReplaceLinesInBlock(
            "BOP_GENERATED_CONDITION_FUNCTIONS_BEGIN",
            "BOP_GENERATED_CONDITION_FUNCTIONS_END",
            [&]([[maybe_unused]] AZStd::string const& blockHeader)
            {
                return m_conditionFunctionDefinitions;
            });

        auto const templateOutputPath = GetOutputPathFromTemplatePath(m_scriptFileDataTemplate.GetPath());
        m_scriptFileDataTemplate.Save(templateOutputPath);

        if (!ReportGeneratedFileStatus())
        {
            AZ_Error( // NOLINT
                "ConversationGraphCompiler",
                false,
                "Compilation failed because the generated files were not successfully processed by the O3DE Asset Processor."); // NOLINT
            SetState(AtomToolsFramework::GraphCompiler::State::Failed);
            return false;
        }

        SetState(AtomToolsFramework::GraphCompiler::State::Complete);

        AZLOG_INFO("Conversation graph compiled successfully.\n"); // NOLINT
        return true;
    }

    auto ConversationGraphCompiler::GetGraphPath() const -> AZStd::string
    {
        if (auto const& graphPath = AtomToolsFramework::GraphCompiler::GetGraphPath(); graphPath.ends_with(".conversationgraph"))
        {
            return graphPath;
        }

        return AZStd::string::format("%s/Assets/Conversations/Generated/untitled.conversationgraph", AZ::Utils::GetProjectPath().c_str());
    }

    auto ConversationGraphCompiler::GetSlotValueTable() const -> AZStd::map<GraphModel::ConstSlotPtr, AZStd::any> const&
    {
        return m_slotValueTable;
    }

    auto ConversationGraphCompiler::ModifySlotValueTable() -> AZStd::map<GraphModel::ConstSlotPtr, AZStd::any>&
    {
        return m_slotValueTable;
    }

    constexpr auto ConversationGraphCompiler::GetStartingIds() -> StartingIdContainer const&
    {
        return m_startingIds;
    }

    constexpr auto ConversationGraphCompiler::ModifyStartingIds() -> StartingIdContainer&
    {
        return m_startingIds;
    }

    constexpr auto ConversationGraphCompiler::GetTemplatePathsForCurrentNode() -> AZStd::set<AZStd::string> const&
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

    constexpr void ConversationGraphCompiler::ClearInstructionsForCurrentNodeAndReserveSize(size_t reserveAmount)
    {
        ModifyInstructionNodesForCurrentNode().clear();
        m_instructionNodesForCurrentNode.reserve(reserveAmount);
    }

    auto ConversationGraphCompiler::BuildDependencyTables() -> AZStd::expected<AZStd::true_type, CompilationError>
    {
        if (!m_graph)
        {
            return AZStd::unexpected(
                CompilationError{ CompilationErrorCode::NullGraphPointer, "The graph assigned to the compiler is null!" });
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
                        });
                }
            }
        }

        return AZStd::true_type{};
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

        for (auto const& inputSlotName : inputSlotNames)
        {
            if (auto const slot = outputNode->GetSlot(inputSlotName))
            {
                if (slot->GetSlotDirection() == GraphModel::SlotDirection::Input)
                {
                    for (auto const& connection : slot->GetConnections())
                    {
                        AZ_Assert( // NOLINT
                            connection->GetSourceNode() != outputNode,
                            "This should never be the source node on an input connection.");
                        AZ_Assert( // NOLINT
                            connection->GetTargetNode() == outputNode,
                            "This should always be the target node on an input connection.");
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
        AtomToolsFramework::DynamicNodeSlotConfig const& slotConfig,
        AZStd::vector<AZStd::pair<AZStd::string, AZStd::string>> const& substitutionSymbols) const -> AZStd::vector<AZStd::string>
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
            [this, &outputNode, &inputSlotNames](auto const& node)
            {
                return !ShouldUseInstructionsFromInputNode(outputNode, node, inputSlotNames);
            });
        return nodes;
    }

    [[nodiscard]] auto ConversationGraphCompiler::GetSymbolNameFromNode(GraphModel::ConstNodePtr const& node) const -> AZStd::string
    {
        auto const nodeNameSlot = node->GetSlot(ToString(GeneralSlots::NodeName));
        auto const nodeNameValueAny = nodeNameSlot != nullptr ? nodeNameSlot->GetValue() : AZStd::any();

        // If there's a NodeName property, we use that as the symbol name instead of generating a name.
        if (nodeNameValueAny.is<AZStd::string>())
        {
            if (auto nodeName = AZStd::any_cast<AZStd::string>(nodeNameValueAny); !nodeName.empty())
            {
                return AtomToolsFramework::GetSymbolNameFromText(nodeName);
            }
        }

        return AtomToolsFramework::GetSymbolNameFromText(AZStd::string::format("node%u_%s", node->GetId(), node->GetTitle()));
    }

    [[nodiscard]] auto ConversationGraphCompiler::GetSymbolNameFromSlot(GraphModel::ConstSlotPtr slot) const -> AZStd::string
    {
        bool allowNameSubstitution = true;
        if (auto dynamicNode = azrtti_cast<AtomToolsFramework::DynamicNode const* const>(slot->GetParentNode().get()))
        {
            auto const& nodeConfig = dynamicNode->GetConfig();
            AtomToolsFramework::VisitDynamicNodeSlotConfigs(
                nodeConfig,
                [&](AtomToolsFramework::DynamicNodeSlotConfig const& slotConfig)
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
        auto const& slotValue = GetValueFromSlot(slot);

        // This code and some of these rules will be refactored and generalized after splitting this class into a document and builder
        // or compiler class. Once that is done, it will be easier to register types, conversions, substitutions with the system.
        for (auto const& connection : slot->GetConnections())
        {
            auto sourceSlot = connection->GetSourceSlot();
            auto targetSlot = connection->GetTargetSlot();
            if (targetSlot == slot)
            {
                // If there is an incoming connection to this slot, the name of the source slot from the incoming connection will be
                // used as part of the value for the slot.
                auto const& sourceSlotSymbolName = GetSymbolNameFromSlot(sourceSlot);
                return sourceSlotSymbolName;
            }
        }

        // If the slot's embedded value is being used then generate lua code to represent it. More generic options will be explored
        // to clean this code up, possibly storing numeric values in a two-dimensional floating point array with the layout
        // corresponding to most vector and matrix types.
        if (auto v = AZStd::any_cast<float const>(&slotValue))
        {
            return AZStd::string::format("%g", *v);
        }
        if (auto v = AZStd::any_cast<int const>(&slotValue))
        {
            return AZStd::string::format("%i", *v);
        }
        if (auto v = AZStd::any_cast<unsigned int const>(&slotValue))
        {
            return AZStd::string::format("%u", *v);
        }
        if (auto v = AZStd::any_cast<bool const>(&slotValue))
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

        for (auto const& slotPair : node->GetSlots())
        {
            auto const& slot = slotPair.second;

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
        ClearInstructionsForCurrentNodeAndReserveSize(m_graph->GetNodeCount());

        AZ::parallel_for_each(
            m_templateFileDataVecForCurrentNode.begin(),
            m_templateFileDataVecForCurrentNode.end(),
            [&](auto& templateFileData)
            {
                templateFileData.ReplaceLinesInBlock(
                    "BOP_GENERATED_INSTRUCTIONS_BEGIN",
                    "BOP_GENERATED_INSTRUCTIONS_END",
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

                AZStd::scoped_lock lock(m_conditionFunctionDefinitionsMutex);
                // We create a string representation of the generated instructions when there's an outCondition slot.
                // When compiling, we'll use it to add a new function to the companion script that can be called to
                // confirm a dialogue's availability in Lua.
                if (currentNode->GetSlot(ToString(ConditionNodeSlots::outCondition)))
                {
                    AZStd::string luaFunc{};
                    AZ::StringFunc::Join(luaFunc, templateFileData.GetLines(), "\n");
                    m_conditionFunctionDefinitions.emplace_back(AZStd::move(luaFunc));
                }
            });
    }

    auto ConversationGraphCompiler::BuildConversationAsset() -> AZStd::expected<AZStd::true_type, CompilationError>
    {
        auto const conversationAsset = AZStd::make_unique<Conversation::ConversationAsset>();

        AZStd::ranges::for_each(
            m_startingIds,
            [&conversationAsset](auto const& startingId) -> void
            {
                conversationAsset->AddStartingId(startingId);
            });

        for (auto& [node, nodeData] : m_nodeDataTable)
        {
            auto dialogueDataOpt = nodeData.m_dialogue;
            if (dialogueDataOpt.has_value())
            {
                auto& dialogueResponses = Conversation::ModifyDialogueResponseIds(*dialogueDataOpt);
                dialogueResponses.insert(dialogueResponses.end(), nodeData.m_responseIds.begin(), nodeData.m_responseIds.end());

                conversationAsset->AddDialogue(*dialogueDataOpt);
            }
        }

        // Create the path where we will save the asset.
        auto const assetOutputPath = [this]() -> AZStd::string
        {
            // "/path/to/somedialogue.conversationgraph"
            auto pathToSaveAsset = GetGraphPath();
            // "/path/to/somedialogue.conversationasset"
            AZ::StringFunc::Path::ReplaceExtension(pathToSaveAsset, Conversation::ConversationAsset::ProductExtension);

            // ""
            AZStd::string fullFileName{};
            // "somedialogue.conversationasset"
            AZ::StringFunc::Path::GetFullFileName(pathToSaveAsset.c_str(), fullFileName);

            // "/path/to/somedialogue.conversationasset" -> "/path/to"
            AZ::StringFunc::Path::StripFullName(pathToSaveAsset);

            // "/path/to/Generated/somedialogue.conversationasset"
            AZ::StringFunc::Path::Join(
                pathToSaveAsset.c_str(), AZStd::string::format("Generated/%s", fullFileName.c_str()).c_str(), pathToSaveAsset);

            return pathToSaveAsset;
        }();

        bool const saveResult = AZ::Utils::SaveObjectToFile(assetOutputPath, AZ::DataStream::ST_JSON, conversationAsset.get());
        if (!saveResult)
        {
            return AZStd::unexpected{ CompilationError{ CompilationErrorCode::FailedToSaveConversationTemplate,
                                                        AZStd::string::format("Failed to save '%s'", assetOutputPath.c_str()) } };
        }

        AzFramework::AssetSystemRequestBus::Broadcast(
            &AzFramework::AssetSystem::AssetSystemRequests::EscalateAssetBySearchTerm, assetOutputPath);

        m_generatedFiles.push_back(assetOutputPath);

        return AZStd::true_type();
    }

    [[nodiscard]] auto ConversationGraphCompiler::GetValueFromSlot(GraphModel::ConstSlotPtr const slot) const -> AZStd::any
    {
        auto const& slotItr = AZStd::ranges::find_if(
            GetSlotValueTable(),
            [slot](auto const& key) -> bool
            {
                return key.first == slot;
            });

        return slotItr != GetSlotValueTable().end() ? slotItr->second : slot->GetValue();
    }

    [[nodiscard]] auto ConversationGraphCompiler::GetValueFromSlotOrConnection(GraphModel::ConstSlotPtr const& slot) const -> AZStd::any
    {
        for (auto const& connection : slot->GetConnections())
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
            for (auto const& nodePair : m_graph->GetNodes())
            {
                nodes.push_back(nodePair.second);
            }

            AtomToolsFramework::SortNodesInExecutionOrder(nodes);
        }

        return AZStd::move(nodes);
    }

    void ConversationGraphCompiler::BuildNode(GraphModel::ConstNodePtr const& currentNode)
    {
        using namespace Conversation;

        if (!m_graph)
        {
            return;
        }

        auto& nodeData = m_nodeDataTable[currentNode];

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
                    AZStd::ranges::contains(
                           dynamicNodeSettings.find(NodeSettings::NodeTypeKey)->second, NodeSettings::NodeTypeValue_Dialogue);
            }();

            if (isDialogueNode)
            {
                if (BuildDialogueNode(currentNode))
                {
                };
            }
        }
        else if (currentNode->RTTI_IsTypeOf(AZ::TypeId{ LinkNodeTypeId }))
        {
            auto* nodeRequests = azrtti_cast<NodeRequests const*>(currentNode.get());
            if (!nodeRequests)
            {
                return;
            }

            nodeRequests->UpdateNodeData(nodeData);
            BuildLinkNode(currentNode);
        }
    }

    auto ConversationGraphCompiler::BuildDialogueNode(GraphModel::ConstNodePtr const& currentNode)
        -> AZStd::expected<AZStd::true_type, CompilationError>
    {
        using namespace Conversation;

        if (!currentNode)
        {
            return AZStd::unexpected(
                CompilationError{ CompilationErrorCode::ExpectedDialogueNodeButGotNullptr, "Nullptr given as argument." });
        }

        DialogueId const currentNodeDialogueId{ GetSymbolNameFromNode(currentNode) };
        auto& nodeDataDialogue = m_nodeDataTable[currentNode].m_dialogue;

        // The DialogueData is an optional since not every node will use one. Since we're processing a dialogue node, we need one, so we
        // instantiate one if one hasn't already been instantiated.
        if (!nodeDataDialogue.has_value())
        {
            nodeDataDialogue.emplace(currentNodeDialogueId);
        }

        // Dialogue nodes that have a connection to inCondition will need to add the connected
        // node's symbol as an availability Id.
        if (auto const inConditionSlot = currentNode->GetSlot(ToString(DialogueNodeSlots::inCondition));
            inConditionSlot && !inConditionSlot->GetConnections().empty())
        {
            AZ_Error( // NOLINT
                "ConversationGraphCompiler",
                inConditionSlot->GetConnections().size() <= 1,
                "There should be a maximum of one connection.");

            if (inConditionSlot->GetConnections().size() == 1)
            {
                auto const sourceNode = inConditionSlot->GetConnections().front()->GetSourceNode();
                SetDialogueAvailabilityId(*nodeDataDialogue, GetSymbolNameFromNode(sourceNode));
            }
        }

        for (auto& [slotId, slot] : currentNode->GetSlots())
        {
            auto const value = m_slotValueTable[slot];

            switch (slot->GetDataType()->GetTypeEnum())
            {
            case AZ_CRC_CE("actor_text"):
                SetDialogueActorText(*nodeDataDialogue, AZStd::any_cast<AZStd::string>(value));
                break;
            case AZ_CRC_CE("speaker_tag"):
                SetDialogueSpeaker(*nodeDataDialogue, AZStd::any_cast<AZStd::string>(value));
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
            m_startingIds.insert(nodeDataDialogue->m_id);
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
                        "There should be exactly one connection on the current node's parent input. We found: %lu.",
                        connections.size());
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

        return AZStd::true_type();
    }

    void ConversationGraphCompiler::BuildLinkNode(GraphModel::ConstNodePtr const& linkNode)
    {
        auto const& linkData = m_nodeDataTable[linkNode].m_linkData;
        auto& fromNodeData = m_nodeDataTable[linkData.m_from];
        auto const& toNode = linkData.m_to;

        // Add the 'To' node's ID to the 'From' node's list of responses.
        fromNodeData.m_responseIds.push_back(Conversation::DialogueId{ GetSymbolNameFromNode(toNode) });
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
            });
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
            [&](AtomToolsFramework::DynamicNodeSettingsMap const& settings)
            {
                AtomToolsFramework::CollectDynamicNodeSettings(settings, "templatePaths", ModifyTemplatePathsForCurrentNode());
            });
    }

    auto ConversationGraphCompiler::LoadTemplatesForCurrentNode() -> bool
    {
        m_templateFileDataVecForCurrentNode.clear();

        for (auto const& templatePath : m_templatePathsForCurrentNode)
        {
            bool const isLuaTemplate = templatePath.ends_with(".lua");
            bool const isConversationTemplate = templatePath.ends_with(".conversationtemplate");

            if (isLuaTemplate || isConversationTemplate)
            {
                // Load the unmodified, template source file data, which will be copied and used for insertions, substitutions, and
                // code generation.
                AtomToolsFramework::GraphTemplateFileData templateFileData;
                AtomToolsFramework::GraphTemplateFileDataCacheRequestBus::EventResult(
                    templateFileData,
                    m_toolId,
                    &AtomToolsFramework::GraphTemplateFileDataCacheRequestBus::Events::Load,
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
        AZStd::string const templateInputFileName = [&templateInputPath]() -> AZStd::string
        {
            AZStd::string result{};
            AZ::StringFunc::Path::GetFullFileName(templateInputPath.c_str(), result);
            return result;
        }();

        AZStd::string templateOutputPath = GetGraphPath();

        AZ::StringFunc::Path::ReplaceFullName(templateOutputPath, templateInputFileName.c_str());

        AZ::StringFunc::Replace(templateOutputPath, "ConversationGraphName", GetUniqueGraphName().c_str());
        // Replacing the condition symbol is only valid while processing a node.
        if (m_currentNode)
        {
            AZ::StringFunc::Replace(templateOutputPath, "ConversationGraphConditionNode", GetSymbolNameFromNode(m_currentNode).c_str());
        }
        return templateOutputPath;
    }

    void ConversationGraphCompiler::DeleteExistingFilesForCurrentNode()
    {
        if (AtomToolsFramework::GetSettingsValue("/O3DE/Atom/ConversationCanvas/ForceDeleteGeneratedFiles", true))
        {
            AZLOG_INFO( // NOLINT
                "Deleting generated files.\n");
            AZ::parallel_for_each(
                ModifyTemplateDataForCurrentNode().begin(),
                ModifyTemplateDataForCurrentNode().end(),
                [this](auto const& templateFileData)
                {
                    auto const templateInputPath = AtomToolsFramework::GetPathWithoutAlias(templateFileData.GetPath());
                    auto const templateOutputPath = GetOutputPathFromTemplatePath(templateInputPath);

                    auto fileIO = AZ::IO::FileIOBase::GetInstance();
                    fileIO->Remove(templateOutputPath.c_str());
                });

            AZLOG_INFO( // NOLINT
                "Finished deleting generated files.\n");
        }
    }

    void ConversationGraphCompiler::PreProcessTemplate(AtomToolsFramework::GraphTemplateFileData& templateFileData)
    {
        // Substitute all references to the placeholder graph name with one generated from the document name
        ReplaceBasicSymbols(templateFileData);

        // Inject include files found while traversing the graph into any include file blocks in the template.
        templateFileData.ReplaceLinesInBlock(
            "BOP_GENERATED_INCLUDES_BEGIN",
            "BOP_GENERATED_INCLUDES_END",
            [&, this]([[maybe_unused]] AZStd::string const& blockHeader)
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
                        relativePathFound,
                        &AzToolsFramework::AssetSystem::AssetSystemRequest::GenerateRelativeSourcePath,
                        AtomToolsFramework::GetPathWithoutAlias(path),
                        relativePath,
                        relativePathFolder);

                    if (relativePathFound)
                    {
                        includeStatements.push_back(AZStd::string::format("require(\"%s\")", relativePath.c_str()));
                    }
                }
                return includeStatements;
            });

        // Inject class definitions found while traversing the graph.
        templateFileData.ReplaceLinesInBlock(
            "BOP_GENERATED_CLASSES_BEGIN",
            "BOP_GENERATED_CLASSES_END",
            [&]([[maybe_unused]] AZStd::string const& blockHeader)
            {
                return m_classDefinitions;
            });

        // Inject function definitions found while traversing the graph.
        templateFileData.ReplaceLinesInBlock(
            "BOP_GENERATED_FUNCTIONS_BEGIN",
            "BOP_GENERATED_FUNCTIONS_END",
            [&]([[maybe_unused]] AZStd::string const& blockHeader)
            {
                return m_functionDefinitions;
            });
    }

    void ConversationGraphCompiler::PreprocessTemplatesForCurrentNode()
    {
        AZ::parallel_for_each(
            ModifyTemplateDataForCurrentNode().begin(),
            ModifyTemplateDataForCurrentNode().end(),
            [&](auto& templateFileData)
            {
                PreProcessTemplate(templateFileData);
            });

        PreProcessTemplate(m_scriptFileDataTemplate);
    }

    [[nodiscard]] auto ConversationGraphCompiler::GetUniqueGraphName() const -> AZStd::string
    {
        return m_templateNodeCount <= 0 ? GetGraphName().data()
                                        : AZStd::string::format("%s_%03i", GetGraphName().data(), m_templateNodeCount);
    }

    auto ConversationGraphCompiler::ExportTemplatesMatchingRegex(AZStd::string const& pattern) -> bool
    {
        AZStd::regex const patternRegex(pattern, AZStd::regex::flag_type::icase);
        for (auto const& templateFileData : m_templateFileDataVecForCurrentNode)
        {
            if (AZStd::regex_match(templateFileData.GetPath(), patternRegex))
            {
                auto const& templateOutputPath = GetOutputPathFromTemplatePath(templateFileData.GetPath());

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
        m_scriptFileDataTemplate = {};
        m_instructionNodesForCurrentNode.clear();
        m_currentNode = nullptr;
        m_generatedFiles.clear();
        m_includePaths.clear();
        m_classDefinitions.clear();
        m_functionDefinitions.clear();
        m_conditionFunctionDefinitions.clear();
        m_slotValueTable.clear();
        m_slotDialogueTable.clear();
        m_startingIds.clear();
        m_nodeDataTable.clear();
        m_templateFileDataVecForCurrentNode.clear();
        m_configIdsVisited.clear();
        m_templateNodeCount = 0;
    }

    void ConversationGraphCompiler::ReplaceBasicSymbols(AtomToolsFramework::GraphTemplateFileData& templateFileData)
    {
        templateFileData.ReplaceSymbol("ConversationGraphName", GetUniqueGraphName());
        if (m_currentNode)
        {
            templateFileData.ReplaceSymbol("ConversationGraphNodeName", GetSymbolNameFromNode(m_currentNode));
        }
    }

} // namespace ConversationEditor
