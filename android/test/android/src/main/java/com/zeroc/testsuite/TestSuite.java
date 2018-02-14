// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package com.zeroc.testsuite;

import java.util.ArrayList;
import java.util.List;

import android.app.*;
import android.content.DialogInterface;
import android.content.Intent;
import android.os.Bundle;
import android.view.View;
import android.widget.*;
import android.widget.CompoundButton.OnCheckedChangeListener;

public class TestSuite extends ListActivity
{
    public static final String INITIALIZE_TAG = "initialize";
    public static final String FAILED_TAG = "failed";

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
                    DialogFragment f= (DialogFragment)getFragmentManager().findFragmentByTag(INITIALIZE_TAG);
                    if(f != null)
                    {
                        f.dismiss();
                    }
                }
            }

            public void onError()
            {
                if(dismiss)
                {
                    DialogFragment f= (DialogFragment)getFragmentManager().findFragmentByTag(INITIALIZE_TAG);
                    if(f != null)
                    {
                        f.dismiss();
                    }
                }
                DialogFragment dialog = new SSLFailedDialogFragment();
                dialog.show(getFragmentManager(), FAILED_TAG);
            }

            public void onWait()
            {
                // Show the initializing dialog.
                dismiss = true;
                DialogFragment dialog = new InitializeDialogFragment();
                dialog.show(getFragmentManager(), INITIALIZE_TAG);
            }
        });
        Spinner mode = (Spinner)findViewById(R.id.mode);
        ArrayAdapter<String> modeAdapter = new ArrayAdapter<String>(this, android.R.layout.simple_spinner_item,
                new String[] { "TCP", "SSL", "WS", "WSS"});
        mode.setAdapter(modeAdapter);
        mode.setOnItemSelectedListener(new android.widget.AdapterView.OnItemSelectedListener()
        {
            public void onItemSelected(AdapterView<?> parent, View view, int position, long id)
            {
                app.setMode((int)id);
                _tests.clear();
                _tests.addAll(app.getTestNames());
                adapter.notifyDataSetChanged();
            }

            public void onNothingSelected(AdapterView<?> arg0)
            {
            }
        });
        if(savedInstanceState == null)
        {
            mode.setSelection(0);
        }
        app.setMode((int)mode.getSelectedItemId());

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

    public static class InitializeDialogFragment extends DialogFragment
    {
        @Override
        public Dialog onCreateDialog(Bundle savedInstanceState)
        {
            ProgressDialog dialog = new ProgressDialog(getActivity());
            dialog.setTitle("Initializing");
            dialog.setMessage("Please wait while initializing SSL...");
            dialog.setIndeterminate(true);
            dialog.setCancelable(false);
            return dialog;
        }
    }

    public static class SSLFailedDialogFragment extends DialogFragment
    {
        @Override
        public Dialog onCreateDialog(Bundle savedInstanceState)
        {
            AlertDialog.Builder builder = new AlertDialog.Builder(getActivity());
            builder.setTitle("Error")
                    .setMessage("SSL Initialization failed")
                    .setCancelable(false)
                    .setPositiveButton("Ok", new DialogInterface.OnClickListener()
                    {
                        public void onClick(DialogInterface dialog, int whichButton)
                        {
                            getActivity().finish();
                        }
                    });
            return builder.create();
        }
    }
}
