// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package com.zeroc.testcontroller;

import java.util.LinkedList;
import android.app.*;
import android.content.Context;
import android.net.wifi.WifiManager;
import android.os.Bundle;
import android.widget.*;

public class ControllerActivity extends ListActivity
{
    private WifiManager _wifiManager;
    private WifiManager.MulticastLock _lock;
    private LinkedList<String> _strings = new LinkedList<String>();
    private ArrayAdapter<String> _adapter;

    @Override
    public void onCreate(Bundle savedInstanceState)
    {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.main);

        _wifiManager = (WifiManager)getApplicationContext().getSystemService(Context.WIFI_SERVICE);
        _lock = _wifiManager.createMulticastLock("com.zeroc.testcontroller");
        _lock.acquire();

        _adapter = new ArrayAdapter<String>(this, android.R.layout.simple_list_item_1, _strings);
        setListAdapter(_adapter);
        final ControllerApp app = (ControllerApp)getApplication();
        app.startController(this);
    }

    public synchronized void println(String data)
    {
        _strings.add(data);
        _adapter.notifyDataSetChanged();
    }
}
