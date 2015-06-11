//
//  main.cpp
//  SearchDatabase
//
//  Created by Zack Liston on 6/4/15.
//  Copyright (c) 2015 Zack Liston. All rights reserved.
//

#include <iostream>
#include "search_database.h"
#include <map>

int main(int argc, const char * argv[]) {
    // insert code here...
    SearchDatabase database = SearchDatabase("/Users/agilediagnosis/Desktop/database");
    
    char *errorMessage;
    bool success = database.setup(&errorMessage);    

    if (success) {
        printf("succeeded\n");
    } else {
        printf("failure %s\n", errorMessage);
    }
    
    
    
    std::map<std::string, std::string> searchable;
    searchable[kZLSearchDBWeight4Key] = "hello";
    searchable[kZLSearchDBWeight3Key] = "hello";
    searchable[kZLSearchDBWeight2Key] = "hello";
    searchable[kZLSearchDBWeight1Key] = "hello";
    searchable[kZLSearchDBWeight0Key] = "hello";
    std::map<std::string, std::string> meta;
    meta[kZLSearchDBTitleKey] = "The title";
    meta[kZLSearchDBUriKey] = "www.url.com";
    
    std::string moduleId = "module";
    std::string fileId = "fileId";
    std::string language = "en";
    double boost = 342.33;
    
    char *insertErrorMessage;
    
    bool indexSuccess = database.index_file(moduleId, fileId, language, boost, searchable, meta, &insertErrorMessage);
    if (indexSuccess) {
        printf("Successfully indexed\n");
    } else {
        printf("Error indexing %s\n", insertErrorMessage);
    }
    
    bool doesFileExist = database.does_file_exist(moduleId, fileId, &errorMessage);
    if (doesFileExist) {
        printf("file is indexed\n");
    } else {
        printf("file not indexed %s\n", errorMessage);
    }
    
    SearchResult searchResults[10];
    int numberOfResults;
    bool searchSuccess = database.search("hello", 10, 0, true, searchResults, &numberOfResults, NULL, NULL, &errorMessage);
    if (searchSuccess) {
        printf("Number of results %i\n", numberOfResults);
    } else {
        printf("Error searching %s\n", errorMessage);
    }
    
    return 0;
}
