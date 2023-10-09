# Ice for Java Compat Build Instructions

This page describes how to build and install Ice for Java Compat from source. If you
prefer, you can also download a [binary distribution].

* [Build Requirements](#build-requirements)
  * [Operating Systems](#operating-systems)
  * [Slice to Java Compiler](#slice-to-java-compiler)
  * [JDK Version](#jdk-version)
  * [Gradle](#gradle)
  * [Bzip2 Compression](#bzip2-compression)
* [Building Ice for Java Compat](#building-ice-for-java-compat)
* [Installing Ice for Java Compat](#installing-ice-for-java-compat)
* [Running the Java Compat Tests](#running-the-java-compat-tests)
* [Building the Ice for Android Tests](#building-the-ice-for-android-tests)

## Build Requirements

### Operating Systems

Ice for Java Compat builds and runs properly on Windows, macOS, and any recent
Linux distributions. It is fully supported on the platforms listed on the
[supported platforms] page.

### Slice to Java Compiler

You need the Slice to Java compiler to build Ice for Java Compat and also to use
Ice for Java Compat. The Slice to Java compiler (`slice2java`) is a command-line tool
written in C++. You can build the Slice to Java compiler from source, or alternatively
you can install an Ice [binary distribution] that includes this compiler.

### JDK Version

You need JDK 8, JDK 11, or JDK 17 to build Ice for Java Compat.

Make sure that the `javac` and `java` commands are present in your PATH.

> The build produces bytecode in the Java 7 class file format ([major version] 51).

### Gradle

Ice for Java Compat uses the [Gradle] build system, and includes the Gradle wrapper
in the distribution. You cannot build the Ice for Java Compat source distribution
without an Internet connection. Gradle will download all required packages
automatically from the Maven Central repository located at
https://repo1.maven.org/maven2/

### Bzip2 Compression

Ice for Java Compat supports protocol compression using the bzip2 classes included
with [Apache Commons Compress].

The Maven package ID for the commons-compress JAR file is as follows:

```gradle
groupId=org.apache.commons, version=1.20, artifactId=commons-compress
```

The demos and tests are automatically setup to enable protocol compression by
adding the commons-compress JAR to the manifest class path. For your own
applications you must add the commons-compress JAR to the application `CLASSPATH`
to enable protocol compression.

## Building Ice for Java Compat

The build system requires the Slice to Java compiler from Ice for C++. If you
have not built Ice for C++ in this source distribution, you must set the
`ICE_HOME` environment variable with the path name of your Ice installation. For
example, on Linux:

```shell
export ICE_HOME=/opt/Ice-3.7.10 (For local build)
export ICE_HOME=/usr (For RPM installation)
```

On Windows:

```shell
set ICE_HOME=C:\Program Files\ZeroC\Ice-3.7.10 (MSI installation)
```

On Windows if you are using Ice for C++ from a source distribution, you must set
the `CPP_PLATFORM` and `CPP_CONFIGURATION` environment variables to match the
platform and configuration used in your C++ build:

```shell
set CPP_PLATFORM=x64
set CPP_CONFIGURATION=Debug
```

The supported values for `CPP_PLATFORM` are `Win32` and `x64` and the supported
values for `CPP_CONFIGURATION` are `Debug` and `Release`.

Before building Ice for Java Compat, review the settings in the file `gradle.properties`
and edit as necessary.

To build Ice, all services, and tests, run

```shell
gradlew build
```

Upon completion, the Ice JAR and POM files are placed in the `lib` subdirectory.

If at any time you wish to discard the current build and start a new one, use
these commands:

```shell
gradlew clean
gradlew build
```

## Installing Ice for Java Compat

To install Ice for Java Compat in the directory specified by the `prefix` variable in
`gradle.properties` run the following command:

```shell
gradlew install
```

The installation installs the following JAR files to `<prefix>/lib`.

* glacier2-compat-3.7.10.jar
* ice-compat-3.7.10.jar
* icebox-compat-3.7.10.jar
* icebt-compat-3.7.10.jar
* icediscovery-compat-3.7.10.jar
* icegrid-compat-3.7.10.jar
* icelocatordiscovery-compat-3.7.10.jar
* icepatch2-compat-3.7.10.jar
* icestorm-compat-3.7.10.jar

POM files are also installed for ease of deployment to a Maven-based
distribution system.

## Running the Java Compat Tests

Some of the Ice for Java Compat tests employ applications that are part of the Ice for
C++ distribution. If you have not built Ice for C++ in this source distribution
then you must set the `ICE_HOME` environment variable with the path name of your
Ice installation. On Linux or macOS:

```shell
export ICE_HOME=/opt/Ice-3.7.10 (For local build)
export ICE_HOME=/usr (For RPM installation)
```

On Windows:

```shell
set ICE_HOME=C:\Program Files\ZeroC\Ice-3.7.10
```

Python is required to run the test suite. To run the tests, open a command
window and change to the top-level directory. At the command prompt, execute:

```shell
python allTests.py
```

If everything worked out, you should see lots of `ok` messages. In case of a
failure, the tests abort with `failed`.

## Building the Ice for Android Tests

The `test/android/controller` directory contains an Android Studio project
for the Ice test suite controller.

### Android Build Requirements

Building any Ice application for Android requires Android Studio and the Android
SDK build tools. We tested with the following components:

* Android Studio Giraffe
* Android SDK 33

Ice requires at minimum API level 21:

* Android 5 (API21)

### Building the Android Test Controller

You must first build Ice for Java Compat refer to [Building Ice for Java Compat](#building-ice-for-java-compat)
for instructions, then follow these steps:

1. Start Android Studio
2. Select "Open an existing Android Studio project"
3. Navigate to and select the "android" subdirectory
4. Click OK and wait for the project to open and build

To build the tests against the Ice binary distribution you must set `ICE_BIN_DIST` environment
variable to `all` before starting Android Studio.

## Running the Android Tests

The Android Studio project contains a `controller` app for the Ice test
suite. Prior to running the app, you must disable Android Studio's Instant Run
feature, located in File / Settings / Build, Execution, Deployment /
Instant Run.

Tests are started from the dev machine using the `allTests.py` script, similar
to the other language mappings. The script uses Ice for Python to communicate
with the Android app, therefore you must build the [Python mapping](../../python)
before continuing.

You also need to add the `tools\bin`, `platform-tools` and `emulator`
directories from the Android SDK to your PATH. On macOS, you can use the
following commands:

```shell
export PATH=~/Library/Android/sdk/cmdline-tools/latest/bin:$PATH
export PATH=~/Library/Android/sdk/platform-tools:$PATH
export PATH=~/Library/Android/sdk/emulator:$PATH
```

On Windows, you can use the following commands:

```shell
set PATH=%LOCALAPPDATA%\Android\sdk\cmdline-tools\latest\bin;%PATH%
set PATH=%LOCALAPPDATA%\Android\sdk\platform-tools;%PATH%
set PATH=%LOCALAPPDATA%\Android\sdk\emulator;%PATH%
```

Run the tests with the Android emulator by running the following command:

```shell
python allTests.py --android --controller-app
```

To run the tests on a Android device connected through USB, you can use
the `--device=usb` option as shown below:

```shell
python allTests.py --android --device=usb --controller-app
```

To connect to an Android device that is running adb you can use the
`--device=<ip-address>`

```shell
python allTests.py --android --device=<ip-address> --controller-app
```

To run the tests against a `controller` application started from Android
Studio you should omit the `--controller-app` option from the commands above.

[binary distribution]: https://zeroc.com/downloads/ice
[supported platforms]: https://doc.zeroc.com/ice/3.7/release-notes/supported-platforms-for-ice-3-7-10
[Gradle]: https://gradle.org
[Apache Commons Compress]: https://commons.apache.org/proper/commons-compress/
[major version]: https://docs.oracle.com/javase/specs/jvms/se21/html/jvms-4.html#jvms-4.1-200-B.2
