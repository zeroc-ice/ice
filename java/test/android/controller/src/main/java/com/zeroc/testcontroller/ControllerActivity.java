// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

package com.zeroc.testcontroller;

import java.util.LinkedList;
import android.app.*;
import android.bluetooth.BluetoothAdapter;
import android.content.Context;
import android.content.Intent;
import android.net.wifi.WifiManager;
import android.os.Bundle;
import android.widget.*;
import android.view.View;

public class ControllerActivity extends ListActivity
{
    private WifiManager _wifiManager;
    private WifiManager.MulticastLock _lock;
    private LinkedList<String> _output = new LinkedList<String>();
    private ArrayAdapter<String> _outputAdapter;
    private ArrayAdapter<String> _ipv4Adapter;
    private ArrayAdapter<String> _ipv6Adapter;

    private static final int REQUEST_ENABLE_BT = 1;

    @Override
    public void onCreate(Bundle savedInstanceState)
    {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.main);

        _wifiManager = (WifiManager)getApplicationContext().getSystemService(Context.WIFI_SERVICE);
        _lock = _wifiManager.createMulticastLock("com.zeroc.testcontroller");
        _lock.acquire();
    }

    @Override
    public void onStart()
    {
        super.onStart();

        //
        // Enable Bluetooth if necessary.
        //
        BluetoothAdapter adapter = BluetoothAdapter.getDefaultAdapter();
        if(adapter == null)
        {
            Toast.makeText(this, R.string.no_bluetooth, Toast.LENGTH_SHORT).show();
            setup(false);
        }
        else if(!adapter.isEnabled())
        {
            Intent enableIntent = new Intent(BluetoothAdapter.ACTION_REQUEST_ENABLE);
            startActivityForResult(enableIntent, REQUEST_ENABLE_BT);
        }
        else
        {
            setup(true);
        }
    }

    @Override
    protected void onActivityResult(int req, int res, Intent data)
    {
        switch(req)
        {
            case REQUEST_ENABLE_BT:
            {
                if(_outputAdapter == null)
                {
                    if(res == Activity.RESULT_OK)
                    {
                        setup(true);
                    }
                    else
                    {
                        Toast.makeText(this, R.string.no_bluetooth, Toast.LENGTH_SHORT).show();
                        setup(false);
                    }
                }
                break;
            }
        }
    }

    private synchronized void setup(boolean bluetooth)
    {
        _outputAdapter = new ArrayAdapter<String>(this, android.R.layout.simple_list_item_1, _output);
        setListAdapter(_outputAdapter);
        final ControllerApp app = (ControllerApp)getApplication();
        final java.util.List<String> ipv4Addresses = app.getAddresses(false);
        _ipv4Adapter = new ArrayAdapter<>(this, android.R.layout.simple_spinner_dropdown_item, ipv4Addresses);
        Spinner s = (Spinner)findViewById(R.id.ipv4);
        s.setAdapter(_ipv4Adapter);
        s.setOnItemSelectedListener(new android.widget.AdapterView.OnItemSelectedListener()
            {
                @Override
                public void onItemSelected(AdapterView<?> parent, View view, int position, long id)
                {
                    app.setIpv4Address(ipv4Addresses.get((int)id));
                }

                @Override
                public void onNothingSelected(AdapterView<?> arg0)
                {
                }
            });
        s.setSelection(0);

        final java.util.List<String> ipv6Addresses = app.getAddresses(true);
        _ipv6Adapter = new ArrayAdapter<>(this, android.R.layout.simple_spinner_dropdown_item, ipv6Addresses);
        s = (Spinner)findViewById(R.id.ipv6);
        s.setAdapter(_ipv6Adapter);
        s.setOnItemSelectedListener(new android.widget.AdapterView.OnItemSelectedListener()
            {
                @Override
                public void onItemSelected(AdapterView<?> parent, View view, int position, long id)
                {
                    app.setIpv6Address(ipv6Addresses.get((int)id));
                }

                @Override
                public void onNothingSelected(AdapterView<?> arg0)
                {
                }
            });
        s.setSelection(0);
        app.startController(this, bluetooth);
    }

    public synchronized void println(String data)
    {
        _output.add(data);
        _outputAdapter.notifyDataSetChanged();
    }
}
