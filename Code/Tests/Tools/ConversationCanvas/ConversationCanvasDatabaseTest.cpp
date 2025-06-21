#include "Tools/ConversationCanvas/ConversationCanvasTestEnvironment.h"

#include <AzTest/AzTest.h>
#include <AzToolsFramework/SQLite/SQLiteConnection.h>

namespace ConversationCanvasTest
{
    class ConversationCanvasDatabaseTests : public ::testing::Test
    {
    protected:
        void SetUp() override
        {
            //       m_conn.Open(":memory:", false);
        }

        void TearDown() override
        {
            //      m_conn.Close();
        }

    private:
        AzToolsFramework::SQLite::Connection m_conn{};
    };
} // namespace ConversationCanvasTest
