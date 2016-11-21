# Building Ice for Java

This page describes how to build and install Ice for Java from source code. If
you prefer, you can also download [binary distributions][1] for the supported
platforms.

## Build Requirements

### Operating Systems

Ice for Java is expected to build and run properly on Windows, OS X, and any
recent Linux distribution for x86 and x86_64, and was extensively tested using
the operating systems and compiler versions listed for our [supported
platforms][2]. Due to the portability of Java, it is very likely that it will
also work on other platforms for which a suitable Java implementation is
available.

### Slice to Java Translator

You will need the Slice to Java translator. ZeroC provides translator binaries
for our supported platforms. For other platforms, you will have to either port
Ice for C++ (which contains the Slice to Java translator), or you will have to
translate your Slice files to Java on a supported platform and then copy the
generated Java files to your target platform.

### Java Version

Ice for Java requires J2SE 1.7.0 or later.

Make sure that the `javac` and `java` commands are present in your PATH.

### Gradle

Ice for Java uses the [Gradle][3] build system, and includes the Gradle wrapper
version 2.4 in the distribution. You cannot build the Ice for Java source
distribution without an Internet connection. Gradle will download all required
packages automatically. These packages are listed below. Gradle will
automatically download any necessary build artifacts from ZeroC's Maven
repository located at

    http://repo.zeroc.com/nexus/content/repositories/thirdparty

### Bzip2 Compression

Ice for Java supports protocol compression using the bzip2 classes included
with Apache Ant or available separately from [kohsuke.org]().

The Maven package id for the bzip2 JAR file is as follows:

    groupId=org.apache.tools, version=1.0, artifactId=bzip2

The demos and tests are automatically setup to enable protocol compression by
adding the bzip2 JAR to the manifest class path. For your own applications you
must add the bzip2 JAR to the application CLASSPATH to enable protocol
compression.

> *These classes are a pure Java implementation of the bzip2 algorithm and
therefore add significant latency to Ice requests.*

## Compiling Ice for Java

### Preparing to Build

The build system  requires the Slice translators from Ice for C++. If you have
not built Ice for C++ in this source distribution, you must set the `ICE_HOME`
environment variable with the path name of your Ice installation. For example,
on Unix:

    $ export ICE_HOME=/opt/Ice-3.7a3 (For local build)
    $ export ICE_HOME=/usr (For RPM installation)

On Windows:

    > set ICE_HOME=C:\Program Files (x86)\ZeroC\Ice-3.7a3

You will also need to set `CPP_PLATFORM` and `CPP_CONFIGURATION` to much your C++
build configuration in order for gradle o locate the slice2java compiler

For example if you build C++ sources for `x64` platform in `Release` mode set
this properties as:

    > set CPP_PLATFORM=x64
    > set CPP_CONFIGURATION=Debug

The supported values for `CPP_PLATFORM` are `Win32` and `x64`and the supported
values for `CPP_CONFIGURATION` are `Debug` and `Release`

Before building Ice for Java, review the settings in the file
`gradle.properties` and edit as necessary.

### Building Ice for Java

To build Ice, all services, and tests, run

    > gradlew build

Upon completion, the Ice JAR and POM files are placed in the `lib` subdirectory.

If at any time you wish to discard the current build and start a new one, use
these commands:

    > gradlew clean
    > gradlew build

## Installing Ice for Java

To install Ice for Java in the directory specified by the `prefix` variable in
`gradle.properties` run the following command

    > gradlew install

The installation installs the following JAR files to `<prefix>/lib`.

    glacier2-3.7a3.jar
    ice-3.7a3.jar
    icebox-3.7a3.jar
    icediscovery-3.7a3.jar
    icegrid-3.7a3jar
    icegridgui.jar
    icelocatordiscovery-3.7a3.jar
    icepatch2-3.7a3.jar
    icestorm-3.7a3jar

POM files are also installed for ease of deployment to a maven-based
distribution system.

## Running the Java Tests

Some of the Ice for Java tests employ applications that are part of the Ice for
C++ distribution. If you have not built Ice for C++ in this source distribution
then you must set the `ICE_HOME` environment variable with the path name of your
Ice installation. On Unix:

    $ export ICE_HOME=/opt/Ice-3.7a3 (For local build)
    $ export ICE_HOME=/usr (For RPM installation)

On Windows:

    > set ICE_HOME=c:\Program Files (x86)\ZeroC\Ice-3.7a3

Python is required to run the test suite. To run the tests, open a command
window and change to the top-level directory. At the command prompt, execute:

    > python allTests.py

You can also run tests individually by changing to the test directory and
running this command:

    > python run.py

If everything worked out, you should see lots of `ok` messages. In case of a
failure, the tests abort with `failed`.

## Test Controller

The test Controller is an Ice server that starts servers for languages that
don't provide a full server side implementation. This is currently used by
the JavaScript and WinRT test suites to start the required servers.

    > gradlew :testController:assemble


[1]: https://zeroc.com/download.html
[2]: https://doc.zeroc.com/display/Ice37/Supported+Platforms+for+Ice+3.7.0
[3]: http://gradle.org
