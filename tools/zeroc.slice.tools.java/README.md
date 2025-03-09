# Slice Tools Gradle Plugin

The **Slice Tools Gradle Plugin** integrates the **ZeroC Slice-to-Java compiler (`slice2java`)** into Gradle-based **Java** and **Android** projects. It automates the compilation of Slice (`.ice`) files into Java source files and integrates seamlessly with the Gradle build system.

## Features

- **Automatic Slice Compilation**: Generates Java classes from `.ice` files using `slice2java`.
- **Supports Both Java and Android Projects**: Integrates with the Gradle Java and Android plugins.
- **Incremental Compilation**: Compiles only modified `.ice` files.
- **Automatic Dependency Management**: Detects and handles Slice file dependencies.
- **Customizable Compiler Arguments**: Supports `-I`, `-D`, and other `slice2java` options.
- **Automatic Tool Extraction**: Bundles `slice2java` for easy use without additional installation.

## Installation

### Applying the Plugin

Apply the plugin to your `build.gradle` (Groovy) or `build.gradle.kts` (Kotlin):

**Groovy DSL:**

```groovy
plugins {
    id 'com.zeroc.slicetools' version '1.0.0'
}
```

**Kotlin DSL:**

```kotlin
plugins {
    id("com.zeroc.slicetools") version "1.0.0"
}
```

## Configuration

The plugin provides the `slice` extension for configuring Slice compilation settings.

### Common Configuration

**Groovy DSL:**

```groovy
slice {
    toolsPath = "/path/to/slice2java"
    includeSearchPath.from("src/main/slice/includes")
    compilerArgs.addAll("-DDEBUG")
}
```

**Kotlin DSL:**

```kotlin
slice {
    toolsPath.set("/path/to/slice2java")
    includeSearchPath.from("src/main/slice/includes")
    compilerArgs.addAll("-DDEBUG")
}
```

### Slice Extension Properties

| Property Name       | Description |
|---------------------|-------------|
| `toolsPath`        | Path to the `slice2java` compiler. If not set, the plugin uses the bundled compiler. |
| `includeSearchPath` | Directories to search for Slice include files (`-I` option). |
| `compilerArgs`     | List of additional arguments passed to `slice2java`. |
| `sourceSets`       | Defines multiple Slice source sets in the project. |

### Source Sets

The plugin extends Gradle's standard **source sets** with Slice-specific properties. A **Slice source set** defines where `.ice` files are located and how they should be compiled.

Each source set has additional properties compared to standard Java source sets:

| Property Name        | Description |
|----------------------|-------------|
| `srcDir`            | Directory containing `.ice` files for this source set. |
| `includeSearchPath` | Directories passed to the Slice compiler using the `-I` option. |
| `compilerArgs`      | Additional arguments passed to the `slice2java` compiler for this source set. |

### Java Project Configuration

For **Java projects**, the plugin automatically registers Slice source sets for each Java source set (`main`, `test`, etc.). Place your `.ice` files in `src/main/slice/`.

```groovy
slice {
    sourceSets {
        main {
            srcDir "src/main/slice"
        }
    }
}
```

The generated Java files are automatically included in `sourceSets.main.java.srcDirs`.

### Android Project Configuration

For **Android projects**, the plugin integrates with the Android build system and registers Slice source sets for each Android source set (e.g., `main`, `debug`, `release`).

```groovy
android {
    compileSdkVersion 34
    defaultConfig {
        applicationId "com.example.app"
    }
}

slice {
    sourceSets {
        main {
            srcDir "src/main/slice"
        }
    }
}
```

The plugin ensures that the generated Java files are included in the correct Android variant's Java sources.

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
    id 'com.zeroc.slicetools' version '3.8.0-alpha.0'
}

slice {
    sourceSets {
        main {
            srcDir 'src/main/slice'
        }
    }
}
```

With an `.ice` file in `src/main/slice/Greeter.ice`:

```slice
[["java:package:com.zeroc.demos"]]
module VisitorCenter
{
    /// Represents a simple greeter.
    interface Greeter
    {
        /// Creates a personalized greeting.
        /// @param name The name of the person to greet.
        /// @return The greeting.
        string greet(string name);
    }
}
```

Running `gradle compileSlice` will generate java sources in `build/generated/source/slice/main/`.
