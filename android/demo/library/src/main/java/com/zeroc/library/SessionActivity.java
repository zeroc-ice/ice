// **********************************************************************
//
// Copyright (c) 2003-2015 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package com.zeroc.library;

import android.app.DialogFragment;
import android.os.Bundle;
import com.zeroc.library.controller.QueryController;
import com.zeroc.library.controller.SessionController;

import android.app.Activity;
import android.app.AlertDialog;
import android.app.Dialog;
import android.content.DialogInterface;

// The base class of any activity created after a session has been established.
public class SessionActivity extends Activity
{
    public static final String ERROR_TAG = "error";
    public static final String FATAL_TAG = "fatal";
    protected SessionController _sessionController;
    protected QueryController _queryController;

    public static class FatalDialogFragment extends DialogFragment
    {
        @Override
        public Dialog onCreateDialog(Bundle savedInstanceState)
        {
            AlertDialog.Builder builder = new AlertDialog.Builder(getActivity());
            builder.setTitle("Error")
                    .setMessage("The session was lost. Please login again.")
                    .setCancelable(false)
                    .setPositiveButton("Ok", new DialogInterface.OnClickListener()
                    {
                        public void onClick(DialogInterface dialog, int whichButton)
                        {
                            ((SessionActivity) getActivity()).fatalOk();
                        }
                    });
            return builder.create();
        }
    }
    public static class ErrorDialogFragment extends DialogFragment
    {
        public static ErrorDialogFragment newInstance(String message)
        {
            ErrorDialogFragment frag = new ErrorDialogFragment();
            Bundle args = new Bundle();
            args.putString("message", message);
            frag.setArguments(args);
            return frag;
        }

        @Override
        public Dialog onCreateDialog(Bundle savedInstanceState)
        {
            AlertDialog.Builder builder = new AlertDialog.Builder(getActivity());
            builder.setTitle("Error")
                    .setMessage(getArguments().getString("message"))
                    .setCancelable(false)
                    .setPositiveButton("Ok", new DialogInterface.OnClickListener()
                    {
                        public void onClick(DialogInterface dialog, int whichButton)
                        {
                            ((SessionActivity) getActivity()).errorOk();
                        }
                    });
            return builder.create();
        }
    }

    private void errorOk()
    {
        _queryController.clearLastError();
    }

    private void fatalOk()
    {
        LibraryApp app = (LibraryApp)getApplication();
        app.logout();
        finish();
    }

    void showDialogError()
    {
        DialogFragment dialog = ErrorDialogFragment.newInstance(_queryController.getLastError());
        dialog.show(getFragmentManager(), SessionActivity.ERROR_TAG);
    }

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
                DialogFragment dialog = new FatalDialogFragment();
                dialog.show(getFragmentManager(), FATAL_TAG);
            }
        });

        _queryController = _sessionController.getCurrentQuery();
    }

}
