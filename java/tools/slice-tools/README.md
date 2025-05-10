# Slice Tools for Ice and Java

The **Slice Tools for Ice and Java** integrates the **Slice-to-Java compiler (`slice2java`)** into Gradle-based
**Java** and **Android** projects. It automates the compilation of Slice (`.ice`) files into Java source files and
integrates seamlessly with the Gradle build system.

> Note: This Gradle plugin replaces the Ice Builder for Gradle plugin (com.zeroc.gradle.ice-builder.slice). While the
> Ice Builder for Gradle is still compatible with Ice 3.8, we recommend using the Slice Tools for Ice and Java
> (com.zeroc.ice.slice-tools) for better integration with Ice 3.8.

## Features

- **Automatic Slice Compilation**: Compiles Slice definitions in `.ice` files into Java `.java` files using
`slice2java`.
- **Supports Both Java and Android Projects**: Integrates with the Gradle Java and Android plugins.
- **Incremental Compilation**: Compiles only modified `.ice` files.
- **Automatic Dependency Management**: Detects and handles Slice file dependencies.
- **Customizable Compiler Arguments**: Supports `-I`, `-D`, and other `slice2java` options.
- **Automatic Slice Compiler Extraction**: The plugin includes the Slice for Java compiler, `slice2java`, with binaries
for Linux (x64 and arm64), macOS (x64 and arm64), and Windows (x64).

## Installation

### Applying the Plugin

Apply the plugin to your `build.gradle` (Groovy) or `build.gradle.kts` (Kotlin):

**Groovy DSL:**

```groovy
plugins {
    id 'com.zeroc.ice.slice-tools' version '3.8.0-alpha.0'
}
```

**Kotlin DSL:**

```kotlin
plugins {
    id("com.zeroc.ice.slice-tools") version "3.8.0-alpha.0"
}
```

## Configuration

The plugin provides the `slice` extension for configuring Slice compilation settings.

### Slice Extension Properties

| Property Name       | Description                                                                                                   |
|---------------------|---------------------------------------------------------------------------------------------------------------|
| `toolsPath`         | Path to the directory containing the `slice2java` compiler. If not set, the plugin uses the bundled compiler. |
| `includeSearchPath` | Directories to search for Slice include files (`-I` option).                                                  |
| `compilerArgs`      | List of additional arguments passed to `slice2java`.                                                          |
| `sourceSets`        | Defines multiple Slice source sets in the project.                                                            |

These settings apply to all Slice source sets compiled with the plugin.

### Sample Configuration

**Groovy DSL:**

```groovy
slice {
    includeSearchPath.from("src/main/slice/includes")
    compilerArgs.addAll("-DDEBUG")
}
```

**Kotlin DSL:**

```kotlin
slice {
    includeSearchPath.from("src/main/slice/includes")
    compilerArgs.addAll("-DDEBUG")
}
```

### Source Sets

The plugin extends Gradle's standard **source sets** with Slice-specific properties. A **Slice source set** defines
where `.ice` files are located and how they should be compiled.

| Property Name       | Description                                                                   |
|---------------------|-------------------------------------------------------------------------------|
| `srcDir`            | Directory containing `.ice` files for this source set.                        |
| `includeSearchPath` | Directories passed to the Slice compiler using the `-I` option.               |
| `compilerArgs`      | Additional arguments passed to the `slice2java` compiler for this source set. |

### Java Project Configuration

For **Java projects**, the plugin automatically registers Slice source sets for each Java source set (`main`, `test`,
etc.). Place your `.ice` files in `src/<sourceSet>/slice/`. The generated Java files are automatically included in
`sourceSets.<sourceSet>.java.srcDirs`.

If you want to include Slice files from a different location or specify additional options for compiling the Slice
files, use the Slice source DSL.

**Groovy DSL:**

```groovy
sourceSets {
    main {
        slice {
            srcDir "slice"
            includeSearchPath.from("../common/slice")
        }
    }
}
```

**Kotlin DSL:**

```kotlin
sourceSets {
    named("main") {
        slice {
            srcDir("slice")
            includeSearchPath.from("../common/slice")
        }
    }
}
```

### Android Project Configuration

For **Android projects**, the plugin integrates with the Android build system and registers Slice source sets for each
Android source set (e.g., `main`, `debug`, `release`). Place your `.ice` files in `src/<sourceSet>/slice/`.
The generated Java files are automatically included in `android.sourceSets.<sourceSet>.java.srcDirs`.

**Groovy DSL:**

```groovy
android {
    compileSdkVersion 34
    sourceSets {
        main {
            slice {
                srcDir "slice"
                includeSearchPath.from("../common/slice")
            }
        }
    }
}
```

**Kotlin DSL:**

```kotlin
android {
    compileSdkVersion 34
    sourceSets.getByName("main") {
        slice {
            srcDir("slice")
            includeSearchPath.from("../common/slice")
        }
    }
}
```

## Tasks

### `compileSlice`

Compiles all `.ice` files and generates Java source files.

### Per-Source Set Tasks

The plugin creates a `compileSlice<SourceSet>` task for each source set:

- **Java**: `compileSliceMain`, `compileSliceTest`
- **Android**: `compileSliceDebug`, `compileSliceRelease`

## Example Project

A minimal Java project with Slice support:

```groovy
plugins {
    id 'java'
    id 'com.zeroc.ice.slice-tools' version '3.8.0-alpha.0'
}
```

With an `.ice` file in `src/main/slice/Greeter.ice`:

```slice
["java:identifier:com.zeroc.demos.VisitorCenter"]
module VisitorCenter {
    interface Greeter {
        string greet(string name);
    }
}
```

Running `gradle compileSlice` will generate Java sources in `build/generated/source/slice/main/` using the `slice2java`
compiler included with the Ice Slice Tools plugin.
