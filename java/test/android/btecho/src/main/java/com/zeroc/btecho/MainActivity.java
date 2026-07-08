// Minimal Bluetooth RFCOMM echo helper used by the IceBT Bluetooth CI harness to force a BR/EDR
// bond between the two Android emulators (over Netsim/Rootcanal) before the IceBT tests run. It
// drives android.bluetooth directly and has no Ice dependencies.
//
// Launch (via adb am start), reading the result from logcat tag BTECHO:
//   server: am start -n com.zeroc.btecho/.MainActivity --es mode server --es uuid <uuid> --ez secure <bool>
//   client: am start -n com.zeroc.btecho/.MainActivity --es mode client --es peer <addr> --es uuid <uuid> --ez secure <bool> --ez bond <bool>
package com.zeroc.btecho;

import android.app.Activity;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothServerSocket;
import android.bluetooth.BluetoothSocket;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.Bundle;
import android.util.Log;

import java.io.InputStream;
import java.io.OutputStream;
import java.util.UUID;

public class MainActivity extends Activity {
    static final String TAG = "BTECHO";

    // Best-effort auto-accept of incoming pairing requests. The app runs as a privileged system app
    // (BLUETOOTH_PRIVILEGED), so setPairingConfirmation should succeed; setPin is a fallback that logs
    // any SecurityException.
    private final BroadcastReceiver pairingReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context ctx, Intent intent) {
            BluetoothDevice dev = intent.getParcelableExtra(BluetoothDevice.EXTRA_DEVICE);
            int variant = intent.getIntExtra(BluetoothDevice.EXTRA_PAIRING_VARIANT, -1);
            Log.i(TAG, "PAIRING_REQUEST variant=" + variant + " from " + (dev == null ? "?" : dev.getAddress()));
            try {
                dev.setPairingConfirmation(true);
                Log.i(TAG, "setPairingConfirmation(true) accepted");
            } catch (Throwable t) {
                Log.w(TAG, "setPairingConfirmation failed: " + t);
                try {
                    dev.setPin("0000".getBytes());
                    Log.i(TAG, "setPin(0000) accepted");
                } catch (Throwable t2) {
                    Log.w(TAG, "setPin failed: " + t2);
                }
            }
        }
    };

    @Override
    protected void onCreate(Bundle b) {
        super.onCreate(b);
        // ACTION_PAIRING_REQUEST is a system broadcast; register exported (and unregister in onDestroy).
        registerReceiver(pairingReceiver, new IntentFilter(BluetoothDevice.ACTION_PAIRING_REQUEST), Context.RECEIVER_EXPORTED);
        Intent it = getIntent();
        final String mode = it.getStringExtra("mode");
        final String peer = it.getStringExtra("peer");
        final String uuid = it.getStringExtra("uuid");
        final boolean secure = it.getBooleanExtra("secure", false);
        final boolean bond = it.getBooleanExtra("bond", false);
        Log.i(TAG, "start mode=" + mode + " peer=" + peer + " secure=" + secure + " bond=" + bond);
        new Thread(() -> run(mode, peer, uuid, secure, bond)).start();
    }

    @Override
    protected void onDestroy() {
        try {
            unregisterReceiver(pairingReceiver);
        } catch (IllegalArgumentException ignored) {
            // Receiver was not registered; nothing to do.
        }
        super.onDestroy();
    }

    void run(String mode, String peer, String uuidStr, boolean secure, boolean bond) {
        try {
            BluetoothAdapter adapter = BluetoothAdapter.getDefaultAdapter();
            if (adapter == null) { result(mode, false, "no adapter"); return; }
            if (!adapter.isEnabled()) { result(mode, false, "adapter not enabled"); return; }
            Log.i(TAG, "adapter enabled=true");
            UUID uuid = UUID.fromString(uuidStr);

            if ("server".equals(mode)) {
                BluetoothServerSocket ss = secure
                    ? adapter.listenUsingRfcommWithServiceRecord("btecho", uuid)
                    : adapter.listenUsingInsecureRfcommWithServiceRecord("btecho", uuid);
                try {
                    Log.i(TAG, "listening secure=" + secure + ", waiting up to 120s for client...");
                    BluetoothSocket s = ss.accept(120000);
                    try {
                        Log.i(TAG, "accepted a connection");
                        InputStream in = s.getInputStream();
                        OutputStream out = s.getOutputStream();
                        byte[] buf = new byte[64];
                        int n = in.read(buf);
                        Log.i(TAG, "server read " + n + " bytes");
                        if (n > 0) { out.write(buf, 0, n); out.flush(); }
                        result(mode, n > 0, "echoed " + n + " bytes");
                    } finally {
                        s.close();
                    }
                } finally {
                    ss.close();
                }
            } else if ("client".equals(mode)) {
                BluetoothDevice dev = adapter.getRemoteDevice(peer);
                if (bond && dev.getBondState() != BluetoothDevice.BOND_BONDED) {
                    Log.i(TAG, "bond state=" + dev.getBondState() + ", createBond()...");
                    dev.createBond();
                    long deadline = System.currentTimeMillis() + 60000;
                    while (dev.getBondState() != BluetoothDevice.BOND_BONDED
                            && System.currentTimeMillis() < deadline) {
                        Thread.sleep(1000);
                    }
                    Log.i(TAG, "final bond state=" + dev.getBondState());
                    if (dev.getBondState() != BluetoothDevice.BOND_BONDED) {
                        result(mode, false, "bond failed state=" + dev.getBondState());
                        return;
                    }
                }
                BluetoothSocket s = secure
                    ? dev.createRfcommSocketToServiceRecord(uuid)
                    : dev.createInsecureRfcommSocketToServiceRecord(uuid);
                try {
                    Log.i(TAG, "connecting secure=" + secure + "...");
                    s.connect();
                    Log.i(TAG, "connected");
                    OutputStream out = s.getOutputStream();
                    InputStream in = s.getInputStream();
                    out.write("PING".getBytes()); out.flush();
                    byte[] buf = new byte[64];
                    int n = in.read(buf);
                    String echo = n > 0 ? new String(buf, 0, n) : "";
                    Log.i(TAG, "client got echo: '" + echo + "'");
                    result(mode, "PING".equals(echo), "echo='" + echo + "'");
                } finally {
                    s.close();
                }
            } else {
                result(mode, false, "unknown mode '" + mode + "'");
            }
        } catch (Throwable t) {
            Log.e(TAG, "EXC " + t, t);
            result(mode, false, "exception: " + t);
        }
    }

    void result(String mode, boolean ok, String detail) {
        Log.i(TAG, "RESULT " + mode + " " + (ok ? "OK" : "FAIL") + " : " + detail);
    }
}
