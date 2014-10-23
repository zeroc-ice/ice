// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package com.zeroc.library;

import android.app.AlertDialog;
import android.app.Dialog;
import android.content.DialogInterface;
import android.content.Intent;
import android.os.Bundle;
import android.view.LayoutInflater;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.EditText;
import android.widget.LinearLayout;
import android.widget.TextView;

import com.zeroc.library.controller.QueryController;
import com.zeroc.library.controller.QueryModel;

public class ViewActivity extends SessionActivity
{
    private static final int EDIT_ID = Menu.FIRST;
    private static final int DELETE_ID = Menu.FIRST + 1;

    public static final int BOOK_DELETED = RESULT_FIRST_USER;
    public static final int BOOK_CHANGED = RESULT_FIRST_USER+1;

    private static final int DIALOG_DELETE = DIALOG_NEXT;
    private static final int DIALOG_RENT_BOOK = DIALOG_NEXT+1;

    private Demo.BookDescription _desc;
    private TextView _isbn;
    private TextView _title;
    private LinearLayout _authorsLayout;
    private LayoutInflater _inflater;
    private TextView _rentedBy;
    private Button _rent;

    private void updateBookDescription()
    {
        // Update the fields.
        _isbn.setText(_desc.isbn);
        _title.setText(_desc.title);
        // Remove all the authors.
        _authorsLayout.removeViews(0, _authorsLayout.getChildCount()-1);
        // Add them back.
        for(String author : _desc.authors)
        {
            TextView auth = (TextView)_inflater.inflate(R.layout.authorview, _authorsLayout, false);
            auth.setText(author);
            _authorsLayout.addView(auth, _authorsLayout.getChildCount()-1);
        }

        _rent.setEnabled(true);
        if(_desc.rentedBy.length() == 0)
        {
            _rentedBy.setVisibility(View.GONE);
            _rent.setText("Rent Book");
        }
        else
        {
            _rentedBy.setVisibility(View.VISIBLE);
            _rentedBy.setText(_desc.rentedBy);
            _rent.setText("Return Book");
        }
    }

    /** Called when the activity is first created. */
    @Override
    public void onCreate(Bundle savedInstanceState)
    {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.view);

        _authorsLayout = (LinearLayout)findViewById(R.id.authorsList);
        _isbn = (TextView)findViewById(R.id.isbn);
        _title = (TextView)findViewById(R.id.title);
        _rentedBy = (TextView)findViewById(R.id.rentedBy);

        // This button is a toggle. If the book is returned, the book is
        // returned. Otherwise, the book is rented.
        _rent = (Button)findViewById(R.id.rent);
        _rent.setOnClickListener(new OnClickListener()
        {
            public void onClick(View v)
            {
                if(_desc.rentedBy.length() == 0)
                {
                    showDialog(DIALOG_RENT_BOOK);
                }
                else
                {
                    _rent.setEnabled(false);
                    _queryController.returnBook();
                }
            }
        });

        _inflater = getLayoutInflater();
    }

    @Override
    public void onResume()
    {
        super.onResume();
        _queryController.setListener(new QueryController.Listener()
        {
            public void onDataChange(QueryModel data, boolean saved)
            {
                _rent.setEnabled(true);
                _desc = data.currentBook;
                if(_desc == null)
                {
                    finish();
                }
                else
                {
                    updateBookDescription();
                }
            }

            public void onError()
            {
                _rent.setEnabled(true);
                showDialog(DIALOG_ERROR);
            }
        });
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu)
    {
        boolean result = super.onCreateOptionsMenu(menu);
        menu.add(0, EDIT_ID, 0, R.string.menu_edit);
        menu.add(0, DELETE_ID, 0, R.string.menu_delete);
        return result;
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item)
    {
        switch (item.getItemId())
        {
        case EDIT_ID:
            startActivity(new Intent(this, EditActivity.class));
            return true;

        case DELETE_ID:
            showDialog(DIALOG_DELETE);
            return true;
        }

        return super.onOptionsItemSelected(item);
    }

    @Override
    protected Dialog onCreateDialog(final int id)
    {
        Dialog d = super.onCreateDialog(id);
        if(d != null)
        {
            return d;
        }
        switch (id)
        {
        case DIALOG_DELETE:
        {
            AlertDialog.Builder builder = new AlertDialog.Builder(this);
            builder.setTitle("Delete");
            builder.setMessage("This book will be deleted.");
            builder.setPositiveButton("OK", new DialogInterface.OnClickListener()
            {
                public void onClick(DialogInterface dialog, int whichButton)
                {
                    _queryController.deleteBook();
                }
            });
            builder.setNegativeButton("Cancel", new DialogInterface.OnClickListener()
            {
                public void onClick(DialogInterface dialog, int whichButton)
                {
                }
            });
            return builder.create();
        }

        case DIALOG_RENT_BOOK:
        {
            // This example shows how to add a custom layout to an AlertDialog
            LayoutInflater factory = LayoutInflater.from(this);
            final View entryView = factory.inflate(R.layout.rentername, null);
            final EditText renter = (EditText)entryView.findViewById(R.id.renter);
            AlertDialog.Builder builder = new AlertDialog.Builder(ViewActivity.this);
            builder.setTitle("Enter Renter");
            builder.setView(entryView);
            builder.setPositiveButton("OK", new DialogInterface.OnClickListener()
            {
                public void onClick(DialogInterface dialog, int whichButton)
                {
                    final String r = renter.getText().toString().trim();
                    if(r.length() > 0)
                    {
                        _rent.setEnabled(false);
                        _queryController.rentBook(r);
                    }
                }
            });
            builder.setNegativeButton("Cancel", new DialogInterface.OnClickListener()
            {
                public void onClick(DialogInterface dialog, int whichButton)
                {
                }
            });
            return builder.create();
        }
        }
        return null;
    }
}
