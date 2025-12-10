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
python3 ../scripts/Controller.py --host-bt="A4:FF:9F:A6:48:C1" --host=192.168.1.51 --id=server --android
```

Where:
- `--host-bt` is the Bluetooth address of the Android device running the server
- `--host` is the IP address of the Android device running the server
- `--id=server` identifies this controller as managing servers
- `--android` indicates this is controlling an Android device

Then from the `cpp` directory, start the C++ test suite to run against the Android servers:

```bash
python3 ./allTests.py --server=server --protocol=bt --cross=java
```

Where:
- `--server=server` indicates to use the remote server controller
- `--protocol=bt` specifies to use Bluetooth transport
- `--cross=java` indicates cross-language testing with Java servers

## Run Android Client Against C++ Server Over Bluetooth

First, start the Android test controller application on your device.

From the `cpp` directory, start the Python controller to manage the C++ server:

```bash
python3 ../scripts/Controller.py --host-bt="00:15:83:ED:D7:29" --host=192.168.1.48 --id=server --debug
```

Where:
- `--host-bt` is the Bluetooth address of the machine running the C++ server
- `--host` is the IP address of the machine running the C++ server
- `--id=server` identifies this controller as managing servers

Then from the `java` directory, start the Java/Android test suite to run against the C++ servers:

```bash
python3 ./allTests.py --server=server --protocol=bt --cross=cpp --android Ice/proxy
```

Where:
- `--server=server` indicates to use the remote server controller
- `--protocol=bt` specifies to use Bluetooth transport
- `--cross=cpp` indicates cross-language testing with C++ servers
- `--android` runs the Android client tests
- `Ice/proxy` specifies which test(s) to run (optional, omit to run all tests)

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
