// Copyright (c) ZeroC, Inc.

// This is a placeholder for the Android API. It is not included in the IceBT JAR file.

package android.bluetooth;

import java.io.IOException;
import java.util.UUID;

public final class BluetoothAdapter {
    public boolean cancelDiscovery() {
        return false;
    }

    public boolean isEnabled() {
        return false;
    }

    public String getAddress() {
        return "";
    }

    public static boolean checkBluetoothAddress(String address) {
        return false;
    }

    public static BluetoothAdapter getDefaultAdapter() {
        return null;
    }

    public BluetoothDevice getRemoteDevice(byte[] address) {
        return null;
    }

    public BluetoothDevice getRemoteDevice(String address) {
        return null;
    }

    public BluetoothServerSocket listenUsingInsecureRfcommWithServiceRecord(
            String name, UUID uuid) throws IOException {
        return null;
    }

    public BluetoothServerSocket listenUsingRfcommWithServiceRecord(
            String name, UUID uuid) throws IOException {
        return null;
    }
}
