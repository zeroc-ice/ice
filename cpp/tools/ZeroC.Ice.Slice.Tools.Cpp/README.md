# Slice Tools for Ice and C++

**ZeroC.Ice.Slice.Tools.Cpp** project provides an MSBuild task for compiling Slice definitions (`.ice` files) into C++
source files (`.cpp` files) automatically as part of your build process.

[Source code][source] | [Package][package] | [slice2cpp options][slice2cpp] | [Slice documentation][slice]

## Features

- **Automatic Slice Compilation:** Compiles Slice `.ice` files to C++ `.cpp` files using the `slice2cpp` compiler.
- **Incremental Build Support:** Only recompiles Slice `.ice` files that have changed or have outdated dependencies.
- **Customizable Compilation Options:** Allows configuring include directories, additional options, and output directories.

## Installation

This task is distributed with the Ice for C++ `ZeroC.Ice.Cpp` NuGet package. Installing the package on a VC++ project
will automatically add the task to the project build.

## Configuring Slice Compilation

By default, all `.ice` files in the project directory and its subdirectories are included in the build. You can
disable this behavior by setting `EnableDefaultSliceCompileItems` to `false`.

For example, to disable automatic inclusion of Slice files, add the following to your `.csproj` file:

```xml
<PropertyGroup>
  <EnableDefaultSliceCompileItems>false</EnableDefaultSliceCompileItems>
</PropertyGroup>
```

To manually specify which `.ice` files to compile, use the `SliceCompile` item type. This is useful when you want to
compile Slice files from a directory outside the project directory:

```xml
<ItemGroup>
  <SliceCompile Include="../slice/Greeter.ice" />
</ItemGroup>
```

### Metadata for `SliceCompile` Items

Each `SliceCompile` item can have metadata attributes that customize the compilation:

| Item Metadata                    | Default Value | Corresponding `slice2cpp` [Option][slice2cpp] |
| -------------------------------- | ------------- | --------------------------------------------- |
| OutputDir                        | $(IntDir)     | `--output-dir`                                |
| HeaderOutputDir                  |               | (none)                                        |
| IncludeDirectories               |               | `-I`                                          |
| BaseDirectoryForGeneratedInclude |               | `--include-dir`                               |
| HeaderExt                        | .h            | `--header-ext`                                |
| SourceExt                        | .cpp          | `--source-ext`                                |
| AdditionalOptions                |               | (any)                                         |

Example:

```xml
<ItemGroup>
  <SliceCompile Include="../slice/Greeter.ice">
    <IncludeDirectories>../slice/includes</IncludeDirectories>
    <AdditionalOptions>-DFOO</AdditionalOptions>
    <OutputDir>$(IntermediateOutputPath)Generated</OutputDir>
  </SliceCompile>
</ItemGroup>
```

[package]: https://www.nuget.org/packages/ZeroC.Ice.Cpp
[slice]: https://doc.zeroc.com/ice/3.7/the-slice-language
[slice2cpp]: https://doc.zeroc.com/ice/latest/language-mappings/c++-mapping/slice2cpp-command-line-options
[source]: https://github.com/zeroc-ice/ice/tree/main/cpp/tools/ZeroC.Ice.Slice.Tools.Cpp
