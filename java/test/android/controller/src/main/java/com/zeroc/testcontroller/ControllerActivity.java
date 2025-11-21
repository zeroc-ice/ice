// Copyright (c) ZeroC, Inc.

package com.zeroc.testcontroller;

import android.app.Activity;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothManager;
import android.content.Context;
import android.content.Intent;
import android.net.wifi.WifiManager;
import android.os.Bundle;
import android.view.View;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.ListView;
import android.widget.Spinner;
import android.widget.Toast;

import java.util.LinkedList;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

public class ControllerActivity extends Activity {
    private final LinkedList<String> _output = new LinkedList<>();
    private ArrayAdapter<String> _outputAdapter;
    private ListView _outputListView;
    private WifiManager.MulticastLock _multicastLock;

    private static final int REQUEST_ENABLE_BT = 1;

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.main);

        _outputListView = findViewById(R.id.outputList);
        if (_outputListView == null) {
            throw new IllegalStateException("Layout must include a View with android:id=\"@+id/outputList\"");
        }

        WifiManager wifiManager = (WifiManager) getApplicationContext().getSystemService(Context.WIFI_SERVICE);
        _multicastLock = wifiManager.createMulticastLock("com.zeroc.testcontroller");
        _multicastLock.acquire();
    }

    @Override
    public void onStart() {
        super.onStart();

        // Enable Bluetooth if necessary.
        BluetoothManager bluetoothManager = (BluetoothManager) getSystemService(Context.BLUETOOTH_SERVICE);
        BluetoothAdapter adapter = bluetoothManager.getAdapter();
        if (adapter == null) {
            Toast.makeText(this, R.string.no_bluetooth, Toast.LENGTH_SHORT).show();
            setup(false);
        } else if (!adapter.isEnabled()) {
            try {
                Intent enableIntent = new Intent(BluetoothAdapter.ACTION_REQUEST_ENABLE);
                startActivityForResult(enableIntent, REQUEST_ENABLE_BT);
            } catch (java.lang.SecurityException ex) {
                // The user didn't grant the required permissions.
                Toast.makeText(this, ex.toString(), Toast.LENGTH_SHORT).show();
            }
        } else {
            setup(true);
        }
    }

    @Override
    protected void onPause() {
        super.onPause();
        if (_multicastLock != null && _multicastLock.isHeld()) {
            _multicastLock.release();
        }
    }

    @Override
    protected void onResume() {
        super.onResume();
        if (_multicastLock != null && !_multicastLock.isHeld()) {
            _multicastLock.acquire();
        }
    }

    @Override
    protected void onActivityResult(int req, int res, Intent data) {
        if (req == REQUEST_ENABLE_BT && _outputAdapter == null) {
            if (res == Activity.RESULT_OK) {
                setup(true);
            } else {
                Toast.makeText(this, R.string.no_bluetooth, Toast.LENGTH_SHORT).show();
                setup(false);
            }
        }
    }

    private synchronized void setup(boolean bluetooth) {
        var spinnerDropdownItem = android.R.layout.simple_spinner_dropdown_item;

        _outputAdapter = new ArrayAdapter<>(this, android.R.layout.simple_list_item_1, _output);
        _outputListView.setAdapter(_outputAdapter);
        final ControllerApp app = (ControllerApp) getApplication();
        final java.util.List<String> ipv4Addresses = app.getAddresses(false);
        ArrayAdapter<String> ipv4Adapter = new ArrayAdapter<>(this, spinnerDropdownItem, ipv4Addresses);
        Spinner s = findViewById(R.id.ipv4);
        s.setAdapter(ipv4Adapter);
        s.setOnItemSelectedListener(new android.widget.AdapterView.OnItemSelectedListener() {
            @Override
            public void onItemSelected(AdapterView<?> parent, View view, int position, long id) {
                app.setIpv4Address(ipv4Addresses.get((int) id));
            }

            @Override
            public void onNothingSelected(AdapterView<?> arg0) {}
        });
        s.setSelection(0);

        final java.util.List<String> ipv6Addresses = app.getAddresses(true);
        ArrayAdapter<String> ipv6Adapter = new ArrayAdapter<>(this, spinnerDropdownItem, ipv6Addresses);
        s = findViewById(R.id.ipv6);
        s.setAdapter(ipv6Adapter);
        s.setOnItemSelectedListener(new android.widget.AdapterView.OnItemSelectedListener() {
            @Override
            public void onItemSelected(AdapterView<?> parent, View view, int position, long id) {
                app.setIpv6Address(ipv6Addresses.get((int) id));
            }

            @Override
            public void onNothingSelected(AdapterView<?> arg0) {}
        });
        s.setSelection(0);

        // Start the controller in a background thread. Starting the controller creates the ObjectAdapter which makes
        // IO calls. Android doesn't allow making IO calls from the main thread.
        ExecutorService executor = Executors.newSingleThreadExecutor();
        executor.submit(() -> {
            try {
                app.startController(this, bluetooth);
            } catch (Exception e) {
                e.printStackTrace();
            }
        });
        executor.shutdown();
    }

    public synchronized void println(String data) {
        _output.add(data);
        _outputAdapter.notifyDataSetChanged();
    }
}
