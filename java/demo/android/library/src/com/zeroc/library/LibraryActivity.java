// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package com.zeroc.library;

import android.content.Intent;
import android.os.Bundle;
import android.text.Editable;
import android.text.TextWatcher;
import android.view.KeyEvent;
import android.view.LayoutInflater;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.view.ViewGroup;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.BaseAdapter;
import android.widget.Button;
import android.widget.EditText;
import android.widget.ListView;
import android.widget.Spinner;
import android.widget.TextView;
import android.widget.AdapterView.OnItemClickListener;

import com.zeroc.library.controller.QueryController;
import com.zeroc.library.controller.QueryModel;

public class LibraryActivity extends SessionActivity
{
    private static final int ADD_ID = Menu.FIRST;
    private static final int LOGOUT_ID = Menu.FIRST + 1;

    private Button _search;
    private Spinner _searchType;
    private EditText _text;
    private ListView _searchTable;

    private QueryModel _querydata;

    private LayoutInflater _inflater;

    class LibraryListAdapter extends BaseAdapter
    {
        public int getCount()
        {
            if(_querydata == null)
            {
                return 0;
            }
            return _querydata.nrows;
        }

        public Object getItem(int position)
        {
            assert _querydata != null;
            return _querydata.books.get(position);
        }

        public long getItemId(int position)
        {
            return 0;
        }

        public View getView(int position, View convertView, ViewGroup parent)
        {
            assert _querydata != null;
            TextView text;

            // The use of the inflater ensures that the list contains a
            // well styled text view.
            if(convertView == null)
            {
                text = (TextView)_inflater.inflate(android.R.layout.simple_list_item_1, parent, false);
            }
            else
            {
                text = (TextView)convertView;
            }
            if(position > _querydata.books.size() - 1)
            {
                _queryController.getMore(position);
                text.setText("<loading>");
            }
            else
            {
                text.setText(_querydata.books.get(position).title);
            }
            return text;
        }
    };

    private LibraryListAdapter _adapter;

    private QueryController.Listener _queryListener = new QueryController.Listener()
    {
        public void onDataChange(QueryModel data, boolean saved)
        {
            _querydata = data;
            _adapter.notifyDataSetChanged();
        }

        public void onError()
        {
            showDialog(DIALOG_ERROR);
        }
    };

    // Hook the back key to logout the session.
    @Override
    public boolean onKeyDown(int keyCode, KeyEvent event)
    {
        if(keyCode == KeyEvent.KEYCODE_BACK)
        {
            logout();
            return true;
        }
        return false;
    }

    /** Called when the activity is first created. */
    @Override
    public void onCreate(Bundle savedInstanceState)
    {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.library);

        _text = (EditText)findViewById(R.id.text);
        _text.addTextChangedListener(new TextWatcher()
        {
            public void afterTextChanged(Editable s)
            {
                setSearchState();
            }

            public void beforeTextChanged(CharSequence s, int start, int count, int after)
            {
            }

            public void onTextChanged(CharSequence s, int start, int count, int after)
            {
            }
        });

        _search = (Button)findViewById(R.id.search);
        _search.setOnClickListener(new android.view.View.OnClickListener()
        {
            public void onClick(android.view.View v)
            {
                String queryString = _text.getText().toString().trim();
                // If there is no query, we're done.
                if(queryString.length() == 0)
                {
                    return;
                }

                QueryController.QueryType type = null;
                switch (_searchType.getSelectedItemPosition())
                {
                case 0: // ISBN
                    type = QueryController.QueryType.ISBN;
                    break;
                case 1: // Author
                    type = QueryController.QueryType.AUTHOR;
                    break;
                case 2: // Title
                    type = QueryController.QueryType.TITLE;
                    break;
                }
                // This immediately calls back on the _callback object with an
                // empty dataset which clears the current query list.
                _queryController = _sessionController.createQuery(_queryListener, type, queryString);
            }
        });

        _inflater = getLayoutInflater();

        _searchType = (Spinner)findViewById(R.id.searchType);
        ArrayAdapter<String> modeAdapter = new ArrayAdapter<String>(this, android.R.layout.simple_spinner_item,
                new String[] { "ISBN", "Author", "Title" });
        _searchType.setAdapter(modeAdapter);
        _adapter = new LibraryListAdapter();

        _searchTable = (ListView)findViewById(R.id.list);
        _searchTable.setAdapter(_adapter);
        _searchTable.setOnItemClickListener(new OnItemClickListener()
        {
            public void onItemClick(AdapterView<?> parent, View view, int position, long id)
            {
                // setCurrentBook returns false if the data for the current row
                // hasn't loaded yet.
                if(_queryController.setCurrentBook(position))
                {
                    startActivity(new Intent(LibraryActivity.this, ViewActivity.class));
                }
            }
        });
    }

    @Override
    public void onResume()
    {
        super.onResume();
        _queryController.setListener(_queryListener);
    }

    protected void setSearchState()
    {
        _search.setEnabled(_text.getText().toString().trim().length() > 0);
    }

    private void logout()
    {
        LibraryApp app = (LibraryApp)getApplication();
        app.logout();
        finish();
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu)
    {
        boolean result = super.onCreateOptionsMenu(menu);
        menu.add(0, ADD_ID, 0, R.string.menu_add);
        menu.add(0, LOGOUT_ID, 0, R.string.menu_logout);
        return result;
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item)
    {
        switch (item.getItemId())
        {
        case ADD_ID:
            _queryController.setCurrentBook(QueryController.NEW_BOOK);
            startActivity(new Intent(LibraryActivity.this, EditActivity.class));
            return true;

        case LOGOUT_ID:
            logout();
            return true;
        }

        return super.onOptionsItemSelected(item);
    }
}
