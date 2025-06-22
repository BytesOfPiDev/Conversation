#pragma once

#include <AzToolsFramework/SQLite/SQLiteConnection.h>

namespace ConversationCanvas
{
    class CompilationDatabase
    {
    public:
        CompilationDatabase();
        ~CompilationDatabase();

    protected:
        void AddStatements();
        void Init();

    private:
        AzToolsFramework::SQLite::Connection m_conn{};
    };
} // namespace ConversationCanvas
