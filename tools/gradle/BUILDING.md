# Building Slice Tools for Ice

This document describes how to build and publish the Slice Tools for Ice Gradle Plugin.

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

### Bundled Compilers and Ice Slice Files

To ensure that the Slice-to-Java compiler (slice2java) and the Ice Slice files are included with the plugin and
available at runtime, they must be copied to the plugin’s resources directory.

The plugin expects the following structure within src/main/resources/:

```shell
src/main/resources/slice         # Ice Slice files
src/main/resources/macos-arm64   # slice2java compiler for macOS (Apple Silicon)
src/main/resources/linux-arm64   # slice2java compiler for Linux (ARM64)
src/main/resources/linux-x64     # slice2java compiler for Linux (x86_64)
src/main/resources/windows-x64   # slice2java compiler for Windows (x64)
```

## Publish

The plugin can be published to **Maven Local** for testing.

### Publish to Maven Local (For Development)

To publish the plugin to your local Maven repository (`~/.m2/repository`), run:

```shell
./gradlew publishToMavenLocal
```

This allows other projects on your machine to use the locally published version.
