# Building Ice for Java

This page describes how to build and install Ice for Java from source. If
you prefer, you can also download a [binary distribution][1].

* [Build Requirements](#build-requirements)
  * [Operating Systems](#operating-systems)
  * [Slice to Java Compiler](#slice-to-java-compiler)
  * [Java Version](#java-version)
  * [Gradle](#gradle)
  * [Bzip2 Compression](#bzip2-compression)
  * [JGoodies](#jgoodies)
  * [ProGuard](#proguard)
  * [Java Application Bundler](#java-application-bundler)
* [Compiling Ice for Java](#compiling-ice-for-java)
  * [Preparing to Build](#preparing-to-build)
  * [Building Ice for Java](#building-ice-for-java-1)
* [Installing Ice for Java](#installing-ice-for-java)
* [Running the Java Tests](#running-the-java-tests)
* [IceGrid GUI Tool](#icegrid-gui-tool)

## Build Requirements

### Operating Systems

Ice for Java builds and runs properly on Windows, macOS, and any recent Linux
distribution, and is fully supported on the platforms listed on the
[supported platforms][2] page.

### Slice to Java Compiler

You need the Slice to Java compiler to build Ice for Java and also to use
Ice for Java. The Slice to Java compiler (`slice2java`) is a command-line tool
written in C++. You can build the Slice to Java compiler from source, or
alternatively you can install an Ice [binary distribution][1] that includes
this compiler.

### Java Version

Ice for Java requires J2SE 1.8 or later.

Make sure that the `javac` and `java` commands are present in your PATH.

The Metrics Graph feature of the IceGrid GUI tool requires JavaFX support. This
feature will not be available if you build the source with a JVM that lacks
support for JavaFX. Alternatively, building the source in an environment with
JavaFX produces an IceGrid GUI JAR file that can be used in JVMs with or without
JavaFX support, as the Metrics Graph feature is enabled dynamically.

### Gradle

Ice for Java uses the [Gradle][3] build system, and includes the Gradle wrapper
version 2.4 in the distribution. You cannot build the Ice for Java source
distribution without an Internet connection. Gradle will download all required
distribution without an Internet connection. Gradle will download all required
packages automatically from ZeroC's Maven repository located at

    https://repo.zeroc.com/nexus/content/repositories/thirdparty

### Bzip2 Compression

Ice for Java supports protocol compression using the bzip2 classes included
with [Apache Commons Compress][5].

The Maven package id for the commons-compress JAR file is as follows:

    groupId=org.apache.commons, version=1.14, artifactId=commons-compress

The demos and tests are automatically setup to enable protocol compression by
adding the commons-compress JAR to the manifest class path. For your own
applications you must add the commons-compress JAR to the application CLASSPATH
to enable protocol compression.

> *These classes are a pure Java implementation of the bzip2 algorithm and
therefore add significant latency to Ice requests.*

### JGoodies

The IceGrid GUI tool uses the JGoodies libraries Forms and Looks. The following
versions were tested:

    JGoodies Forms 1.9.0
    JGoodies Looks 2.7.0

The Maven package ids for the JGoodies packages are as follows:

    groupId=com.jgoodies, version=1.9.0, artifactId=jgoodies-forms
    groupId=com.jgoodies, version=2.7.0, artifactId=jgoodies-looks

### ProGuard

Gradle uses [ProGuard][4] to create the standalone JAR file for the IceGrid
GUI tool.

The Maven package id for the ProGuard package is as follows:

    groupId='net.sf.proguard', artifactId='proguard-gradle', version='6.0'

### Java Application Bundler

Under macOS Gradle uses the Java Application Bundler to create an application
bundle for the IceGrid GUI tool.

The Maven package id for the application bundler package is as follows:

    groupId=com.oracle, version=1.0, artifactId=appbundler

## Compiling Ice for Java

### Preparing to Build

The build system requires the Slice to Java compiler from Ice for C++. If you
have not built Ice for C++ in this source distribution, you must set the
`ICE_BIN_DIST` environment variable to `cpp` and the `ICE_HOME` environment
variable with the path name of your Ice installation. For example, on Linux with
an RPM installation:
```
export ICE_BIN_DIST=cpp
export ICE_HOME=/usr
```

On Windows with an MSI installation:
```
set ICE_BIN_DIST=cpp
set ICE_HOME=C:\Program Files\ZeroC\Ice-3.7.1
```

If you are using Ice for C++ from a source build on Windows, you must set
the `CPP_PLATFORM` and `CPP_CONFIGURATION` environment variables to match the
platform and configuration used in your C++ build:
```
set CPP_PLATFORM=x64
set CPP_CONFIGURATION=Debug
```

The supported values for `CPP_PLATFORM` are `Win32` and `x64` and the supported
values for `CPP_CONFIGURATION` are `Debug` and `Release`.

Before building Ice for Java, review the settings in the file
`gradle.properties` and edit as necessary.

### Building Ice for Java

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

The following JAR files will be installed to `<prefix>/lib`.

    glacier2-3.7.1.jar
    ice-3.7.1.jar
    icebox-3.7.1.jar
    icebt-3.7.1.jar
    icediscovery-3.7.1.jar
    icegrid-3.7.1.jar
    icegridgui.jar
    icelocatordiscovery-3.7.1.jar
    icepatch2-3.7.1.jar
    icessl-3.7.1.jar
    icestorm-3.7.1.jar

POM files are also installed for ease of deployment to a Maven-based
distribution system.

## Running the Java Tests

Some of the Ice for Java tests employ applications that are part of the Ice for
C++ distribution. If you have not built Ice for C++ in this source distribution
then you must set the `ICE_HOME` environment variable with the path name of your
Ice installation. On Unix:
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

## IceGrid GUI Tool

Ice for Java includes the IceGrid GUI tool. It can be found in the file
`lib/icegridgui.jar`.

This JAR file is completely self-contained and has no external dependencies.
You can start the tool with the following command:
```
java -jar icegridgui.jar
```

On macOS, the build also creates an application bundle named IceGrid GUI. You
can start the IceGrid GUI tool by double-clicking the IceGrid GUI icon in
Finder.

[1]: https://zeroc.com/distributions/ice
[2]: https://doc.zeroc.com/display/Rel/Supported+Platforms+for+Ice+3.7.1
[3]: https://gradle.org
[4]: http://proguard.sourceforge.net
[5]: https://commons.apache.org/proper/commons-compress/
