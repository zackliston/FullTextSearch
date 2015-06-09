//
//  search_database.h
//  SearchDatabase
//
//  Created by Zack Liston on 6/4/15.
//  Copyright (c) 2015 Zack Liston. All rights reserved.
//

#ifndef __SearchDatabase__search_database__
#define __SearchDatabase__search_database__

#include <stdio.h>
#include <string>
#include <map>
#include "sqlite3.h"

static const std::string kZLSearchDBIndexTableName = "searchindex";
static const std::string kZLSearchDBMetadataTableName = "searchmetadata";

static const std::string kZLSearchDBModuleIdKey = "moduleid";
static const std::string kZLSearchDBEntityIdKey = "entityid";
static const std::string kZLSearchDBLanguageKey = "language";
static const std::string kZLSearchDBBoostKey = "boost";
static const std::string kZLSearchDBWeight0Key = "weight0";
static const std::string kZLSearchDBWeight1Key = "weight1";
static const std::string kZLSearchDBWeight2Key = "weight2";
static const std::string kZLSearchDBWeight3Key = "weight3";
static const std::string kZLSearchDBWeight4Key = "weight4";

static const std::string kZLSearchDBTitleKey = "title";
static const std::string kZLSearchDBSubtitleKey = "subtitle";
static const std::string kZLSearchDBUriKey = "uri";
static const std::string kZLSearchDBTypeKey = "type";
static const std::string kZLSearchDBImageUriKey = "imageuri";

class SearchDatabase {
public:
    SearchDatabase(const char *filename);
    bool setup(char **errorMessage);
    bool close();
    bool reset_database(char **errorMessage);
    bool index_file(std::string moduleId, std::string fileId, std::string language, double boost, std::map<std::string, std::string> searchableStrings, std::map<std::string, std::string> fileMetadata, char **errorMessage);
    bool remove_file(std::string moduleId, std::string fileId, char **errorMessage);
private:
    bool setup_tables(sqlite3 * db, char **errorMessage);
    bool issue_automerge_command(sqlite3 *db, char **errorMessage);
    bool register_ranking_function(sqlite3 *db, char **errorMessage);
    
    bool does_file_exist(std::string moduleId, std::string fileId, char **errorMessage);
    sqlite3_stmt * index_insert_statement(std::string moduleId, std::string fileId, std::string language, double boost, std::map<std::string, std::string> searchableStrings, std::map<std::string, std::string> fileMetadata);
    
    bool begin_transaction(sqlite3 *db, char **errorMessage);
    bool rollback_transaction(sqlite3 *db, char **errorMessage);
    bool commit_transaction(sqlite3 *db, char **errorMessage);
    const char * dbName;
    sqlite3 *database;
};

#endif /* defined(__SearchDatabase__search_database__) */
