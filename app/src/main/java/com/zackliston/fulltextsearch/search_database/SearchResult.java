package com.zackliston.fulltextsearch.search_database;

/**
 * Created by Zack Liston on 6/1/15.
 */
public class SearchResult
{
    //region Properties
    String title = null;
    String subtitle = null;
    String parentTitle = null;
    String uri = null;
    String type = null;
    String imageUri = null;
    boolean isFavorited = false;
    String fileId = null;
    String moduleId = null;

    SearchManager.IsSearchResultFavorited isSearchResultFavoritedDelegate;
    //endregion

    //region Getters
    public String getTitle() {
        return title;
    }

    public String getSubtitle() {
        return subtitle;
    }

    public String getParentTitle() {
        return parentTitle;
    }

    public String getUri() {
        return uri;
    }

    public String getType() {
        return type;
    }

    public String getImageUri() {
        return imageUri;
    }

    public boolean isFavorited() {
        return isFavorited;
    }

    public String getFileId() {
        return fileId;
    }

    public String getModuleId() {
        return moduleId;
    }
    //endregion
}
