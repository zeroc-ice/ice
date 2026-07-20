# Bluetooth Testing Guide

This document describes how to run the Ice test suite over Bluetooth connections between different language mappings.

## Prerequisites

- C++ and Java source builds, refer to the [cpp/BUILDING.md] and [java/BUILDING.md] for details
- An Android device with Bluetooth capability
- The [Android test controller] application installed and running on the device
- Bluetooth adapters on both client and server machines
- Paired Bluetooth devices (the Android device and the machine running the C++ tests)

## Run C++ Client Against Android Server Over Bluetooth

First, start the Android test controller application on your device.

From the `java` directory, start the Python controller to manage the Android server:

```bash
python ../scripts/Controller.py --host-bt="A4:FF:9F:A6:48:C1" --host=192.168.1.51 --id=server --android
```

Where:
- `--host-bt` is the Bluetooth address of the Android device running the server
- `--host` is the IP address of the Android device running the server
- `--id=server` identifies this controller as managing servers
- `--android` indicates this is controlling an Android device

Then from the `cpp` directory, start the C++ test suite to run against the Android servers:

```bash
python ./allTests.py --server=server --protocol=bt --cross=java
```

Where:
- `--server=server` indicates to use the remote server controller
- `--protocol=bt` specifies to use Bluetooth transport, use `--protocol=bts` to test Bluetooth over SSL
- `--cross=java` indicates cross-language testing with Java servers

## Run Android Client Against C++ Server Over Bluetooth

First, start the Android test controller application on your device.

From the `cpp` directory, start the Python controller to manage the C++ server:

```bash
python ../scripts/Controller.py --host-bt="00:15:83:ED:D7:29" --host=192.168.1.48 --id=server
```

Where:
- `--host-bt` is the Bluetooth address of the machine running the C++ server
- `--host` is the IP address of the machine running the C++ server
- `--id=server` identifies this controller as managing servers

Then from the `java` directory, start the Java/Android test suite to run against the C++ servers:

```bash
python ./allTests.py --server=server --protocol=bt --cross=cpp --android
```

Where:
- `--server=server` indicates to use the remote server controller
- `--protocol=bt` specifies to use Bluetooth transport, use `--protocol=bts` to test Bluetooth over SSL
- `--cross=cpp` indicates cross-language testing with C++ servers
- `--android` runs the Android client tests

## Run Android Client Against Android Server Over Bluetooth (two emulators)

Both sides can run on Android emulators using the emulator's virtual Bluetooth controller
(Netsim/Rootcanal) — no radios required. One emulator runs the IceBT server, the other the client,
each driven by a controller bound to it via `--device`. The steps below reproduce, locally, what the
`bt-android-harness` CI workflow does; all the adb work is inside the harness (`Controller.py
--bt-setup` / `--bt-bond` / `--bt-diagnostics`), so no manual bonding is needed.

Run everything from the repository root with:

```bash
export PYTHONPATH="$PWD/python/python"
UUID=8ce255c0-200a-11e0-ac64-0800200c9a66
IMG="system-images;android-36;google_apis;x86_64"  # use arm64-v8a on Apple silicon
CLIENT=emulator-5554
SERVER=emulator-5556
```

**1. Build the `btecho` pre-bond helper.** IceBT uses secure RFCOMM, so the emulators must be bonded;
`btecho` is a tiny privileged app that auto-confirms pairing.

```bash
keytool -genkeypair -v -keystore java/test/android/btecho/debug.keystore -storepass android \
  -keypass android -alias androiddebugkey -keyalg RSA -keysize 2048 -validity 10000 \
  -dname "CN=Android Debug,O=Android,C=US"
(cd java/test/android/btecho && ./gradlew assembleDebug)
APK=$(find java/test/android/btecho/build/outputs/apk -name '*.apk')
```

**2. Create and boot the two emulators** (detached, on the shared Netsim Bluetooth network;
`-writable-system` is set for you so `btecho` can be installed as a privileged system app):

```bash
python scripts/Controller.py --android --bt-emulators \
  --bt-client="$CLIENT" --bt-server="$SERVER" --bt-image="$IMG"
```

**3. Prepare and bond them.** This waits for boot, installs `btecho` as a privileged system app,
enables Bluetooth, bonds the pair, and prints the server's Bluetooth address (per-device progress
goes to stderr and to `setup_client.log` / `setup_server.log`):

```bash
BT_ADDR=$(python scripts/Controller.py --android --bt-prepare \
  --bt-client="$CLIENT" --bt-server="$SERVER" --bt-setup="$APK" --uuid="$UUID")
```

**4. Run the tests:**

```bash
cd java
python ../scripts/Controller.py --id=server --android --controller-app \
  --device="$SERVER" --host-bt="$BT_ADDR" &
python allTests.py --server=server --protocol=bt --cross=java --android --controller-app \
  --device="$CLIENT" --host-bt="$BT_ADDR" Ice/operations
```

`Ice/operations` is just one suite; pass as many as you like. CI runs the full set listed in the
`TEST` variable of `.github/workflows/bt-android-harness.yml`.

If a run fails, dump an emulator's controller state (pid, adb forwards, logcat) with:

```bash
python scripts/Controller.py --android --device="$CLIENT" --bt-diagnostics
```

Individual emulators can also be prepared or bonded on their own with `--device=<serial> --bt-setup=<apk>`
and `--device=<serial> --bt-bond=<peer> --uuid=<uuid>`.

## Finding Bluetooth Addresses

On Linux, you can find the Bluetooth address of your machine using:

```bash
hciconfig
```

On Android, go to Settings → About phone → Status (or Settings → System → About phone) to find the Bluetooth address.

## Troubleshooting

- Ensure Bluetooth is enabled on all devices
- Verify devices are paired before running tests
- Check that the Android test controller application is running and visible
- Ensure IP addresses and Bluetooth addresses are correct
- Use `--debug` flag for verbose output to diagnose connection issues

[Android test controller]: java/test//android/controller/
[cpp/BUILDING.md]: cpp/BUILDING.md
[java/BUILDING.md]: java/BUILDING.md
