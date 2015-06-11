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
    SearchDatabase database = SearchDatabase("filename");
    
    char *errorMessage;
    bool success = database.setup(&errorMessage);    

    if (success) {
        printf("succeeded\n");
    } else {
        printf("failure %s\n", errorMessage);
    }
    
    char *resetErrorMessage;
    bool resetSuccess = database.reset_database(&resetErrorMessage);
    if (resetSuccess) {
        printf("Reset was successful\n");
    } else {
        printf("Reset failed %s\n", resetErrorMessage);
    }
    
    std::map<std::string, std::string> searchable;
    searchable[kZLSearchDBWeight4Key] = "this is a hello message";
    std::map<std::string, std::string> meta;
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
