
#include "AtomToolsFramework/Graph/GraphCompiler.h"
#include "AtomToolsFramework/Graph/GraphTemplateFileDataCache.h"
#include "AzTest/AzTest.h"

#include "ConversationEditorTestEnvironment.h"
#include "Tools/Document/ConversationGraphCompiler.h"

AZ_UNIT_TEST_HOOK(new ConversationEditorTest::ConversationEditorTestEnvironment);

namespace ConversationEditorTest
{
    constexpr auto TestToolId{ AZ_CRC_CE("TestConversationTool") };

    class ConversationCompilerTests : public ::testing::Test
    {
    protected:
        void SetUp() override
        {
            m_fileDataCache = AZStd::make_unique<AtomToolsFramework::GraphTemplateFileDataCache>(TestToolId);
            m_compiler = AZStd::make_unique<ConversationEditor::ConversationGraphCompiler>();
        }

        void TearDown() override
        {
            m_compiler = nullptr;
            m_fileDataCache = nullptr;
        }

        AZStd::unique_ptr<AtomToolsFramework::GraphTemplateFileDataCache> m_fileDataCache{};
        AZStd::unique_ptr<ConversationEditor::ConversationGraphCompiler> m_compiler{};
    };

    TEST_F(ConversationCompilerTests, Fixture_SanityCheck)
    {
        using namespace ConversationEditor;

        EXPECT_EQ(m_compiler->GetState(), AtomToolsFramework::GraphCompiler::State::Idle);
        EXPECT_NE(m_compiler, nullptr);
    }
} // namespace ConversationEditorTest
