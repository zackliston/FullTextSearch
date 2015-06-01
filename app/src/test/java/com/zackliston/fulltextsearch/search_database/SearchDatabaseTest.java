package com.zackliston.fulltextsearch.search_database;

import android.database.Cursor;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.robolectric.Robolectric;
import org.robolectric.RobolectricTestRunner;
import org.robolectric.annotation.Config;

import java.util.Arrays;
import java.util.List;

import static org.hamcrest.MatcherAssert.assertThat;
import static org.hamcrest.CoreMatchers.is;

/**
 * Created by Zack Liston on 5/27/15.
 */
@Config(manifest=Config.NONE)
@RunWith(RobolectricTestRunner.class)
public class SearchDatabaseTest
{
    private SearchDatabase searchDatabase;

    @Before
    public void setUp() {
        searchDatabase = new SearchDatabase(Robolectric.application, null);
    }

    @After
    public void tearDown() {
        searchDatabase = null;
    }

    @Test
    public void testIndexTableInitializedCorrectly() throws Exception {
        Cursor cursor = searchDatabase.getReadableDatabase().query(SearchDatabase.INDEX_TABLE_NAME, SearchDatabase.INDEX_TABLE_COLUMNS, null, null, null, null, null);

        assertThat(cursor.getColumnCount(), is(SearchDatabase.INDEX_TABLE_COLUMNS.length));

        List<String> columnArrayList = Arrays.asList(cursor.getColumnNames());

        for (String columnName: SearchDatabase.INDEX_TABLE_COLUMNS) {
            assertThat(columnArrayList.contains(columnName), is(true));
        }
    }

    @Test
    public void testMetaDataTableInitializedCorrectly() throws Exception {
        Cursor cursor = searchDatabase.getReadableDatabase().query(SearchDatabase.METADATA_TABLE_NAME, SearchDatabase.METADATA_TABLE_COLUMNS, null, null, null, null, null);

        assertThat(cursor.getColumnCount(), is(SearchDatabase.METADATA_TABLE_COLUMNS.length));

        List<String> columnArrayList = Arrays.asList(cursor.getColumnNames());

        for (String columnName: SearchDatabase.METADATA_TABLE_COLUMNS) {
            assertThat(columnArrayList.contains(columnName), is(true));
        }
    }

}