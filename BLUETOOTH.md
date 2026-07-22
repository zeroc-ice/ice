# Bluetooth Testing Guide

Runs the Ice test suite over Bluetooth (IceBT). Three setups:

- C++ client against an Android server — needs hardware
- Android client against a C++ server — needs hardware
- Android against Android on two emulators — no hardware; this is what CI runs

## Common flags

- `--host-bt` — Bluetooth address of the machine or device running the server
- `--host` — its IP address
- `--id=server` — this controller manages servers
- `--android` — the controller drives an Android device
- `--server=server` — use the remote server controller
- `--protocol=bt` — Bluetooth transport; `bts` for Bluetooth over SSL
- `--cross=<mapping>` — language mapping on the other side

## Hardware setups

Prerequisites:

- C++ and Java source builds — see [cpp/BUILDING.md] and [java/BUILDING.md]
- An Android device with Bluetooth, already paired with the host
- The [Android test controller] app installed and running on it

**C++ client, Android server.** From `java`, start the controller for the Android server:

```bash
python ../scripts/Controller.py --host-bt="A4:FF:9F:A6:48:C1" --host=192.168.1.51 --id=server --android
```

Then run the client from `cpp`:

```bash
python ./allTests.py --server=server --protocol=bt --cross=java
```

**Android client, C++ server.** From `cpp`, start the controller for the C++ server:

```bash
python ../scripts/Controller.py --host-bt="00:15:83:ED:D7:29" --host=192.168.1.48 --id=server
```

Then run the client from `java`:

```bash
python ./allTests.py --server=server --protocol=bt --cross=cpp --android
```

## Two emulators, no hardware

The emulator ships a virtual Bluetooth controller (Netsim/Rootcanal) that does RFCOMM between
emulators. One runs the server, the other the client. This is what CI's `android-bt` configuration
does. All the adb work lives in the harness, so nothing is bonded by hand.

Run from the repository root, with:

```bash
export PYTHONPATH="$PWD/python/python"
UUID=8ce255c0-200a-11e0-ac64-0800200c9a66
IMG="system-images;android-36;google_apis;x86_64"  # arm64-v8a on Apple silicon
CLIENT=emulator-5554
SERVER=emulator-5556
```

**1. Build `btbond`.** IceBT needs bonded devices. `btbond` is a privileged helper that
auto-confirms pairing.

```bash
keytool -genkeypair -v -keystore java/test/android/btbond/debug.keystore -storepass android \
  -keypass android -alias androiddebugkey -keyalg RSA -keysize 2048 -validity 10000 \
  -dname "CN=Android Debug,O=Android,C=US"
(cd java/test/android/btbond && ../../../gradlew assembleDebug)
APK=$(find java/test/android/btbond/build/outputs/apk -name '*.apk')
```

**2. Create and boot the emulators.** Detached, on a shared Netsim network. `-writable-system` is
set for you, so `btbond` can be installed as a privileged system app.

```bash
python scripts/Controller.py --android --bt-emulators \
  --bt-client="$CLIENT" --bt-server="$SERVER" --bt-image="$IMG"
```

**3. Prepare and bond them.** Waits for boot, installs `btbond`, enables Bluetooth, bonds the pair,
prints the server's address. Progress goes to stderr and to `setup_client.log` / `setup_server.log`.

```bash
BT_ADDR=$(python scripts/Controller.py --android --bt-prepare \
  --bt-client="$CLIENT" --bt-server="$SERVER" --bt-setup="$APK" --uuid="$UUID")
```

**4. Run the tests.**

```bash
cd java
python ../scripts/Controller.py --id=server --android --controller-app \
  --device="$SERVER" --host-bt="$BT_ADDR" &
python allTests.py --server=server --protocol=bt --cross=java --android --controller-app \
  --device="$CLIENT" --host-bt="$BT_ADDR" Ice/operations
```

Pass as many suites as you like. CI's list is in the `android-bt` entry of
`.github/workflows/ci.yml`; its setup is in `.github/actions/setup-bt`.

Dump an emulator's controller state (pid, adb forwards, logcat):

```bash
python scripts/Controller.py --android --device="$CLIENT" --bt-diagnostics
```

Emulators can also be prepared or bonded one at a time, with `--device=<serial> --bt-setup=<apk>`
and `--device=<serial> --bt-bond=<peer> --uuid=<uuid>`.

## Finding Bluetooth addresses

On Linux, run `hciconfig`. On Android, see Settings → About phone → Status.

## Troubleshooting

- Is Bluetooth enabled on all devices?
- Are the devices paired?
- Is the Android test controller running and visible?
- Are the IP and Bluetooth addresses correct?
- Add `--debug` for verbose output.

[Android test controller]: java/test/android/controller/
[cpp/BUILDING.md]: cpp/BUILDING.md
[java/BUILDING.md]: java/BUILDING.md
