// Copyright (c) ZeroC, Inc.

// This is a placeholder for the Android API. It is not included in the IceBT JAR file.

package android.bluetooth;

import java.io.IOException;
import java.util.UUID;

public final class BluetoothDevice {
    public BluetoothSocket createInsecureRfcommSocketToServiceRecord(UUID uuid)
            throws IOException {
        return null;
    }

    public BluetoothSocket createRfcommSocketToServiceRecord(UUID uuid)
            throws IOException {
        return null;
    }

    public String getAddress() {
        return "";
    }
}
