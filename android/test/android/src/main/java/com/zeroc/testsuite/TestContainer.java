// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package com.zeroc.testsuite;

import java.util.LinkedList;

import android.app.ListActivity;
import android.os.Bundle;
import android.widget.ArrayAdapter;
import android.widget.Button;

public class TestContainer extends ListActivity implements TestApp.TestListener
{
    private LinkedList<String> _strings = new LinkedList<String>();
    private ArrayAdapter<String> _adapter;
    private boolean _partial = false;
    private Button _next;

    /** Called when the activity is first created. */
    @Override
    public void onCreate(Bundle savedInstanceState)
    {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.container);
        _adapter = new ArrayAdapter<String>(this, android.R.layout.simple_list_item_1, _strings);
        setListAdapter(_adapter);
        
        _next = (Button)findViewById(R.id.next);
        _next.setOnClickListener(new android.view.View.OnClickListener()
        {
            public void onClick(android.view.View v)
            {
                TestApp app = (TestApp)getApplication();
                app.startNextTest();
            }
        });
        _next.setEnabled(false);
    }

    @Override
    public void onResume()
    {
        super.onResume();
        TestApp app = (TestApp)getApplication();
        app.setTestListener(this);
    }

    @Override
    public void onStop()
    {
        super.onStop();
        TestApp app = (TestApp)getApplication();
        app.setTestListener(null);
    }

    public void onStartTest(String test)
    {
        setTitle(test);
        _partial = false;
        _strings.clear();
        _adapter.notifyDataSetChanged();
        _next.setEnabled(false);
    }
    
    public void onComplete(int status)
    {
        _next.setEnabled(true);
    }

    public void onOutput(String s)
    {
        int start = 0;
        int end = s.length();
        int pos;
        do
        {
            pos = s.indexOf('\n', start);
            if(pos != -1)
            {
                addData(s.substring(start, pos), false);
                start = pos+1;
            }
        }
        while(start < end && pos != -1);
        if(start < end)
        {
            addData(s.substring(start, end), true);
        }
    }
    
    private void addData(String s, boolean partialLine)
    {
        if(_partial)
        {
            String last = _strings.removeLast();
            last = last + s;
            _strings.add(last);
            _adapter.notifyDataSetChanged();
        }
        else
        {
            _adapter.add(s);
        }
        _partial = partialLine;
    }
}
