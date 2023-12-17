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
    using StartingIdContainer = AZStd::set<Conversation::DialogueId>;
    using SlotValueTable = AZStd::map<GraphModel::ConstSlotPtr, AZStd::any>;
    using SlotDialogueTable = AZStd::map<GraphModel::ConstSlotPtr, Conversation::DialogueData>;
    using SlotConditionTable = AZStd::map<GraphModel::ConstSlotPtr, Conversation::ConditionFunction>;

    // NOTE: Experimenting. Trying to find an expressive way to say what errors can happen w/o exceptions.
    AZ_ENUM( // NOLINT
        CompilationErrorCode,
        NullGraphPointer,
        FailedToSaveConversationTemplate,
        ExpectedDialogueNodeButGotNullptr,
        ExpectedDialogueNodeButGotSomethingElse,
        InvalidDialogueData);

    struct CompilationError
    {
        CompilationErrorCode const m_errorCode;
        AZStd::string const m_errorMessage;
    };

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

        auto CompileGraph(GraphModel::GraphPtr graph, AZStd::string const& graphName, AZStd::string const& graphPath) -> bool override;

        [[nodiscard]] auto GetGraphPath() const -> AZStd::string override;

        [[nodiscard]] constexpr auto GetGraphName() const -> AZStd::string_view
        {
            return m_graphName;
        }

        [[nodiscard]] auto GetSlotValueTable() const -> AZStd::map<GraphModel::ConstSlotPtr, AZStd::any> const&;

        [[nodiscard]] auto ModifySlotValueTable() -> AZStd::map<GraphModel::ConstSlotPtr, AZStd::any>&;

        [[nodiscard]] constexpr auto GetStartingIds() -> StartingIdContainer const&;

        [[nodiscard]] constexpr auto ModifyStartingIds() -> StartingIdContainer&;

        [[nodiscard]] constexpr auto GetTemplatePathsForCurrentNode() -> AZStd::set<AZStd::string> const&;

        [[nodiscard]] auto ModifyTemplatePathsForCurrentNode() -> AZStd::set<AZStd::string>&;

        [[nodiscard]] auto GetTemplateDataForCurrentNode() -> AZStd::list<AtomToolsFramework::GraphTemplateFileData> const&;

        [[nodiscard]] auto ModifyTemplateDataForCurrentNode() -> AZStd::list<AtomToolsFramework::GraphTemplateFileData>&;

        [[nodiscard]] auto ModifyInstructionNodesForCurrentNode() -> AZStd::vector<GraphModel::ConstNodePtr>&;

        constexpr void ClearInstructionsForCurrentNodeAndReserveSize(size_t reserveAmount);

        [[nodiscard]] auto BuildDependencyTables() -> AZStd::expected<AZStd::true_type, CompilationError>;

        [[nodiscard]] auto ShouldUseInstructionsFromInputNode(
            GraphModel::ConstNodePtr const& outputNode,
            GraphModel::ConstNodePtr const& inputNode,
            AZStd::vector<AZStd::string> const& inputSlotNames) const -> bool;

        [[nodiscard]] auto GetInstructionsFromSlot(
            GraphModel::ConstNodePtr const& node,
            AtomToolsFramework::DynamicNodeSlotConfig const& slotConfig,
            AZStd::vector<AZStd::pair<AZStd::string, AZStd::string>> const& substitutionSymbols) const -> AZStd::vector<AZStd::string>;

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
        auto BuildConversationAsset() -> AZStd::expected<AZStd::true_type, CompilationError>;

        [[nodiscard]] auto GetValueFromSlot(GraphModel::ConstSlotPtr const slot) const -> AZStd::any;

        [[nodiscard]] auto GetValueFromSlotOrConnection(GraphModel::ConstSlotPtr const& slot) const -> AZStd::any;

        [[nodiscard]] auto GetAllNodesInExecutionOrder() const -> AZStd::vector<GraphModel::ConstNodePtr>;

        /**
         * @brief Calls the build function responsible for generating the code or asset for the given node.
         *
         * It examines the given node to determine how to compile it, and then calls the necessary function that knows how to build it.
         */
        void BuildNode(GraphModel::ConstNodePtr const& node);
        /**
         * @brief Gathers the data needed to build a DialogueData for the ConversationAsset.
         *
         * @param dialogueGraphNode A node with the slots needed to build a DialogueData.
         * @return true or CompilationError
         */
        auto BuildDialogueNode(GraphModel::ConstNodePtr const& dialogueGraphNode) -> AZStd::expected<AZStd::true_type, CompilationError>;

        /**
         * @brief Gathers the data needed by the compiler to setup dialogue links.
         */
        void BuildLinkNode(GraphModel::ConstNodePtr const& linkNode);

        /**
         * @brief Builds a table of all slot values and their slot IDs.
         *
         * @note The function cannot fail, so are no reasonable errors to return, even if there are zero slots.
         *
         */
        void BuildSlotValueTable();

        void BuildTemplatePathsForCurrentNode(GraphModel::ConstNodePtr const& currentNode);

        auto LoadTemplatesForCurrentNode() -> bool;

        [[nodiscard]] auto GetOutputPathFromTemplatePath(AZStd::string const& templateInputPath) const -> AZStd::string;

        void DeleteExistingFilesForCurrentNode();

        void PreprocessTemplatesForCurrentNode();

        [[nodiscard]] auto GetUniqueGraphName() const -> AZStd::string;

        auto ExportTemplatesMatchingRegex(AZStd::string const& pattern) -> bool;

        void ClearData();

    private:
        AZStd::mutex m_instructionNodesForCurrentNodeMutex;

        GraphModel::ConstNodePtr m_currentNode{};

        /**
         * These are a list of files we've generated so far during a compilation. Keeping track of them allows us to see what's been
         * generated and perform specific tasks based on what has been generated.
         */
        AZStd::vector<AZStd::string> m_generatedFiles;
        // Not currently implemented.
        AZStd::set<AZStd::string> m_includePaths{};
        // Not currently implemented.
        AZStd::vector<AZStd::string> m_classDefinitions{};
        // Not currently implemented.
        AZStd::vector<AZStd::string> m_functionDefinitions{};
        SlotValueTable m_slotValueTable{};
        SlotDialogueTable m_slotDialogueTable{};
        StartingIdContainer m_startingIds{};
        AZStd::map<GraphModel::ConstNodePtr, NodeData> m_nodeDataTable{};
        AZStd::list<AtomToolsFramework::GraphTemplateFileData> m_templateFileDataVecForCurrentNode{};
        AtomToolsFramework::GraphTemplateFileData m_conversationAssetFileDataVecForCurrentCompile{};
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
