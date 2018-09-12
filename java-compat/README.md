# Building Ice for Java Compat

This page describes how to build and install Ice for Java from source. If
you prefer, you can also download a [binary distribution][1].

* [Build Requirements](#build-requirements)
  * [Operating Systems](#operating-systems)
  * [Slice to Java Compiler](#slice-to-java-compiler)
  * [Java Version](#java-version)
  * [Gradle](#gradle)
  * [Bzip2 Compression](#bzip2-compression)
* [Building Ice for Java Compat](#building-ice-for-java-compat-1)
* [Installing Ice for Java Compat](#installing-ice-for-java-compat)
* [Running the Java Compat Tests](#running-the-java-compat-tests)
* [Building the Ice for Android Tests](#building-the-ice-for-android-tests)

## Build Requirements

### Operating Systems

Ice for Java builds and runs properly on Windows, macOS, and any recent
Linux distribution, and is fully supported on the platforms listed on the
[supported platforms][2] page.

### Slice to Java Compiler

You need the Slice to Java compiler to build Ice for Java and also to use
Ice for Java. The Slice to Java compiler (`slice2java`) is a command-line tool
written in C++. You can build the Slice to Java compiler from source, or
alternatively you can install an Ice [binary distribution][1] that includes
this compiler.

### Java Version

Ice for Java requires J2SE 1.7.0 or later.

Make sure that the `javac` and `java` commands are present in your PATH.

### Gradle

Ice for Java uses the [Gradle][3] build system, and includes the Gradle wrapper
in the distribution. You cannot build the Ice for Java source distribution without 
an Internet connection. Gradle will download all required packages automatically 
from Maven Central repository located at http://central.maven.org/maven2/

### Bzip2 Compression

Ice for Java supports protocol compression using the bzip2 classes included
with [Apache Commons Compress][4].

The Maven package id for the commons-compress JAR file is as follows:

```
groupId=org.apache.commons, version=1.14, artifactId=commons-compress
```

The demos and tests are automatically setup to enable protocol compression by
adding the commons-compress JAR to the manifest class path. For your own
applications you must add the commons-compress JAR to the application CLASSPATH
to enable protocol compression.

> *These classes are a pure Java implementation of the bzip2 algorithm and
therefore add significant latency to Ice requests.*

## Building Ice for Java

The build system requires the Slice to Java compiler from Ice for C++. If you
have not built Ice for C++ in this source distribution, you must set the
`ICE_HOME` environment variable with the path name of your Ice installation. For
example, on Linux:

```
export ICE_HOME=/opt/Ice-3.7.1 (For local build)
export ICE_HOME=/usr (For RPM installation)
```

On Windows:

```
set ICE_HOME=C:\Program Files\ZeroC\Ice-3.7.1 (MSI installation)
```

On Windows if you are using Ice for C++ from a source distribution, you must set
the `CPP_PLATFORM` and `CPP_CONFIGURATION` environment variables to match the
platform and configuration used in your C++ build:

```
set CPP_PLATFORM=x64
set CPP_CONFIGURATION=Debug
```

The supported values for `CPP_PLATFORM` are `Win32` and `x64` and the supported
values for `CPP_CONFIGURATION` are `Debug` and `Release`.

Before building Ice for Java, review the settings in the file `gradle.properties`
and edit as necessary.

To build Ice, all services, and tests, run

```
gradlew build
```

Upon completion, the Ice JAR and POM files are placed in the `lib` subdirectory.

If at any time you wish to discard the current build and start a new one, use
these commands:

```
gradlew clean
gradlew build
```

## Installing Ice for Java

To install Ice for Java in the directory specified by the `prefix` variable in
`gradle.properties` run the following command:

```
gradlew install
```

The installation installs the following JAR files to `<prefix>/lib`.

```
glacier2-compat-3.7.1.jar
ice-compat-3.7.1.jar
icebox-compat-3.7.1.jar
icebt-compat-3.7.1.jar
icediscovery-compat-3.7.1.jar
icegrid-compat-3.7.1.jar
icelocatordiscovery-compat-3.7.1.jar
icepatch2-compat-3.7.1.jar
icestorm-compat-3.7.1.jar
```

POM files are also installed for ease of deployment to a Maven-based
distribution system.

## Running the Java Tests

Some of the Ice for Java tests employ applications that are part of the Ice for
C++ distribution. If you have not built Ice for C++ in this source distribution
then you must set the `ICE_HOME` environment variable with the path name of your
Ice installation. On Linux or macOS:

```
export ICE_HOME=/opt/Ice-3.7.1 (For local build)
export ICE_HOME=/usr (For RPM installation)
```

On Windows:

```
set ICE_HOME=C:\Program Files\ZeroC\Ice-3.7.1
```

Python is required to run the test suite. To run the tests, open a command
window and change to the top-level directory. At the command prompt, execute:

```
python allTests.py
```

If everything worked out, you should see lots of `ok` messages. In case of a
failure, the tests abort with `failed`.

## Building the Ice for Android Tests

The `test/android/controller` directory contains an Android Studio project
for the Ice test suite controller.

### Build Requirements

Building any Ice application for Android requires Android Studio and the Android
SDK build tools. We tested with the following components:

- Android Studio 3.0.1
- Android SDK 21
- Android Build Tools 27.0.1

Ice requires at minimum API level 21:

- Android 5 (API21)

If you want to target a later version of the Android API level for the test
suite, edit `test/android/controller/gradle.properties` and change the 
following variables:

```
ice_compileSdkVersion
ice_minSdkVersion
ice_targetSdkVersion
```

*NOTE: Do not use Android Studio to modify the project's settings.*

### Building the Android Test Controller

You must first build Ice for Java refer to [Building Ice for Java](#building-ice-for-java-1)
for instructions, then follow these steps:

1. Start Android Studio
2. Select "Open an existing Android Studio project"
3. Navigate to and select the "android" subdirectory
4. Click OK and wait for the project to open and build

## Running the Android Tests

The Android Studio project contains a `controller` app for the Ice test
suite. Prior to running the app, you must disable Android Studio's Instant Run
feature, located in File / Settings / Build, Execution, Deployment /
Instant Run.

Tests are started from the dev machine using the `allTests.py` script, similar
to the other language mappings. The script uses Ice for Python to communicate
with the Android app, therefore you must build the [Python mapping]
(../../python) before continuing.

You also need to add the `tools\bin`, `platform-tools` and `emulator`
directories from the Android SDK to your PATH. On macOS, you can use the
following commands:

```
export PATH=~/Library/Android/sdk/tools/bin:$PATH
export PATH=~/Library/Android/sdk/platform-tools:$PATH
export PATH=~/Library/Android/sdk/emulator:$PATH
```

On Windows, you can use the following commands:

```
set PATH=%LOCALAPPDATA%\Android\sdk\tools;%PATH%
set PATH=%LOCALAPPDATA%\Android\sdk\platform-tools;%PATH%
set PATH=%LOCALAPPDATA%\Android\sdk\emulator;%PATH%
```

Run the tests with the Android emulator by running the following command:

```
cd test/android
python allTests.py --androidemulator --controller-app
```

To run the tests on a specific Android connected device, you can use the
`--device` argument as shown below:

```
cd test/android
python allTests.py --device=ZX1C2234XF --controller-app
```

You can see the list of connected devices with the `adb` command:

```
adb devices -l
```

To run the tests against a `controller` application started from Android
Studio you should omit the `--controller-app` option from the commands above.

[1]: https://zeroc.com/distributions/ice
[2]: https://doc.zeroc.com/display/Rel/Supported+Platforms+for+Ice+3.7.1
[3]: https://gradle.org
[4]: https://commons.apache.org/proper/commons-compress/
