#pragma once

#include "SQLite/SQLiteQuery.h"
namespace ConversationCanvas
{
    static auto constexpr COMPILER_LOG_NAME = "ConversationGraphCompiler";

    static auto constexpr CREATE_GRAPHS_TABLE = "CreateGraphsTable";
    static auto constexpr CREATE_GRAPHS_SQL = ""
                                              "CREATE TABLE 'graphs' ("
                                              "	'id'	INTEGER,"
                                              "	'name'	INTEGER,"
                                              "	'path'	TEXT,"
                                              "	PRIMARY KEY('id' AUTOINCREMENT)"
                                              ");";
    static auto const s_CreateGraphsTable =
        AzToolsFramework::SQLite::MakeSqlQuery(
            CREATE_GRAPHS_TABLE, CREATE_GRAPHS_SQL, COMPILER_LOG_NAME);

    static auto constexpr CREATE_NODES_TABLE = "CreateNodesTable";
    static auto constexpr CREATE_NODES_TABLE_SQL =
        ""
        "CREATE TABLE 'nodes' ("
        "	'id'	INTEGER,"
        "	'graph'	INTEGER,"
        "	'graph_node_id'	INTEGER,"
        "	'pos_x'	INTEGER,"
        "	'pos_y'	INTEGER,"
        "	PRIMARY KEY('id'),"
        "	FOREIGN KEY('graph') REFERENCES 'graphs'('id')"
        ");";
    static auto const s_CreateNodesTable =
        AzToolsFramework::SQLite::MakeSqlQuery(
            CREATE_NODES_TABLE, CREATE_NODES_TABLE_SQL, COMPILER_LOG_NAME);
} // namespace ConversationCanvas
