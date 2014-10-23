// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package com.zeroc.testsuite;

import java.util.ArrayList;
import java.util.List;

import android.app.AlertDialog;
import android.app.Dialog;
import android.app.ListActivity;
import android.app.ProgressDialog;
import android.content.DialogInterface;
import android.content.Intent;
import android.os.Bundle;
import android.view.View;
import android.widget.ArrayAdapter;
import android.widget.CheckBox;
import android.widget.CompoundButton;
import android.widget.ListView;
import android.widget.CompoundButton.OnCheckedChangeListener;

public class TestSuite extends ListActivity
{
    private static final int DIALOG_INITIALIZING = 1;
    private static final int DIALOG_SSL_FAILED = 2;
    private List<String> _tests = new ArrayList<String>();

    @Override
    public void onCreate(Bundle savedInstanceState)
    {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.main);

        final TestApp app = (TestApp)getApplication();
        _tests.addAll(app.getTestNames());
        final ArrayAdapter<String> adapter =
            new ArrayAdapter<String>(this, android.R.layout.simple_list_item_1, _tests);
        setListAdapter(adapter);
        app.setSSLInitializationListener(new TestApp.SSLInitializationListener()
        {
            private boolean dismiss = false;

            public void onComplete()
            {
                if(dismiss)
                {
                    dismissDialog(DIALOG_INITIALIZING);
                }
            }

            public void onError()
            {
                if(dismiss)
                {
                    dismissDialog(DIALOG_INITIALIZING);
                }
                showDialog(DIALOG_SSL_FAILED);
            }

            public void onWait()
            {
                // Show the initializing dialog.
                dismiss = true;
                showDialog(DIALOG_INITIALIZING);
            }
        });
        CheckBox secure = (CheckBox)findViewById(R.id.secure);
        if(app.isSSLSupported())
        {
            secure.setOnCheckedChangeListener(new OnCheckedChangeListener()
            {
                public void onCheckedChanged(CompoundButton buttonView, boolean isChecked)
                {
                    app.setSSL(isChecked);
                    _tests.clear();
                    _tests.addAll(app.getTestNames());
                    adapter.notifyDataSetChanged();
                }
            });
        }
        else
        {
            secure.setEnabled(false);
        }
        
        CheckBox ipv6 = (CheckBox)findViewById(R.id.ipv6);
        ipv6.setOnCheckedChangeListener(new OnCheckedChangeListener()
        {
            public void onCheckedChanged(CompoundButton buttonView, boolean isChecked)
            {
                app.setIPv6(isChecked);
            }
        });
    }

    protected void onListItemClick(ListView l, View v, int position, long id)
    {
        TestApp app = (TestApp)getApplication();
        app.startTest(position);
        startActivity(new Intent(this, TestContainer.class));
    }

    @Override
    protected Dialog onCreateDialog(int id)
    {
        switch (id)
        {
        case DIALOG_INITIALIZING:
        {
            ProgressDialog dialog = new ProgressDialog(this);
            dialog.setTitle("Initializing");
            dialog.setMessage("Please wait while initializing SSL...");
            dialog.setIndeterminate(true);
            dialog.setCancelable(false);
            return dialog;
        }

        case DIALOG_SSL_FAILED:
        {
            AlertDialog.Builder builder = new AlertDialog.Builder(this);
            builder.setTitle("Error");
            builder.setMessage("SSL Initialization failed");
            builder.setCancelable(false);
            builder.setPositiveButton("Ok", new DialogInterface.OnClickListener()
            {
                public void onClick(DialogInterface dialog, int whichButton)
                {
                    finish();
                }
            });
            return builder.create();
        }
        }
        return null;
    }
}
