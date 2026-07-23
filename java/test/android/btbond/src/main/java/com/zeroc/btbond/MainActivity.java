// Copyright (c) ZeroC, Inc.

// Forces a BR/EDR bond between the two Android emulators (over Netsim/Rootcanal) before the IceBT
// tests run: IceBT uses secure RFCOMM, which only works between bonded devices, and bonding is
// otherwise an interactive pairing dialog with no adb equivalent. Installed as a privileged system
// app so it can hold BLUETOOTH_PRIVILEGED and auto-confirm the pairing request.
//
// Opening a secure RFCOMM socket is what triggers pairing, so the two modes below exchange a short
// PING/echo: that round-trip both drives the bond and proves the bonded channel carries data. The
// activity finishes once its mode completes, so it holds nothing open while the tests run and a
// later `am start` re-runs it. It drives android.bluetooth directly and has no Ice dependencies.
//
// Launch (via adb am start), reading the result from logcat tag BTBOND:
//   server: am start -n com.zeroc.btbond/.MainActivity --es mode server --es uuid <uuid>
//   client: am start -n com.zeroc.btbond/.MainActivity --es mode client --es peer <addr> --es uuid <uuid>

package com.zeroc.btbond;

import android.app.Activity;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothManager;
import android.bluetooth.BluetoothServerSocket;
import android.bluetooth.BluetoothSocket;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.Bundle;
import android.os.Handler;
import android.os.Looper;
import android.util.Log;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.nio.charset.StandardCharsets;
import java.util.Arrays;
import java.util.UUID;

public class MainActivity extends Activity {
    static final String TAG = "BTBOND";

    // Probe exchanged over the bonded channel to prove it carries data. Held as bytes so the buffer
    // sizes below are in the same units as what goes on the wire.
    static final byte[] PROBE = "PING".getBytes(StandardCharsets.UTF_8);

    // The whole run is bounded: BluetoothSocket.connect() and the stream reads are native blocking
    // calls that ignore interrupt(), so the only way to unwedge them is to close the socket. Without
    // this a stalled run never reaches finish(), and because `am start` on a resident
    // standard-launch-mode activity is a no-op bring-to-front, btbond would stay unusable for the
    // rest of the boot.
    static final long WATCHDOG_MS = 150_000;

    // Touched from the main thread (watchdog) and the worker, so all are volatile.
    private volatile Thread worker;
    private volatile BluetoothServerSocket serverSocket;
    private volatile BluetoothSocket socket;

    // Best-effort auto-accept of incoming pairing requests. The app runs as a privileged system app
    // (BLUETOOTH_PRIVILEGED), so setPairingConfirmation should succeed; setPin is a fallback that logs
    // any SecurityException.
    private final BroadcastReceiver pairingReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context ctx, Intent intent) {
            BluetoothDevice dev = intent.getParcelableExtra(BluetoothDevice.EXTRA_DEVICE, BluetoothDevice.class);
            int variant = intent.getIntExtra(BluetoothDevice.EXTRA_PAIRING_VARIANT, -1);
            if (dev == null) {
                Log.w(TAG, "PAIRING_REQUEST variant=" + variant + " with no device; ignoring");
                return;
            }
            Log.i(TAG, "PAIRING_REQUEST variant=" + variant + " from " + dev.getAddress());
            try {
                // setPairingConfirmation reports failure by returning false as well as by throwing;
                // both need to reach the setPin fallback below, which handles the variants it can't.
                if (dev.setPairingConfirmation(true)) {
                    Log.i(TAG, "setPairingConfirmation(true) accepted");
                    return;
                }
                Log.w(TAG, "setPairingConfirmation(true) returned false");
            } catch (Throwable t) {
                Log.w(TAG, "setPairingConfirmation failed: " + t);
            }
            try {
                dev.setPin("0000".getBytes(StandardCharsets.UTF_8));
                Log.i(TAG, "setPin(0000) accepted");
            } catch (Throwable t2) {
                Log.w(TAG, "setPin failed: " + t2);
            }
        }
    };

    @Override
    protected void onCreate(Bundle b) {
        super.onCreate(b);
        // ACTION_PAIRING_REQUEST is a system broadcast; register exported (and unregister in onDestroy).
        registerReceiver(
            pairingReceiver,
            new IntentFilter(BluetoothDevice.ACTION_PAIRING_REQUEST),
            Context.RECEIVER_EXPORTED);
        Intent it = getIntent();
        final String mode = it.getStringExtra("mode");
        final String peer = it.getStringExtra("peer");
        final String uuid = it.getStringExtra("uuid");
        Log.i(TAG, "start mode=" + mode + " peer=" + peer);
        worker = new Thread(() -> {
            try {
                run(mode, peer, uuid);
            } finally {
                // Finish so a subsequent `am start` re-runs onCreate with the new extras. A
                // standard-launch-mode activity left resident is simply brought to front, and the
                // caller would then match the previous run's result line out of logcat.
                // finish() is not documented as thread-safe, and the relaunch depends on it.
                runOnUiThread(MainActivity.this::finish);
            }
        });
        worker.start();
        new Handler(Looper.getMainLooper()).postDelayed(
            () -> {
                if (worker != null && worker.isAlive()) {
                    Log.w(TAG, "watchdog: run exceeded " + WATCHDOG_MS + "ms, closing sockets");
                    result(mode, false, "timed out after " + WATCHDOG_MS + "ms");
                    closeSockets();
                }
            },
            WATCHDOG_MS);
    }

    // Closing the sockets is what actually unblocks accept/connect/read; interrupt() does not.
    private void closeSockets() {
        BluetoothServerSocket ss = serverSocket;
        if (ss != null) {
            try {
                ss.close();
            } catch (Throwable ignored) {
                // Already closed.
            }
        }
        BluetoothSocket s = socket;
        if (s != null) {
            try {
                s.close();
            } catch (Throwable ignored) {
                // Already closed.
            }
        }
    }

    @Override
    protected void onDestroy() {
        try {
            unregisterReceiver(pairingReceiver);
        } catch (IllegalArgumentException ignored) {
            // Receiver was not registered; nothing to do.
        }
        // Deliberately does NOT close the sockets. Closing them here looks like the right cleanup --
        // it does leak a listener and its SDP record until accept() times out -- but the system
        // recreates this activity during a normal run, and closing the socket out from under a
        // blocked accept() makes it throw, turning a survivable recreate into a failed bond. That
        // regressed CI once; the leak is transient and self-heals, so leave it alone. The watchdog
        // bounds a genuinely stuck run instead.
        super.onDestroy();
    }

    // Everything here is deliberately fixed rather than configurable: only secure RFCOMM triggers
    // pairing, which is the whole point of this app, so there is no insecure mode to select.
    void run(String mode, String peer, String uuidStr) {
        try {
            BluetoothManager manager = getSystemService(BluetoothManager.class);
            BluetoothAdapter adapter = manager == null ? null : manager.getAdapter();
            if (adapter == null) {
                result(mode, false, "no adapter");
                return;
            }
            if (!adapter.isEnabled()) {
                result(mode, false, "adapter not enabled");
                return;
            }
            UUID uuid = UUID.fromString(uuidStr);

            if ("server".equals(mode)) {
                BluetoothServerSocket ss = serverSocket =
                    adapter.listenUsingRfcommWithServiceRecord("btbond", uuid);
                try {
                    Log.i(TAG, "listening, waiting up to 120s for client...");
                    BluetoothSocket s = socket = ss.accept(120000);
                    try {
                        Log.i(TAG, "accepted a connection");
                        InputStream in = s.getInputStream();
                        OutputStream out = s.getOutputStream();
                        byte[] buf = new byte[PROBE.length];
                        int n = readFully(in, buf);
                        Log.i(TAG, "server read " + n + " bytes");
                        // Only echo a complete probe: a short read is already a failure, and echoing
                        // it would just make the client report a mismatch instead of the real cause.
                        if (n == buf.length) {
                            out.write(buf);
                            out.flush();
                        }
                        result(mode, n == buf.length, "echoed " + n + " bytes");
                    } finally {
                        s.close();
                    }
                } finally {
                    ss.close();
                }
            } else if ("client".equals(mode)) {
                BluetoothDevice dev = adapter.getRemoteDevice(peer);
                if (dev.getBondState() != BluetoothDevice.BOND_BONDED) {
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
                BluetoothSocket s = socket = dev.createRfcommSocketToServiceRecord(uuid);
                try {
                    Log.i(TAG, "connecting...");
                    s.connect();
                    Log.i(TAG, "connected");
                    OutputStream out = s.getOutputStream();
                    InputStream in = s.getInputStream();
                    out.write(PROBE);
                    out.flush();
                    byte[] buf = new byte[PROBE.length];
                    int n = readFully(in, buf);
                    String echo = new String(buf, 0, n, StandardCharsets.UTF_8);
                    Log.i(TAG, "client got echo: '" + echo + "'");
                    // buf is exactly probe-sized, so a short read leaves trailing zeros and compares
                    // unequal; no separate length check is needed.
                    result(mode, Arrays.equals(buf, PROBE), "echo='" + echo + "'");
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

    // RFCOMM is a byte stream, so a single read may return fewer bytes than the peer sent. Read the
    // whole probe (or EOF) before echoing or comparing it, otherwise a split write reads as a
    // bonding failure.
    static int readFully(InputStream in, byte[] buf) throws IOException {
        int n = 0;
        while (n < buf.length) {
            int r = in.read(buf, n, buf.length - n);
            if (r < 0) {
                break;
            }
            n += r;
        }
        return n;
    }

    void result(String mode, boolean ok, String detail) {
        Log.i(TAG, "RESULT " + mode + " " + (ok ? "OK" : "FAIL") + " : " + detail);
    }
}
