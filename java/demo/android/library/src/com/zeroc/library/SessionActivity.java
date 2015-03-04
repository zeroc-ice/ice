// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package com.zeroc.library;

import com.zeroc.library.controller.QueryController;
import com.zeroc.library.controller.SessionController;

import android.app.Activity;
import android.app.AlertDialog;
import android.app.Dialog;
import android.content.DialogInterface;

// The base class of any activity created after a session has been established.
public class SessionActivity extends Activity
{
    protected static final int DIALOG_ERROR = 0;
    protected static final int DIALOG_FATAL = 1;
    protected static final int DIALOG_NEXT = 2;

    protected SessionController _sessionController;
    protected QueryController _queryController;

    @Override
    public void onResume()
    {
        super.onResume();

        LibraryApp app = (LibraryApp)getApplication();
        _sessionController = app.getSessionController();
        if(_sessionController == null)
        {
            finish();
            return;
        }

        _sessionController.setSessionListener(new SessionController.Listener()
        {
            public void onDestroy()
            {
                showDialog(DIALOG_FATAL);
            }
        });

        _queryController = _sessionController.getCurrentQuery();
    }

    @Override
    protected Dialog onCreateDialog(final int id)
    {
        switch (id)
        {
        case DIALOG_ERROR:
        {
            AlertDialog.Builder builder = new AlertDialog.Builder(this);
            builder.setTitle("Error");
            builder.setMessage(""); // Filled in onPrepareDialog.
            builder.setCancelable(false);
            builder.setPositiveButton("Ok", new DialogInterface.OnClickListener()
            {
                public void onClick(DialogInterface dialog, int whichButton)
                {
                    _queryController.clearLastError();
                }
            });
            return builder.create();
        }

        case DIALOG_FATAL:
        {
            AlertDialog.Builder builder = new AlertDialog.Builder(this);
            builder.setTitle("Error");
            builder.setMessage("The session was lost. Please login again.");
            builder.setCancelable(false);
            builder.setPositiveButton("Ok", new DialogInterface.OnClickListener()
            {
                public void onClick(DialogInterface dialog, int whichButton)
                {
                    LibraryApp app = (LibraryApp)getApplication();
                    app.logout();
                    finish();
                }
            });
            return builder.create();
        }
        }

        return null;
    }

    @Override
    protected void onPrepareDialog(int id, Dialog dialog)
    {
        super.onPrepareDialog(id, dialog);
        if(id == DIALOG_ERROR)
        {
            AlertDialog alert = (AlertDialog)dialog;
            alert.setMessage(_queryController.getLastError());
        }
    }
}
