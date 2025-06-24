#include "AzCore/UnitTest/UnitTest.h"
#include "SQLite/SQLiteQuery.h"
#include "Tools/ConversationCanvas/CompilationDatabase.h"
#include "Tools/ConversationCanvas/ConversationCanvasTestEnvironment.h"
#include "Tools/ConversationCanvas/Sql.h"

#include <AzTest/AzTest.h>
#include <AzToolsFramework/SQLite/SQLiteConnection.h>

namespace ConversationCanvasTest
{
    class EmptyDatabaseTests : public ::testing::Test
    {
    protected:
        void SetUp() override
        {
            m_conn.Open(":memory:", false);
            ConversationCanvas::CompilationDatabase::AddStatements(m_conn);
        }

        void TearDown() override
        {
            m_conn.Close();
        }

    protected:
        AzToolsFramework::SQLite::Connection m_conn{};
    };

    class ConfiguredDatabaseTests : public ::testing::Test
    {
    public:
        struct ExampleNode
        {
            AZ::u32 NodeId{};
        };

        struct TestDialogueData
        {
            AZStd::string_view Speaker;
            AZStd::string_view ActorText;
            AZStd::string_view Condition;
            AZStd::string_view Script;
            AZStd::string_view AudioControl;
            AZ::u32 EntryDelay;
            AZ::u32 ExitDelay;
        };

        struct TestData
        {
            ExampleNode Node;
            TestDialogueData DialogueData;
        };

        TestData Node1337 = { .Node = { .NodeId = 1337 },
                              .DialogueData = {
                                  .Speaker = "player",
                                  .ActorText = "Hello",
                                  .Condition = "GetLevel() >= 20 && IsHuman()",
                                  .Script = "return true;",
                                  .AudioControl = "",
                                  .EntryDelay = 5,
                                  .ExitDelay = 2 } };

    protected:
        void SetUp() override
        {
            m_conn.Open(":memory:", false);
            ConversationCanvas::CompilationDatabase::AddStatements(m_conn);
            ConversationCanvas::CompilationDatabase::ConfigureTables(m_conn);
        }

        void TearDown() override
        {
            m_conn.Close();
        }

    protected:
        AzToolsFramework::SQLite::Connection m_conn{};
    };

    TEST_F(EmptyDatabaseTests, SANITY_CHECK)
    {
        EXPECT_TRUE(m_conn.IsOpen());
    }

    TEST_F(EmptyDatabaseTests, SqlLiteDatabase_EmptyDb_CreateGraphTable_Success)
    {
        EXPECT_TRUE(
            ConversationCanvas::s_CreateGraphsTable.BindAndStep(m_conn));
    }

    TEST_F(EmptyDatabaseTests, SqlLiteDatabase_EmptyDb_CreateNodesTable_Success)
    {
        EXPECT_TRUE(ConversationCanvas::s_CreateNodesTable.BindAndStep(m_conn));
    }

    TEST_F(
        EmptyDatabaseTests,
        SqlLiteDatabase_EmptyDb_CreateNodeDataDialogueTable_Success)
    {
        EXPECT_TRUE(
            ConversationCanvas::s_CreateNodeDataDialogueTable.BindAndStep(
                m_conn));
    }

    TEST_F(ConfiguredDatabaseTests, InsertNode_NewNode_Success)
    {
        EXPECT_TRUE(ConversationCanvas::s_InsertNode.BindAndStep(m_conn, 1337));
    }

    TEST_F(ConfiguredDatabaseTests, InsertDialogue_WithNewNode_Success)
    {
        EXPECT_TRUE(
            ConversationCanvas::s_InsertNode.BindAndStep(
                m_conn, Node1337.Node.NodeId));
        EXPECT_TRUE(
            ConversationCanvas::s_InsertNodeDataDialogue.BindAndStep(
                m_conn,
                Node1337.Node.NodeId,
                Node1337.DialogueData.Speaker.data(),
                Node1337.DialogueData.ActorText.data(),
                Node1337.DialogueData.Condition.data(),
                Node1337.DialogueData.Script.data(),
                Node1337.DialogueData.AudioControl.data(),
                Node1337.DialogueData.EntryDelay,
                Node1337.DialogueData.ExitDelay));
    }

    TEST_F(ConfiguredDatabaseTests, InsertDialogue_WrongNode_Failure)
    {
        AZ_TEST_START_TRACE_SUPPRESSION;
        EXPECT_TRUE(
            ConversationCanvas::s_InsertNode.BindAndStep(
                m_conn, Node1337.Node.NodeId + 1));
        EXPECT_FALSE(
            ConversationCanvas::s_InsertNodeDataDialogue.BindAndStep(
                m_conn,
                Node1337.Node.NodeId,
                Node1337.DialogueData.Speaker.data(),
                Node1337.DialogueData.ActorText.data(),
                Node1337.DialogueData.Condition.data(),
                Node1337.DialogueData.Script.data(),
                Node1337.DialogueData.AudioControl.data(),
                Node1337.DialogueData.EntryDelay,
                Node1337.DialogueData.ExitDelay));
        AZ_TEST_STOP_TRACE_SUPPRESSION(1);
    }

} // namespace ConversationCanvasTest
