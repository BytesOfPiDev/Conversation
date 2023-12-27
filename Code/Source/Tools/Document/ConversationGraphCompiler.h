#pragma once

#include "AtomToolsFramework/Graph/DynamicNode/DynamicNodeSlotConfig.h"
#include "AtomToolsFramework/Graph/GraphCompiler.h"
#include "AtomToolsFramework/Graph/GraphTemplateFileData.h"
#include "AzCore/RTTI/RTTIMacros.h"
#include "AzCore/std/utility/expected.h"
#include "GraphModel/Model/Common.h"

#include "Conditions/ConditionFunction.h"
#include "Conversation/DialogueData.h"
#include "Tools/NodeData.h"

namespace ConversationEditor
{
    using StartingIdContainer = AZStd::vector<Conversation::UniqueId>;
    using SlotValueTable = AZStd::map<GraphModel::ConstSlotPtr, AZStd::any>;
    using SlotDialogueTable =
        AZStd::map<GraphModel::ConstSlotPtr, Conversation::DialogueData>;
    using SlotConditionTable =
        AZStd::map<GraphModel::ConstSlotPtr, Conversation::ConditionFunction>;

    // NOTE: Experimenting. Trying to find an expressive way to say what errors
    // can happen w/o exceptions.
    AZ_ENUM( // NOLINT
        CompilationErrorCode,
        NullGraphPointer,
        FailedToSaveTemplate,
        BadDialogueNodeData);

    using SuccessTypeWhenNoReturnDataExpected = AZStd::true_type;

    struct CompilationError
    {
        static constexpr auto ErrorMessageSize{ 64 };
        using ErrorMessageType = std::array<char, ErrorMessageSize>;

        CompilationErrorCode m_errorCode;
        ErrorMessageType m_errorMessage;
    };

    static_assert(
        AZStd::is_pod_v<CompilationError>, "Ensure CompilationError is POD");

    class ConversationGraphCompiler : public AtomToolsFramework::GraphCompiler
    {
    public:
        AZ_RTTI_NO_TYPE_INFO_DECL(); // NOLINT
        AZ_TYPE_INFO_WITH_NAME_DECL(ConversationGraphCompiler); // NOLINT
        AZ_CLASS_ALLOCATOR_DECL; // NOLINT
        AZ_DISABLE_COPY_MOVE(ConversationGraphCompiler); // NOLINT

        ConversationGraphCompiler() = default;
        ConversationGraphCompiler(AZ::Crc32 const& toolId);
        ~ConversationGraphCompiler() override = default;

        static void Reflect(AZ::ReflectContext* context);

        auto CompileGraph(
            GraphModel::GraphPtr graph,
            AZStd::string const& graphName,
            AZStd::string const& graphPath) -> bool override;

        [[nodiscard]] auto GetGraphPath() const -> AZStd::string override;

        [[nodiscard]] constexpr auto GetGraphName() const -> AZStd::string_view
        {
            return m_graphName;
        }

        [[nodiscard]] auto GetSlotValueTable() const
            -> AZStd::map<GraphModel::ConstSlotPtr, AZStd::any> const&;

        [[nodiscard]] auto ModifySlotValueTable()
            -> AZStd::map<GraphModel::ConstSlotPtr, AZStd::any>&;

        [[nodiscard]] constexpr auto GetStartingIds()
            -> StartingIdContainer const&;

        [[nodiscard]] constexpr auto ModifyStartingIds()
            -> StartingIdContainer&;

        [[nodiscard]] constexpr auto GetTemplatePathsForCurrentNode()
            -> AZStd::set<AZStd::string> const&;

        [[nodiscard]] auto ModifyTemplatePathsForCurrentNode()
            -> AZStd::set<AZStd::string>&;

        [[nodiscard]] auto GetTemplateDataForCurrentNode()
            -> AZStd::list<AtomToolsFramework::GraphTemplateFileData> const&;

        [[nodiscard]] auto ModifyTemplateDataForCurrentNode()
            -> AZStd::list<AtomToolsFramework::GraphTemplateFileData>&;

        [[nodiscard]] auto ModifyInstructionNodesForCurrentNode()
            -> AZStd::vector<GraphModel::ConstNodePtr>&;

        constexpr void ClearInstructionsForCurrentNodeAndReserveSize(
            size_t reserveAmount);

        [[nodiscard]] auto BuildDependencyTables() -> AZ::
            Outcome<SuccessTypeWhenNoReturnDataExpected, CompilationError>;

        [[nodiscard]] auto ShouldUseInstructionsFromInputNode(
            GraphModel::ConstNodePtr const& outputNode,
            GraphModel::ConstNodePtr const& inputNode,
            AZStd::vector<AZStd::string> const& inputSlotNames) const -> bool;

        [[nodiscard]] auto GetInstructionsFromSlot(
            GraphModel::ConstNodePtr const& node,
            AtomToolsFramework::DynamicNodeSlotConfig const& slotConfig,
            AZStd::vector<AZStd::pair<AZStd::string, AZStd::string>> const&
                substitutionSymbols) const -> AZStd::vector<AZStd::string>;

        [[nodiscard]] auto GetInstructionNodesInExecutionOrder(
            GraphModel::ConstNodePtr const& outputNode,
            AZStd::vector<AZStd::string> const& inputSlotNames) const
            -> AZStd::vector<GraphModel::ConstNodePtr>;

        [[nodiscard]] auto GetSymbolNameFromNode(
            GraphModel::ConstNodePtr const& node) const -> AZStd::string;

        [[nodiscard]] auto GetSymbolNameFromSlot(
            GraphModel::ConstSlotPtr slot) const -> AZStd::string;

        [[nodiscard]] auto GetLuaTypeFromSlot(
            GraphModel::ConstSlotPtr const& slot) const -> AZStd::string;

        [[nodiscard]] auto GetLuaValueFromSlot(
            GraphModel::ConstSlotPtr const& slot) const -> AZStd::string;

        [[nodiscard]] auto GetSubstitutionSymbolsFromNode(
            GraphModel::ConstNodePtr const& node) const
            -> AZStd::vector<AZStd::pair<AZStd::string, AZStd::string>>;

        [[nodiscard]] auto GetInstructionsFromConnectedNodes(
            GraphModel::ConstNodePtr const& outputNode,
            AZStd::vector<AZStd::string> const& inputSlotNames,
            AZStd::vector<GraphModel::ConstNodePtr>& instructionNodes) const
            -> AZStd::vector<AZStd::string>;

        void BuildInstructionsForCurrentNode(
            GraphModel::ConstNodePtr const& currentNode);
        auto BuildConversationAsset() -> AZ::
            Outcome<SuccessTypeWhenNoReturnDataExpected, CompilationError>;
        auto BuildConversationScript() -> AZ::
            Outcome<SuccessTypeWhenNoReturnDataExpected, CompilationError>;

        [[nodiscard]] auto GetValueFromSlot(
            GraphModel::ConstSlotPtr const slot) const -> AZStd::any;

        [[nodiscard]] auto GetValueFromSlotOrConnection(
            GraphModel::ConstSlotPtr const& slot) const -> AZStd::any;

        [[nodiscard]] auto GetAllNodesInExecutionOrder() const
            -> AZStd::vector<GraphModel::ConstNodePtr>;

        /**
         * @brief Calls the build function responsible for generating the code
         * or asset for the given node.
         *
         * It examines the given node to determine how to compile it, and then
         * calls the necessary function that knows how to build it.
         */
        void BuildNode(GraphModel::ConstNodePtr const& node);
        /**
         * @brief Gathers the data needed to build a DialogueData for the
         * ConversationAsset.
         *
         * @param dialogueGraphNode A node with the slots needed to build a
         * DialogueData.
         * @return true or CompilationError
         */
        auto BuildDialogueNode(
            GraphModel::ConstNodePtr const& dialogueGraphNode) -> AZStd::
            expected<SuccessTypeWhenNoReturnDataExpected, CompilationError>;

        /**
         * @brief Gathers the data needed by the compiler to setup dialogue
         * links.
         */
        void BuildLinkNode(GraphModel::ConstNodePtr const& linkNode);

        /**
         * @brief Builds a table of all slot values and their slot IDs.
         *
         * @note The function cannot fail, so are no reasonable errors to
         * return, even if there are zero slots.
         *
         */
        void BuildSlotValueTable();

        void BuildTemplatePathsForCurrentNode(
            GraphModel::ConstNodePtr const& currentNode);

        auto LoadTemplatesForCurrentNode() -> bool;

        [[nodiscard]] auto GetOutputPathFromTemplatePath(
            AZStd::string const& templateInputPath) const -> AZStd::string;

        void DeleteExistingFilesForCurrentNode();

        void PreProcessTemplate(
            AtomToolsFramework::GraphTemplateFileData& templateFileData);
        void PreprocessTemplatesForCurrentNode();

        [[nodiscard]] auto GetUniqueGraphName() const -> AZStd::string;

        auto ExportTemplatesMatchingRegex(AZStd::string const& pattern) -> bool;

        void ClearData();

        void ReplaceBasicSymbols(
            AtomToolsFramework::GraphTemplateFileData& templateFileData);

    private:
        AZStd::mutex m_instructionNodesForCurrentNodeMutex{};
        AZStd::mutex m_functionDefinitionsMutex{};

        GraphModel::ConstNodePtr m_currentNode{};

        /**
         * These are a list of files we've generated so far during a
         * compilation. Keeping track of them allows us to see what's been
         * generated and perform specific tasks based on what has been
         * generated.
         */
        AZStd::vector<AZStd::string> m_generatedFiles;
        AZStd::set<AZStd::string> m_includePaths{};
        // Not currently implemented.
        AZStd::vector<AZStd::string> m_classDefinitions{};
        AZStd::vector<AZStd::string> m_functionDefinitions{};
        /**
         * Holds the cached value of every node in the graph.
         *
         * When wanting the value of a node, it should be gotten from here and
         * *NOT* directly from the node pointer to ensure consistency.
         */
        SlotValueTable m_slotValueTable{};
        // Contains the UniqueId of each starting dialogue.
        StartingIdContainer m_startingIds{};
        // The names(symbols) of any nodes we've encountered.
        AZStd::vector<AZ::Name> m_names{};
        //  A map containing the NodeData of each node we've encountered.
        AZStd::map<GraphModel::ConstNodePtr, NodeData> m_nodeDataTable{};
        // Contains each template associated with the current node
        AZStd::list<AtomToolsFramework::GraphTemplateFileData>
            m_templateFileDataVecForCurrentNode{};
        // Contains the path to each template associated with the current node.
        AZStd::set<AZStd::string> m_templatePathsForCurrentNode{};
        // Nodes that contribute to the current node's instructions.
        AZStd::vector<GraphModel::ConstNodePtr>
            m_instructionNodesForCurrentNode{};
        // The template used to generate the companion script.
        AtomToolsFramework::GraphTemplateFileData m_scriptFileDataTemplate{};

        // Container of unique node configurations IDs visited on the graph to
        // collect include paths, class definitions, and function definitions.
        AZStd::unordered_set<AZ::Uuid> m_configIdsVisited;

        // This counter will be used as a suffix for graph name substitutions in
        // case multiple template nodes are included in the same graph
        [[maybe_unused]] int m_templateNodeCount = 0;
    };
} // namespace ConversationEditor
