# Building Ice Slice Tools Gradle Plugin

This document describes how to build and publish the Ice Slice Tools Gradle Plugin.

## Prerequisites

- JDK 17 or greater

## Build

The plugin can be built using the following Gradle command:

```shell
./gradlew build
```

This will:

- Compile the source code
- Run tests
- Package the plugin

### Bundled Compiler and Ice Slice Files

To ensure that the Slice-to-Java compiler (slice2java) and the Ice Slice files are included with the plugin and
available at runtime, they must be copied to the plugin’s resources directory.

The plugin expects the following structure within src/main/resources/:

```shell
src/main/resources/slice         # Ice Slice files
src/main/resources/macos-arm64   # slice2java compiler for macOS (Apple Silicon)
src/main/resources/linux-arm64   # slice2java compiler for Linux (ARM64)
src/main/resources/linux-x64     # slice2java compiler for Linux (x86_64)
src/main/resources/windows-x64   # slice2java compiler for Windows (x86_64)
```

## Publish

The plugin can be published to **Maven Local** for testing or to a **remote repository** for distribution.

### Publish to Maven Local (For Development)

To publish the plugin to your local Maven repository (`~/.m2/repository`), run:

```shell
./gradlew publishToMavenLocal
```

This allows other projects on your machine to use the locally published version.

### Publish to a Remote Repository (Maven Central, Artifactory, etc.)

To publish the plugin to a remote repository, ensure your **Gradle properties** (`~/.gradle/gradle.properties`) contain the necessary credentials:

```shell
mavenRepository=repository-url
mavenUsername=your-username
mavenPassword=your-password
```

Then, publish the plugin using:

```shell
./gradlew publish
```

### Publish to the Gradle Plugin Portal

To publish the plugin to the **Gradle Plugin Portal**, follow these steps:

- Add the following to your `gradle.properties`:

```shell
gradle.publish.key=your-key
gradle.publish.secret=your-secret
```

- Run the following command to publish:

```shell
./gradlew publishPlugins
```
