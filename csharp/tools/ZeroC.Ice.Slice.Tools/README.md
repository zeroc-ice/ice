# Slice Tools for Ice and CSharp

**ZeroC.Ice.Slice.Tools** provides an MSBuild task for compiling Slice definitions (`.ice` files) into C# source
files (`.cs` files) automatically as part of your build process.

[Source code][source] | [Package][package] | [slice2cs options][slice2cs] | [Slice documentation][slice]

## Features

- **Automatic Slice Compilation:** Compiles Slice `.ice` files to C# `.cs` files using the `slice2cs` compiler.
- **Incremental Build Support:** Only recompiles Slice `.ice` files that have changed or have outdated dependencies.
- **Bundled Slice-to-CSharp Compilers:** With binaries for Windows (x64), Linux (x64, arm64), and macOS (x64, arm64).
- **Customizable Compilation Options:** Allows configuring include directories, additional options, and output
  directories.

## Installation

To use this task, add the NuGet package to your project:

```sh
dotnet add package ZeroC.Ice.Slice.Tools
```

Once installed, Slice files are automatically compiled into C# files every time the project is built.

## Configuring Slice Compilation

By default, all `.ice` files in the project directory and its subdirectories are included in the build. You can
disable this behavior by setting either [`EnableDefaultItems`][default-items] or `EnableDefaultSliceCompileItems` to
`false`.

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

| Item Metadata      | Default Value                        | Corresponding `slice2cs` [Option][slice2cs] |
| ------------------ | ------------------------------------ | ------------------------------------------- |
| OutputDir          | $(MSBuildProjectDirectory)/generated | `--output-dir`                              |
| IncludeDirectories |                                      | `-I`                                        |
| AdditionalOptions  |                                      | (any)                                       |

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

[default-items]: https://learn.microsoft.com/en-us/dotnet/core/project-sdk/msbuild-props#enabledefaultitems
[package]: https://www.nuget.org/packages/ZeroC.Ice.Slice.Tools
[slice]: https://doc.zeroc.com/ice/3.7/the-slice-language
[slice2cs]: https://doc.zeroc.com/ice/3.7/language-mappings/c-sharp-mapping/client-side-slice-to-c-sharp-mapping/slice2cs-command-line-options
[source]: https://github.com/zeroc-ice/ice/tree/main/csharp/tools/ZeroC.Ice.Slice.Tools
