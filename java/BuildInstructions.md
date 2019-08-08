
# Building Ice for Java

This page describes how to build and install Ice for Java from source code. If
you prefer, you can also download [binary distributions][1] for the supported
platforms.

## Build Requirements

### Operating Systems

Ice for Java is expected to build and run properly on Windows, macOS, and any
recent Linux distribution for x86 and x86_64, and was extensively tested using
the operating systems and compiler versions listed on the [supported platforms][2].

### Slice to Java Compiler

You will need the Slice-to-Java compiler (`slice2java`) for your build platform.
`slice2java` is a C++ application included in the Ice binary distributions.

### Java Version

Ice for Java requires J2SE 1.7.0 or later.

The Metrics Graph feature of the graphical IceGrid administrative tool requires
J2SE 7u6 or later with JavaFX support. This feature will not be available if you
build the source with a JVM that lacks support for JavaFX. Alternatively,
building the source using J2SE 7u6 or later with JavaFX produces a JAR file that
can be used in JVMs with or without JavaFX support, as the Metrics Graph feature
is enabled dynamically.

Make sure that the `javac` and `java` commands are present in your PATH.

### Berkeley DB

"Freeze" is an optional Ice component that provides a persistence facility for
Ice applications. Freeze uses Berkeley DB as its underlying database and
currently requires Berkeley DB version 5.3 (the recommended version is 5.3.28).

In order to run an application that uses Freeze, you must add `db.jar` to your
CLASSPATH and verify that the Berkeley DB shared library is in your
`java.library.path`.

### Gradle

Ice for Java uses the [Gradle][3] build system, and includes the Gradle wrapper
version 2.4 in the distribution. Except for Berkeley DB, Gradle will
download all required packages automatically. These packages are listed below.
Gradle will automatically download any necessary build artifacts from the Maven
Central repository.

### Bzip2 Compression

Ice for Java supports protocol compression using the bzip2 classes included
with Apache Ant. The Maven package id for the bzip2 JAR file in ZeroC's Maven
repository is as follows:
```
groupId=org.apache.tools, version=1.0, artifactId=bzip2
```

The demos and tests are automatically setup to enable protocol compression by
adding the bzip2 JAR to the manifest class path. For your own applications you
must add the bzip2 JAR to the application CLASSPATH to enable protocol
compression.

> *These classes are a pure Java implementation of the bzip2 algorithm and
therefore add significant latency to Ice requests.*

### JGoodies

The graphical IceGrid administrative tool uses the JGoodies libraries Forms,
and Looks. The following versions were tested:

    JGoodies Forms 1.9.0
    JGoodies Looks 2.7.0

The Maven package ids for the JGoodies packages are as follows:

    groupId=com.jgoodies, version=1.9.0, artifactId=jgoodies-forms
    groupId=com.jgoodies, version=2.7.0, artifactId=jgoodies-looks

### ProGuard

Gradle uses [ProGuard][4] to create the standalone JAR file for the graphical
IceGrid administrative tool.

The Maven package id for the ProGuard package is as follows:

    groupId='net.sf.proguard', artifactId='proguard-gradle', version='6.1.1'

### Java Application Bundler

Under macOS Gradle uses the Java Application Bundler to create an application
bundle for the graphical IceGrid administrative tool.

The Maven package id for the application bundler package is as follows:

    groupId=com.oracle, version=1.0, artifactId=appbundler

## Compiling Ice for Java

### Preparing to Build

This source distribution cannot be compiled successfully without the Berkeley DB
run time for Java (`db.jar`). The build system searches in standard locations
for the following two JAR files:

    db-5.3.28.jar
    db.jar

If neither of these files is present in the standard locations on your system,
you must set `dbHome` in `gradle.properties`.

The build system also requires the Slice translators from Ice for C++. If you
have not built Ice for C++ in this source distribution, you must set `USE_BIN_DIST`
enviroment variable to `yes` and the `ICE_HOME` environment variable with the path
name of your Ice installation. For example, on Linux:
```
export USE_BIN_DIST=yes
export ICE_HOME=/opt/Ice-3.6.5 (For local build)
export ICE_HOME=/usr (For RPM installation)
```

On Windows:
```
set USE_BIN_DIST=yes
set ICE_HOME=C:\Program Files (x86)\ZeroC\Ice-3.6.5
```

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
`gradle.properties` run the following command
```
gradlew install
```

The installation installs the following JAR files to `<prefix>/lib`.

    freeze-3.6.5.jar
    glacier2-3.6.5.jar
    ice-3.6.5.jar
    icebox-3.6.5.jar
    icediscovery-3.6.5.jar
    icegrid-3.6.5.jar
    icegridgui.jar
    icelocatordiscovery-3.6.5.jar
    icepatch2-3.6.5.jar
    icestorm-3.6.5.jar

POM files are also installed for ease of deployment to a maven-based
distribution system.

## Running the Java Tests

Some of the Ice for Java tests employ applications that are part of the Ice for
C++ distribution. If you have not built Ice for C++ in this source distribution
then you must set the `ICE_HOME` environment variable with the path name of your
Ice installation. On Linux:
```
export ICE_HOME=/opt/Ice-3.6.5 (For local build)
export ICE_HOME=/usr (For RPM installation)
```

On Windows:
```
set ICE_HOME=c:\Program Files (x86)\ZeroC\Ice-3.6.5
```

Python is required to run the test suite. To run the tests, open a command
window and change to the top-level directory. At the command prompt, execute:
```
python allTests.py
```

You can also run tests individually by changing to the test directory and
running this command:
```
python run.py
```

If everything worked out, you should see lots of `ok` messages. In case of a
failure, the tests abort with `failed`.

## IceGrid Admin Tool

Ice for Java includes a graphical administrative tool for IceGrid. It can be
found in the file `lib/icegridgui.jar`.

The JAR file is completely self-contained and has no external dependencies.
You can start the tool with the following command:
```
java -jar icegridgui.jar
```

In macOS there is also an application bundle named IceGrid Admin. You can start
the IceGrid Admin tool by double-clicking the IceGrid Admin icon in Finder.

## Test Controller

The test Controller is an Ice server that allows to start servers for languages
that doesn't provide a full server side. This is currently used by JavaScript
and UWP test suites to start the required servers.
```
gradlew :testController:assemble
```

[1]: https://zeroc.com/downloads/ice/3.6
[2]: https://doc.zeroc.com/rel/ice-releases/ice-3-6/ice-3-6-5-release-notes/supported-platforms-for-ice-3-6-5
[3]: http://gradle.org
[4]: http://proguard.sourceforge.net
