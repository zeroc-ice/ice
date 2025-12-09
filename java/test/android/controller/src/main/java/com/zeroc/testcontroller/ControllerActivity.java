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

import android.Manifest;
import android.content.pm.PackageManager;

import androidx.core.app.ActivityCompat;
import androidx.core.content.ContextCompat;

import java.util.LinkedList;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

public class ControllerActivity extends Activity {
    private static final int REQ_CODE_BT_PERMISSIONS = 1001;
    private static final int REQ_CODE_BT_ENABLE = 1002;

    private final LinkedList<String> _output = new LinkedList<>();
    private ArrayAdapter<String> _outputAdapter;
    private ListView _outputListView;
    private WifiManager.MulticastLock _multicastLock;
    private boolean _isSetupComplete = false;

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.main);

        _outputListView = findViewById(R.id.outputList);
        if (_outputListView == null) {
            throw new IllegalStateException("Layout must include a View with android:id=\"@+id/outputList\"");
        }

        _outputAdapter = new ArrayAdapter<>(this, android.R.layout.simple_list_item_1, _output);
        _outputListView.setAdapter(_outputAdapter);

        WifiManager wifiManager = (WifiManager) getApplicationContext().getSystemService(Context.WIFI_SERVICE);
        _multicastLock = wifiManager.createMulticastLock("com.zeroc.testcontroller");
        _multicastLock.acquire();
    }

    @Override
    public void onStart() {
        super.onStart();
        
        if (!_isSetupComplete) {
            initializeBluetooth();
        }
    }

    @Override
    public void onRequestPermissionsResult(int requestCode, String[] permissions, int[] grantResults) {
        super.onRequestPermissionsResult(requestCode, permissions, grantResults);

        if (requestCode == REQ_CODE_BT_PERMISSIONS) {

            if (grantResults.length == 0) {
                showBluetoothError(R.string.no_bluetooth);
                completeSetup(false);
                return;
            }

            boolean allGranted = true;
            for (int result : grantResults) {
                if (result != PackageManager.PERMISSION_GRANTED) {
                    allGranted = false;
                    break;
                }
            }
            
            if (allGranted) {
                enableBluetoothIfNeeded();
            } else {
                showBluetoothError(R.string.bluetooth_permission_denied);
                completeSetup(false);
            }
        }
    }

    public synchronized void println(String data) {
        _output.add(data);
        if (_outputAdapter != null) {
            _outputAdapter.notifyDataSetChanged();
        }
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        super.onActivityResult(requestCode, resultCode, data);
        
        if (requestCode == REQ_CODE_BT_ENABLE) {
            if (resultCode == Activity.RESULT_OK) {
                completeSetup(true);
            } else {
                showBluetoothError(R.string.no_bluetooth);
                completeSetup(false);
            }
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

    private synchronized void completeSetup(boolean bluetooth) {
        if (_isSetupComplete) {
            return;
        }
        
        _isSetupComplete = true;
        var spinnerDropdownItem = android.R.layout.simple_spinner_dropdown_item;

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

    private void enableBluetoothIfNeeded() {
        BluetoothManager bluetoothManager = (BluetoothManager) getSystemService(Context.BLUETOOTH_SERVICE);
        BluetoothAdapter adapter = bluetoothManager.getAdapter();
        
        if (adapter == null) {
            showBluetoothError(R.string.no_bluetooth);
            completeSetup(false);
        } else if (adapter.isEnabled()) {
            completeSetup(true);
        } else {
            try {
                Intent enableIntent = new Intent(BluetoothAdapter.ACTION_REQUEST_ENABLE);
                startActivityForResult(enableIntent, REQ_CODE_BT_ENABLE);
            } catch (SecurityException ex) {
                Toast.makeText(this, ex.toString(), Toast.LENGTH_LONG).show();
                completeSetup(false);
            }
        }
    }

    private void initializeBluetooth() {
        BluetoothManager bluetoothManager = (BluetoothManager) getSystemService(Context.BLUETOOTH_SERVICE);
        BluetoothAdapter adapter = bluetoothManager.getAdapter();
        
        if (adapter == null) {
            showBluetoothError(R.string.no_bluetooth);
            completeSetup(false);
            return;
        }

        requestBluetoothPermissionsIfNeeded();
    }

    private void requestBluetoothPermissionsIfNeeded() {
        java.util.List<String> neededPermissions = new java.util.ArrayList<>();

        if (ContextCompat.checkSelfPermission(this, Manifest.permission.BLUETOOTH_SCAN) 
                != PackageManager.PERMISSION_GRANTED) {
            neededPermissions.add(Manifest.permission.BLUETOOTH_SCAN);
        }

        if (ContextCompat.checkSelfPermission(this, Manifest.permission.BLUETOOTH_CONNECT) 
                != PackageManager.PERMISSION_GRANTED) {
            neededPermissions.add(Manifest.permission.BLUETOOTH_CONNECT);
        }

        if (!neededPermissions.isEmpty()) {
            ActivityCompat.requestPermissions(
                    this,
                    neededPermissions.toArray(new String[0]),
                    REQ_CODE_BT_PERMISSIONS
            );
        } else {
            enableBluetoothIfNeeded();
        }
    }

    private void showBluetoothError(int messageResId) {
        Toast.makeText(this, messageResId, Toast.LENGTH_LONG).show();
    }
}
