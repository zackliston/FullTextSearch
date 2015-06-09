//
//  main.cpp
//  SearchDatabase
//
//  Created by Zack Liston on 6/4/15.
//  Copyright (c) 2015 Zack Liston. All rights reserved.
//

#include <iostream>
#include "search_database.h"

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
    
    return 0;
}
