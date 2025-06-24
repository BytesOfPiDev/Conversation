#pragma once

#include "SQLite/SQLiteQuery.h"
namespace ConversationCanvas
{
    static auto constexpr COMPILER_LOG_NAME = "ConversationGraphCompiler";

    static auto constexpr CREATE_GRAPHS_TABLE = "CreateGraphsTable";
    static auto constexpr CREATE_GRAPHS_SQL = ""
                                              "CREATE TABLE 'graph' ("
                                              "	'id'	INTEGER,"
                                              "	'name'	INTEGER,"
                                              "	PRIMARY KEY('id' AUTOINCREMENT)"
                                              ");";
    static auto const s_CreateGraphsTable =
        AzToolsFramework::SQLite::MakeSqlQuery(
            CREATE_GRAPHS_TABLE, CREATE_GRAPHS_SQL, COMPILER_LOG_NAME);

    static auto constexpr CREATE_NODES_TABLE = "CreateNodesTable";
    static auto constexpr CREATE_NODES_TABLE_SQL =
        "CREATE TABLE 'nodes' ("
        "	'id'	INTEGER,"
        "	'node_id'	INTEGER NOT NULL UNIQUE,"
        "	'pos_x'	INTEGER,"
        "	'pos_y'	INTEGER,"
        "	PRIMARY KEY('id' AUTOINCREMENT)"
        ");";
    static auto const s_CreateNodesTable =
        AzToolsFramework::SQLite::MakeSqlQuery(
            CREATE_NODES_TABLE, CREATE_NODES_TABLE_SQL, COMPILER_LOG_NAME);

    static auto constexpr CREATE_NODE_DATA_DIALOGUE_TABLE =
        "CreateNodeDataDialogueTable";
    static auto constexpr CREATE_NODE_DATA_DIALOGUE_TABLE_SQL =
        ""
        "CREATE TABLE 'node_data_dialogue' ("
        "	'id'	INTEGER,"
        "	'node_id'	INTEGER UNIQUE,"
        "	'actor_text'	TEXT,"
        "	'condition'	TEXT,"
        "	'speaker'	TEXT,"
        "	'script'	TEXT,"
        "	'entry_delay'	INTEGER NOT NULL DEFAULT 0,"
        "	'exit_delay'	INTEGER NOT NULL DEFAULT 0,"
        "	'audio_control'	TEXT,"
        "	PRIMARY KEY('id' AUTOINCREMENT),"
        "	CONSTRAINT 'fk_node_to_nodes' FOREIGN KEY('node_id') "
        "REFERENCES 'nodes'('node_id')"
        ");";

    static auto const s_CreateNodeDataDialogueTable =
        AzToolsFramework::SQLite::MakeSqlQuery(
            CREATE_NODE_DATA_DIALOGUE_TABLE,
            CREATE_NODE_DATA_DIALOGUE_TABLE_SQL,
            COMPILER_LOG_NAME);

    static auto constexpr INSERT_NODE = "InsertNode";
    static auto constexpr INSERT_NODE_SQL =
        "INSERT INTO nodes(node_id) VALUES (:nodeId);";
    static auto const s_InsertNode = AzToolsFramework::SQLite::MakeSqlQuery(
        INSERT_NODE,
        INSERT_NODE_SQL,
        COMPILER_LOG_NAME,
        AzToolsFramework::SQLite::SqlParam<AZ::u32>(":nodeId"));

    static auto constexpr INSERT_NODE_DATA_DIALOGUE = "InsertNodeDataDialogue";
    static auto constexpr INSERT_NODE_DATA_DIALOGUE_SQL =
        "INSERT INTO "
        "node_data_dialogue("
        "node_id,actor_text,condition,speaker,script,entry_delay,exit_delay, "
        "audio_control)"
        " VALUES "
        "(:nodeId,:actorText,:condition,:speaker,:script,:entryDelay,"
        ":exitDelay,:audioControl);";
    static auto const s_InsertNodeDataDialogue =
        AzToolsFramework::SQLite::MakeSqlQuery(
            INSERT_NODE_DATA_DIALOGUE,
            INSERT_NODE_DATA_DIALOGUE_SQL,
            COMPILER_LOG_NAME,
            AzToolsFramework::SQLite::SqlParam<AZ::u32>(":nodeId"),
            AzToolsFramework::SQLite::SqlParam<char const*>(":speaker"),
            AzToolsFramework::SQLite::SqlParam<char const*>(":actorText"),
            AzToolsFramework::SQLite::SqlParam<char const*>(":condition"),
            AzToolsFramework::SQLite::SqlParam<char const*>(":script"),
            AzToolsFramework::SQLite::SqlParam<char const*>(":audioControl"),
            AzToolsFramework::SQLite::SqlParam<AZ::u32>(":entryDelay"),
            AzToolsFramework::SQLite::SqlParam<AZ::u32>(":exitDelay"));

} // namespace ConversationCanvas
