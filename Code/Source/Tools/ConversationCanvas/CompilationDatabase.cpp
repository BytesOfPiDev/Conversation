#include "Tools/ConversationCanvas/CompilationDatabase.h"
#include "SQLite/SQLiteQuery.h"
#include "Tools/ConversationCanvas/Sql.h"
#include <cstdlib>

namespace ConversationCanvas
{
    CompilationDatabase::CompilationDatabase()
    {
        m_conn.Open(":memory:", false);
        AddStatements();

        Init();
    }

    CompilationDatabase::~CompilationDatabase()
    {
        m_conn.Close();
    }

    void CompilationDatabase::AddStatements()
    {
        AzToolsFramework::SQLite::AddStatement(&m_conn, s_CreateGraphsTable);
        AzToolsFramework::SQLite::AddStatement(&m_conn, s_CreateNodesTable);
    }

    void CompilationDatabase::Init()
    {
        ConversationCanvas::s_CreateGraphsTable.BindAndStep(m_conn);
        ConversationCanvas::s_CreateNodesTable.BindAndStep(m_conn);
    }
} // namespace ConversationCanvas
