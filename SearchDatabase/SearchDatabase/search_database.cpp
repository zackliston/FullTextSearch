//
//  search_database.cpp
//  SearchDatabase
//
//  Created by Zack Liston on 6/4/15.
//  Copyright (c) 2015 Zack Liston. All rights reserved.
//

#include "search_database.h"
#include "sqlite3.h"
#include "Rank.h"
#include <iostream>
#include <exception>
#include <unistd.h>

SearchDatabase::SearchDatabase(const char *filename) {
    dbName = filename;
}

#pragma mark - Ranking
static void rankWrapperFunction(sqlite3_context *context, int argc, sqlite3_value **argv) {
    if(argc!=(2)) {
        sqlite3_result_error(context, "wrong number of arguments to function rank()", -1);
        return;
    }
    // rank method parameters
    unsigned int *aMatchinfo = (unsigned int *)sqlite3_value_blob(argv[0]);
    double boost = sqlite3_value_double(argv[1]);
    double weights[5] = {1,2,10,20,50};
    
    double score = rank(aMatchinfo, boost, weights);
    
    sqlite3_result_double(context, score);
    return;
}

#pragma mark - Setup
bool SearchDatabase::setup(char **errorMessage) {
    int returnValue = sqlite3_open_v2(dbName, &database, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL);
    if (returnValue == SQLITE_OK) {
        bool success = setup_tables(database, errorMessage);
        if (!success) {
            sqlite3_close(database);
            return false;
        }
        success = issue_automerge_command(database, errorMessage);
        if (!success) {
            sqlite3_close(database);
            return false;
        }
        
        success = register_ranking_function(database, errorMessage);
        if (!success) {
            sqlite3_close(database);
            return false;
        }
        
        return true;
    } else {
        const char *dbError = sqlite3_errmsg(database);
        printf("Error opening search database %s",  dbError);
        sqlite3_close(database);
        *errorMessage = strdup(dbError);
        return false;
    }
}

bool SearchDatabase::setup_tables(sqlite3 * db, char **errorMessage) {
    std::string create_index_table_command = "CREATE VIRTUAL TABLE IF NOT EXISTS " + kZLSearchDBIndexTableName + " USING FTS4 (" +
    kZLSearchDBModuleIdKey + " TEXT NOT NULL, " +
    kZLSearchDBEntityIdKey + " TEXT NOT NULL, " +
    kZLSearchDBLanguageKey + " TEXT NOT NULL, " +
    kZLSearchDBBoostKey + " FLOAT NOT NULL, " +
    kZLSearchDBWeight0Key + " TEXT, " +
    kZLSearchDBWeight1Key + " TEXT, " +
    kZLSearchDBWeight2Key + " TEXT, " +
    kZLSearchDBWeight3Key + " TEXT, " +
    kZLSearchDBWeight4Key + " TEXT, PRIMARY KEY (" + kZLSearchDBModuleIdKey + ", " + kZLSearchDBEntityIdKey +"));";
    
    std::string create_meta_table_command = "CREATE TABLE IF NOT EXISTS " + kZLSearchDBMetadataTableName + " (" +
    kZLSearchDBModuleIdKey + " TEXT NOT NULL," +
    kZLSearchDBEntityIdKey + " TEXT NOT NULL," +
    kZLSearchDBTitleKey + " TEXT," +
    kZLSearchDBSubtitleKey + " TEXT," +
    kZLSearchDBUriKey + " TEXT," +
    kZLSearchDBTypeKey + " TEXT," +
    kZLSearchDBImageUriKey + " TEXT, PRIMARY KEY ("+ kZLSearchDBModuleIdKey +","+ kZLSearchDBEntityIdKey + "));";
    
    std::string combined_command = create_index_table_command + " " + create_meta_table_command;
    
    int returnValue = sqlite3_exec(db, combined_command.c_str(), NULL, NULL, errorMessage);
    if (returnValue == SQLITE_OK) {
        return true;
    } else {
        printf("Error creating index table %s", *errorMessage);
        return false;
    }
}

bool SearchDatabase::issue_automerge_command(sqlite3 *db, char **errorMessage) {
    std::string automerge_command = "INSERT INTO " + kZLSearchDBIndexTableName + "(" + kZLSearchDBIndexTableName + ") VALUES('automerge=2');";
    
    int returnValue = sqlite3_exec(db, automerge_command.c_str(), NULL, NULL, errorMessage);
    if (returnValue == SQLITE_OK) {
        return true;
    }
    return false;
}

bool SearchDatabase::register_ranking_function(sqlite3 *db, char **errorMessage) {
    int returnValue = sqlite3_create_function(db, "rank", 2, SQLITE_UTF8, NULL, &rankWrapperFunction, NULL, NULL);
    if (returnValue == SQLITE_OK) {
        return true;
    } else {
        *errorMessage = strdup(sqlite3_errmsg(db));
        return false;
    }
}

#pragma mark - Close

bool SearchDatabase::close() {
    int returnValue = sqlite3_close_v2(database);
    return (returnValue == SQLITE_OK);
}

#pragma mark - Searching

bool SearchDatabase::search(std::string searchText, int limit, int offset, bool preferPhraseSearching, SearchResult searchResults[], int * numberOfResults, std::string suggestions[], int * numberOfSuggestions) {
 
    
    
    return false;
}

#pragma mark - Indexing

bool SearchDatabase::index_file(std::string moduleId, std::string fileId, std::string language, double boost, std::map<std::string, std::string> searchableStrings, std::map<std::string, std::string> fileMetadata, char **errorMessage) {
    bool doesFileExist = does_file_exist(moduleId, fileId, errorMessage);
    
    if (doesFileExist) {
        bool success = remove_file(moduleId, fileId, errorMessage);
        if (!success) {
            return false;
        }
    }
    
    bool beginTransactionSuccess = begin_transaction(database, errorMessage);
    if (!beginTransactionSuccess) {
        return false;
    }    
    
    sqlite3_stmt * indexInsertStatement = index_insert_statement(moduleId, fileId, language, boost, searchableStrings);
    int insertReturnValue = sqlite3_step(indexInsertStatement);
    sqlite3_finalize(indexInsertStatement);
    
    if (insertReturnValue != SQLITE_DONE) {
        *errorMessage = strdup(sqlite3_errmsg(database));
        rollback_transaction(database, NULL);
        return false;
    }
    
    sqlite3_stmt * metaInsertStatement = meta_insert_statement(moduleId, fileId, fileMetadata);
    insertReturnValue = sqlite3_step(metaInsertStatement);
    sqlite3_finalize(metaInsertStatement);
    
    if (insertReturnValue != SQLITE_DONE) {
        *errorMessage = strdup(sqlite3_errmsg(database));
        rollback_transaction(database, NULL);
        return false;
    }
    
    return commit_transaction(database, errorMessage);
}

#pragma mark - Removing

bool SearchDatabase::remove_file(std::string moduleId, std::string fileId, char **errorMessage) {
    bool beginTransactionSuccess = begin_transaction(database, errorMessage);
    if (!beginTransactionSuccess) {
        return false;
    }
    std::string indexDeleteCommand = "DELETE FROM " + kZLSearchDBIndexTableName + " WHERE " + kZLSearchDBModuleIdKey + " = ? AND " + kZLSearchDBEntityIdKey + " = ?";
    sqlite3_stmt * indexDeleteStatement;
    sqlite3_prepare_v2(database, indexDeleteCommand.c_str(), -1, &indexDeleteStatement, NULL);
    sqlite3_bind_text(indexDeleteStatement, 1, moduleId.c_str(), -1, NULL);
    sqlite3_bind_text(indexDeleteStatement, 2, fileId.c_str(), -1, NULL);
    
    int deleteReturnValue = sqlite3_step(indexDeleteStatement);
    sqlite3_finalize(indexDeleteStatement);
    if (deleteReturnValue != SQLITE_DONE) {
        *errorMessage = strdup(sqlite3_errmsg(database));
        rollback_transaction(database, NULL);
        return false;
    }
    
    std::string metaDeleteCommand = "DELETE FROM " + kZLSearchDBMetadataTableName + " WHERE " + kZLSearchDBModuleIdKey + " = ? AND " + kZLSearchDBEntityIdKey + " = ?";
    sqlite3_stmt * metaDeleteStatement;
    sqlite3_prepare_v2(database, metaDeleteCommand.c_str(), -1, &metaDeleteStatement, NULL);
    sqlite3_bind_text(metaDeleteStatement, 1, moduleId.c_str(), -1, NULL);
    sqlite3_bind_text(metaDeleteStatement, 2, fileId.c_str(), -1, NULL);
    
    deleteReturnValue = sqlite3_step(metaDeleteStatement);
    sqlite3_finalize(metaDeleteStatement);
    
    if (deleteReturnValue != SQLITE_DONE) {
        *errorMessage = strdup(sqlite3_errmsg(database));
        rollback_transaction(database, NULL);
        return false;
    }
    
    return commit_transaction(database, errorMessage);
}

#pragma mark - Reset

bool SearchDatabase::reset_database(char **errorMessage) {
    std::string deleteCommand = "DROP TABLE IF EXISTS " + kZLSearchDBIndexTableName + "; " +
    "DROP TABLE IF EXISTS " + kZLSearchDBMetadataTableName + ";";
    
    int returnValue = sqlite3_exec(database, deleteCommand.c_str(), NULL, NULL, errorMessage);
    if (returnValue != SQLITE_OK) {
        return false;
    }
    return setup(errorMessage);
}

#pragma mark - Helpers

sqlite3_stmt * SearchDatabase::index_insert_statement(std::string moduleId, std::string fileId, std::string language, double boost, std::map<std::string, std::string> searchableStrings) {
    std::string insertCommand = "INSERT INTO " + kZLSearchDBIndexTableName + "(" + kZLSearchDBModuleIdKey + ", " + kZLSearchDBEntityIdKey + ", " + kZLSearchDBLanguageKey + ", " + kZLSearchDBBoostKey + ", " + kZLSearchDBWeight0Key + ", " + kZLSearchDBWeight1Key + ", " + kZLSearchDBWeight2Key + ", " + kZLSearchDBWeight3Key + ", " + kZLSearchDBWeight4Key + ") VALUES(?, ?, ?, ?, ?, ?, ?, ?, ?);";
    
    sqlite3_stmt * statement;
    sqlite3_prepare_v2(database, insertCommand.c_str(), -1, &statement, NULL);
    
    sqlite3_bind_text(statement, 1, moduleId.c_str(), -1, NULL);
    sqlite3_bind_text(statement, 2, fileId.c_str(), -1, NULL);
    sqlite3_bind_text(statement, 3, language.c_str(), -1, NULL);
    sqlite3_bind_double(statement, 4, boost);
    sqlite3_bind_text(statement, 5, searchableStrings[kZLSearchDBWeight0Key].c_str(), -1, NULL);
    sqlite3_bind_text(statement, 6, searchableStrings[kZLSearchDBWeight1Key].c_str(), -1, NULL);
    sqlite3_bind_text(statement, 7, searchableStrings[kZLSearchDBWeight2Key].c_str(), -1, NULL);
    sqlite3_bind_text(statement, 8, searchableStrings[kZLSearchDBWeight3Key].c_str(), -1, NULL);
    sqlite3_bind_text(statement, 9, searchableStrings[kZLSearchDBWeight4Key].c_str(), -1, NULL);
    
    return statement;
}

sqlite3_stmt * SearchDatabase::meta_insert_statement(std::string moduleId, std::string fileId, std::map<std::string, std::string> metadata) {
    std::string insertCommand = "INSERT INTO " + kZLSearchDBMetadataTableName + " (" + kZLSearchDBModuleIdKey + ", " + kZLSearchDBEntityIdKey + ", " + kZLSearchDBTitleKey + ", " + kZLSearchDBSubtitleKey + ", " + kZLSearchDBTypeKey + ", " + kZLSearchDBUriKey + ", " + kZLSearchDBImageUriKey + ") VALUES (?, ?, ?, ?, ?, ?, ?);";
    
    sqlite3_stmt * statement;
    sqlite3_prepare_v2(database, insertCommand.c_str(), -1, &statement, NULL);
    sqlite3_bind_text(statement, 1, moduleId.c_str(), -1, NULL);
    sqlite3_bind_text(statement, 2, fileId.c_str(), -1, NULL);
    sqlite3_bind_text(statement, 3, metadata[kZLSearchDBTitleKey].c_str(), -1, NULL);
    sqlite3_bind_text(statement, 4, metadata[kZLSearchDBSubtitleKey].c_str(), -1, NULL);
    sqlite3_bind_text(statement, 5, metadata[kZLSearchDBTypeKey].c_str(), -1, NULL);
    sqlite3_bind_text(statement, 6, metadata[kZLSearchDBUriKey].c_str(), -1, NULL);
    sqlite3_bind_text(statement, 7, metadata[kZLSearchDBImageUriKey].c_str(), -1, NULL);

    return statement;
}

bool SearchDatabase::begin_transaction(sqlite3 *db, char **errorMessage) {
    int beginReturnValue = sqlite3_exec(db, "BEGIN", NULL, NULL, errorMessage);
    if (beginReturnValue == SQLITE_OK) {
        return true;
    }
    return false;
}

bool SearchDatabase::rollback_transaction(sqlite3 *db, char **errorMessage) {
    int returnValue = sqlite3_exec(db, "ROLLBACK", NULL, NULL, errorMessage);
    if (returnValue == SQLITE_OK) {
        return true;
    }
    return false;
}

bool SearchDatabase::commit_transaction(sqlite3 *db, char **errorMessage) {
    int returnValue;
    do {
        returnValue = sqlite3_exec(db, "COMMIT", NULL, NULL, errorMessage);
    } while (returnValue == SQLITE_BUSY);
    if (returnValue == SQLITE_OK) {
        return true;
    }
    return false;
}

bool SearchDatabase::does_file_exist(std::string moduleId, std::string fileId, char **errorMessage) {
    std::string indexQuery = "SELECT * FROM " + kZLSearchDBIndexTableName + " WHERE " + kZLSearchDBModuleIdKey + " = ? AND " + kZLSearchDBEntityIdKey + " = ?";
    
    sqlite3_stmt * statement;
    sqlite3_prepare_v2(database, indexQuery.c_str(), -1, &statement, NULL);
    sqlite3_bind_text(statement, 1, moduleId.c_str(), -1, NULL);
    sqlite3_bind_text(statement, 2, fileId.c_str(), -1, NULL);
    
    int returnValue = sqlite3_step(statement);
    while (returnValue == SQLITE_BUSY) {
        usleep(100);
        returnValue = sqlite3_step(statement);
    }
    
    sqlite3_finalize(statement);
    
    if (returnValue == SQLITE_ROW) {
        return true;
    } else if (returnValue == SQLITE_ERROR) {
        *errorMessage = strdup(sqlite3_errmsg(database));
    }
    return false;
}

std::string SearchDatabase::formatSearchText(std::string *searchText) {
    const std::string whitespace = " \t";
    const auto strBegin = searchText->find_first_not_of(whitespace);
    if (strBegin == std::string::npos)
        return ""; // no content
    
    const auto strEnd = searchText->find_last_not_of(whitespace);
    const auto strRange = strEnd - strBegin + 1;
    
    return searchText->substr(strBegin, strRange) + "*";
}

std::string SearchDatabase::stringForPhraseSearching(std::string *searchText) {
    return ("\""+ *searchText +"\"");
}

