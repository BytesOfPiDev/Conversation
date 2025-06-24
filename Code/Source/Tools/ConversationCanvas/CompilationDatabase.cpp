#include "Tools/ConversationCanvas/CompilationDatabase.h"
#include "SQLite/SQLiteConnection.h"
#include "SQLite/SQLiteQuery.h"
#include "Tools/ConversationCanvas/Sql.h"

namespace ConversationCanvas
{
    CompilationDatabase::CompilationDatabase()
    {
        m_conn.Open(":memory:", false);
        AddStatements(m_conn);
        ConfigureTables(m_conn);
    }

    CompilationDatabase::~CompilationDatabase()
    {
        m_conn.Close();
    }

    void CompilationDatabase::AddStatements(
        AzToolsFramework::SQLite::Connection& conn)
    {
        AzToolsFramework::SQLite::AddStatement(&conn, s_CreateGraphsTable);
        AzToolsFramework::SQLite::AddStatement(&conn, s_CreateNodesTable);
        AzToolsFramework::SQLite::AddStatement(
            &conn, s_CreateNodeDataDialogueTable);
        AzToolsFramework::SQLite::AddStatement(&conn, s_InsertNode);
        AzToolsFramework::SQLite::AddStatement(&conn, s_InsertNodeDataDialogue);
    }

    void CompilationDatabase::ConfigureTables(
        AzToolsFramework::SQLite::Connection& conn)
    {
        ConversationCanvas::s_CreateGraphsTable.BindAndStep(conn);
        ConversationCanvas::s_CreateNodesTable.BindAndStep(conn);
        ConversationCanvas::s_CreateNodeDataDialogueTable.BindAndStep(conn);
    }
} // namespace ConversationCanvas
