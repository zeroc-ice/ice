# Bluetooth Testing Guide

Runs the Ice test suite over Bluetooth (IceBT). Four setups:

- C++ client against an Android server — needs hardware
- Android client against a C++ server — needs hardware
- Android against Android on two emulators — no hardware; CI's `android-bt`
- C++ against C++ on two virtual controllers — no hardware, Linux only; CI's `cpp-bt`

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
rm -f java/test/android/btbond/debug.keystore   # keytool fails if the alias already exists
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
`.github/workflows/ci.yml`; its setup is in `.github/actions/setup-android-bt`.

Dump an emulator's controller state (pid, adb forwards, logcat):

```bash
python scripts/Controller.py --android --device="$CLIENT" --bt-diagnostics
```

Emulators can also be prepared or bonded one at a time, with `--device=<serial> --bt-setup=<apk>`
and `--device=<serial> --bt-bond=<peer> --uuid=<uuid>`.

## Two virtual controllers, no hardware (Linux)

`btvirt` creates a pair of virtual Bluetooth controllers in the kernel, so a C++ client and a C++
server can talk RFCOMM on one machine. This is what CI's `cpp-bt` configuration does; its setup is
in `.github/actions/setup-vhci`. Linux only, and needs root.

The awkward part is BlueZ, not the controllers. Ice registers the same RFCOMM UUID on the client
and the server side, and BlueZ allows one registration per UUID **per daemon** (`src/profile.c`,
"UUID already registered"). So the two sides must talk to different daemons: the stock system one
serves the server side, and a second `bluetoothd` on a private D-Bus bus serves the client.

```bash
sudo apt-get install -y "linux-modules-extra-$(uname -r)" bluez bluez-test-tools bluez-tools
for m in bluetooth rfcomm bnep hci_vhci; do sudo modprobe "$m"; done   # l2cap is built into bluetooth.ko
```

**1. Create the controllers.** `-B` (BR/EDR only) is load-bearing: with LE enabled the first daemon
holds the GATT socket and the second cannot register an adapter at all — it fails with
`l2cap_bind: Address already in use`.

```bash
sudo btvirt -B -l2 &
sudo hciconfig hci0 up && sudo hciconfig hci1 up
BT_ADDR=$(hciconfig hci1 | sed -n 's/.*BD Address: \([0-9A-F:]*\).*/\1/p')          # server
BT_CLIENT_ADDR=$(hciconfig hci0 | sed -n 's/.*BD Address: \([0-9A-F:]*\).*/\1/p')   # client
```

**2. Start the second daemon** on its own bus:

```bash
cat > /tmp/bus2.conf <<'EOF'
<!DOCTYPE busconfig PUBLIC "-//freedesktop//DTD D-Bus Bus Configuration 1.0//EN"
 "http://www.freedesktop.org/standards/dbus/1.0/busconfig.dtd">
<busconfig>
  <type>system</type>
  <listen>unix:path=/tmp/bus2.sock</listen>
  <policy context="default">
    <allow user="*"/>
    <allow own="*"/>
    <allow send_type="method_call"/>
    <allow send_type="signal"/>
    <allow send_type="method_return"/>
    <allow send_type="error"/>
    <allow receive_type="*"/>
  </policy>
</busconfig>
EOF
sudo dbus-daemon --config-file=/tmp/bus2.conf --fork
sudo DBUS_SYSTEM_BUS_ADDRESS=unix:path=/tmp/bus2.sock /usr/libexec/bluetooth/bluetoothd -n &
```

**3. Bond the pair.** Run an agent per daemon, as a *daemon* — an agent registered inside a
`bluetoothctl` session dies with it, and BlueZ then answers the pending confirmation negatively,
which surfaces as `org.bluez.Error.AuthenticationFailed`.

```bash
sudo bt-agent --capability=NoInputNoOutput &
sudo DBUS_SYSTEM_BUS_ADDRESS=unix:path=/tmp/bus2.sock bt-agent --capability=NoInputNoOutput &
# The server has to be discoverable and pairable, or the pair below fails with "Device not available".
printf 'select %s\npower on\ndiscoverable on\npairable on\nquit\n' "$BT_ADDR" | sudo bluetoothctl
printf 'select %s\npower on\nquit\n' "$BT_CLIENT_ADDR" | sudo bluetoothctl
sudo bluetoothctl --timeout 20 scan on < /dev/null
sudo bluetoothctl pair "$BT_ADDR"
```

Check the stored link key rather than `bluetoothctl info` — the device object can be pruned while
the key on disk stays valid, and `bluetoothctl` segfaults intermittently mid-pair, so its exit
status says little:

```bash
sudo sed -n '/^\[LinkKey\]/,/^\[/p' "/var/lib/bluetooth/$BT_CLIENT_ADDR/$BT_ADDR/info" | grep "^Key="
```

**4. Run the tests.** The server controller stays on the system bus; the client uses the second
daemon. `--cross=cpp` skips the collocated case, which has no server side and so leaves
`Ice.Default.Host` as an IP address IceBT rejects.

```bash
cd cpp
python3 ../scripts/Controller.py --id=server --host-bt="$BT_ADDR" &
DBUS_SYSTEM_BUS_ADDRESS=unix:path=/tmp/bus2.sock \
  python3 allTests.py --server=server --protocol=bt --cross=cpp --host-bt="$BT_ADDR" Ice/operations
```

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
