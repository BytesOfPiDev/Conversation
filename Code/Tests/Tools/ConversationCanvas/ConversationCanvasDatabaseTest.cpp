#include "SQLite/SQLiteQuery.h"
#include "Tools/ConversationCanvas/CompilationDatabase.h"
#include "Tools/ConversationCanvas/ConversationCanvasTestEnvironment.h"
#include "Tools/ConversationCanvas/Sql.h"

#include <AzTest/AzTest.h>
#include <AzToolsFramework/SQLite/SQLiteConnection.h>

namespace ConversationCanvasTest
{
    class ConversationCanvasDatabaseTests : public ::testing::Test
    {
    protected:
        void SetUp() override
        {
            m_conn.Open(":memory:", false);
        }

        void TearDown() override
        {
            m_conn.Close();
        }

    protected:
        AzToolsFramework::SQLite::Connection m_conn{};
    };

    TEST_F(ConversationCanvasDatabaseTests, SANITY_CHECK)
    {
        EXPECT_TRUE(m_conn.IsOpen());
    }

    TEST_F(
        ConversationCanvasDatabaseTests,
        SqlLiteDatabase_CreateGraphsTable_Success)
    {
        ConversationCanvas::CompilationDatabase{};
    }
} // namespace ConversationCanvasTest
