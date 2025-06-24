#pragma once

#include <AzToolsFramework/SQLite/SQLiteConnection.h>

namespace ConversationCanvas
{
    class CompilationDatabase
    {
    public:
        CompilationDatabase();
        ~CompilationDatabase();

        static void AddStatements(AzToolsFramework::SQLite::Connection& conn);
        static void ConfigureTables(AzToolsFramework::SQLite::Connection& conn);

        void InsertNode(AZ::u32 nodeId)
        {
        }

        void UpdateDialogueNodeActorText(AZ::u32 nodeId, AZStd::string_view)
        {
        }

    private:
        AzToolsFramework::SQLite::Connection m_conn{};
    };
} // namespace ConversationCanvas
