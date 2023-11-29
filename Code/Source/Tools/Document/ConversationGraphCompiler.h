#pragma once

#include "AtomToolsFramework/Graph/DynamicNode/DynamicNodeSlotConfig.h"
#include "AtomToolsFramework/Graph/GraphCompiler.h"
#include "AtomToolsFramework/Graph/GraphTemplateFileData.h"
#include "AzCore/Math/Color.h"
#include "AzCore/RTTI/RTTIMacros.h"
#include "Conditions/ConditionFunction.h"
#include "GraphModel/Model/Common.h"

#include "Conversation/DialogueData.h"

namespace ConversationEditor
{
    class ConversationGraphCompiler;

    using NodeContainer = AZStd::set<GraphModel::NodeId>;
    using NodeIdToDialogueTable = AZStd::map<GraphModel::NodeId, Conversation::DialogueData>;
    using StartingIdContainer = AZStd::set<Conversation::DialogueId>;
    using SlotValueTable = AZStd::map<GraphModel::ConstSlotPtr, AZStd::any>;
    using SlotDialogueTable = AZStd::map<GraphModel::ConstSlotPtr, Conversation::DialogueData>;
    using SlotConditionTable = AZStd::map<GraphModel::ConstSlotPtr, Conversation::ConditionFunction>;

    class ConversationGraphCompiler : public AtomToolsFramework::GraphCompiler
    {
    public:
        AZ_RTTI(ConversationGraphCompiler, "F57500CA-5B13-4833-988A-DDA6ACF238A0", AtomToolsFramework::GraphCompiler); // NOLINT
        AZ_CLASS_ALLOCATOR(ConversationGraphCompiler, AZ::SystemAllocator); // NOLINT
        AZ_DISABLE_COPY_MOVE(ConversationGraphCompiler); // NOLINT

        ConversationGraphCompiler() = default;
        ConversationGraphCompiler(AZ::Crc32 const& toolId);
        ~ConversationGraphCompiler() override;

        static void Reflect(AZ::ReflectContext* context);

        auto CompileGraph(GraphModel::GraphPtr graph, AZStd::string const& graphName, AZStd::string const& graphPath) -> bool override;
        constexpr auto GetGraphName() const -> AZStd::string_view
        {
            return m_graphName;
        }

        [[nodiscard]] auto GetSlotValueTable() const -> AZStd::map<GraphModel::ConstSlotPtr, AZStd::any> const&;

        [[nodiscard]] auto ModifySlotValueTable() -> AZStd::map<GraphModel::ConstSlotPtr, AZStd::any>&;

        [[nodiscard]] auto GetStartingIds() -> StartingIdContainer const&;

        [[nodiscard]] auto ModifyStartingIds() -> StartingIdContainer&;

        [[nodiscard]] auto GetTemplatePathsForCurrentNode() -> AZStd::set<AZStd::string> const&;

        [[nodiscard]] auto ModifyTemplatePathsForCurrentNode() -> AZStd::set<AZStd::string>&;

        [[nodiscard]] auto GetTemplateDataForCurrentNode() -> AZStd::list<AtomToolsFramework::GraphTemplateFileData> const&;

        [[nodiscard]] auto ModifyTemplateDataForCurrentNode() -> AZStd::list<AtomToolsFramework::GraphTemplateFileData>&;

        [[nodiscard]] auto ModifyInstructionNodesForCurrentNode() -> AZStd::vector<GraphModel::ConstNodePtr>&;

        void ClearInstructionsForCurrentNode(size_t reserveAmount);

        void BuildDependencyTables();

        [[nodiscard]] auto ShouldUseInstructionsFromInputNode(
            GraphModel::ConstNodePtr const& outputNode,
            GraphModel::ConstNodePtr const& inputNode,
            AZStd::vector<AZStd::string> const& inputSlotNames) const -> bool;

        [[nodiscard]] auto GetInstructionsFromSlot(
            GraphModel::ConstNodePtr const& node,
            const AtomToolsFramework::DynamicNodeSlotConfig& slotConfig,
            const AZStd::vector<AZStd::pair<AZStd::string, AZStd::string>>& substitutionSymbols) const -> AZStd::vector<AZStd::string>;

        [[nodiscard]] auto GetInstructionNodesInExecutionOrder(
            GraphModel::ConstNodePtr const& outputNode, AZStd::vector<AZStd::string> const& inputSlotNames) const
            -> AZStd::vector<GraphModel::ConstNodePtr>;

        [[nodiscard]] auto GetSymbolNameFromNode(GraphModel::ConstNodePtr const& node) const -> AZStd::string;

        [[nodiscard]] auto GetSymbolNameFromSlot(GraphModel::ConstSlotPtr slot) const -> AZStd::string;

        [[nodiscard]] auto GetLuaTypeFromSlot(GraphModel::ConstSlotPtr const& slot) const -> AZStd::string;

        [[nodiscard]] auto GetLuaValueFromSlot(GraphModel::ConstSlotPtr const& slot) const -> AZStd::string;

        [[nodiscard]] auto GetSubstitutionSymbolsFromNode(GraphModel::ConstNodePtr const& node) const
            -> AZStd::vector<AZStd::pair<AZStd::string, AZStd::string>>;

        [[nodiscard]] auto GetInstructionsFromConnectedNodes(
            GraphModel::ConstNodePtr const& outputNode,
            AZStd::vector<AZStd::string> const& inputSlotNames,
            AZStd::vector<GraphModel::ConstNodePtr>& instructionNodes) const -> AZStd::vector<AZStd::string>;

        void BuildInstructionsForCurrentNode(GraphModel::ConstNodePtr const& currentNode);

        [[nodiscard]] auto GetVectorSize(AZStd::any const& slotValue) const -> unsigned int;

        [[nodiscard]] auto GetValueFromSlot(GraphModel::ConstSlotPtr const slot) const -> AZStd::any;

        [[nodiscard]] auto GetValueFromSlotOrConnection(GraphModel::ConstSlotPtr const& slot) const -> AZStd::any;

        [[nodiscard]] auto GetAllNodesInExecutionOrder() const -> AZStd::vector<GraphModel::ConstNodePtr>;

        void BuildNode(GraphModel::ConstNodePtr const& node);
        void BuildDialogueNode(GraphModel::ConstNodePtr const& node);

        void BuildSlotValueTable();

        // void BuildDialogueNodeData();

        [[nodiscard]] auto ConvertToScalar(const AZStd::any& slotValue) const -> AZStd::any;

        template<typename T>
        [[nodiscard]] auto ConvertToVector(const AZStd::any& slotValue) const -> AZStd::any
        {
            if (auto v = AZStd::any_cast<const AZ::Color>(&slotValue))
            {
                return AZStd::any(T(v->GetAsVector4()));
            }
            if (auto v = AZStd::any_cast<const AZ::Vector4>(&slotValue))
            {
                return AZStd::any(T(*v));
            }
            if (auto v = AZStd::any_cast<const AZ::Vector3>(&slotValue))
            {
                return AZStd::any(T(*v));
            }
            if (auto v = AZStd::any_cast<const AZ::Vector2>(&slotValue))
            {
                return AZStd::any(T(*v));
            }
            return slotValue;
        }

        [[nodiscard]] auto ConvertToVector(const AZStd::any& slotValue, unsigned int score) const -> AZStd::any;

        void BuildTemplatePathsForCurrentNode(GraphModel::ConstNodePtr const& currentNode);

        auto LoadTemplatesForCurrentNode() -> bool;

        [[nodiscard]] auto GetOutputPathFromTemplatePath(AZStd::string const& templateInputPath) const -> AZStd::string;

        void DeleteExistingFilesForCurrentNode();

        void PreprocessTemplatesForCurrentNode();

        [[nodiscard]] auto GetUniqueGraphName() const -> AZStd::string;

        auto ExportTemplatesMatchingRegex(AZStd::string const& pattern) -> bool;

        void ClearData();

    private:
        struct NodeData
        {
            AZStd::optional<Conversation::DialogueData> m_dialogue;
            AZStd::vector<AZ::Name> m_conditions;
            AZStd::vector<Conversation::DialogueId> m_responseIds;
        };

        struct DialogueNodeData
        {
            Conversation::DialogueData m_dialogue;
            AZStd::vector<AZ::Name> m_conditions;
            AZStd::vector<Conversation::DialogueId> m_responseIds;
        };

        struct ConditionNodeData
        {
        };
        struct ScriptNodeData
        {
        };

        using ConversationNodeData = AZStd::variant<DialogueNodeData, ConditionNodeData, ScriptNodeData>;

        AZStd::mutex m_instructionNodesForCurrentNodeMutex;

        GraphModel::ConstNodePtr m_currentNode{};

        AZStd::vector<AZStd::string> m_generatedFiles;
        AZStd::set<AZStd::string> m_includePaths{};
        AZStd::vector<AZStd::string> m_classDefinitions{};
        AZStd::vector<AZStd::string> m_functionDefinitions{};
        AZStd::vector<AZStd::string> m_conditionInstructions;
        SlotValueTable m_slotValueTable{};
        SlotDialogueTable m_slotDialogueTable{};
        SlotConditionTable m_slotConditionTable{};
        StartingIdContainer m_startingIds{};
        AZStd::map<GraphModel::ConstNodePtr, NodeData> m_nodeDataTable{};
        AZStd::map<GraphModel::ConstNodePtr, ConversationNodeData> m_tempNodeDataTableDoNotUse{};
        AZStd::list<AtomToolsFramework::GraphTemplateFileData> m_templateFileDataVecForCurrentNode{};
        AZStd::set<AZStd::string> m_templatePathsForCurrentNode{};
        AZStd::vector<GraphModel::ConstNodePtr> m_instructionNodesForCurrentNode;
        AZStd::unordered_set<Conversation::DialogueChunk> m_chunks;

        // Container of unique node configurations IDs visited on the graph to collect include paths, class definitions, and function
        // definitions.
        AZStd::unordered_set<AZ::Uuid> m_configIdsVisited;

        // This counter will be used as a suffix for graph name substitutions in case multiple template nodes are included in the same
        // graph
        [[maybe_unused]] int m_templateNodeCount = 0;
    };
} // namespace ConversationEditor
