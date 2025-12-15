# CompileSlice Plugin

The `CompileSlice` plugin is a SwiftPM plugin that compiles Slice files to Swift source files. This allows
compiling Slice files as part of the SwiftPM build process, without having to manually run the `slice2swift`
compiler.

To use the plugin with a target that relies on generated Swift files, add the plugin to the target's plugins list:

```swift
.target(
    name: "MyTarget",
    ...,
    plugins: [.plugin(name: "CompileSlice", package: "ice"),
    ]
)
```

Next, create a `slice-plugin.json` file in the target's source directory. The file will contain the
configuration for the plugin, including the list of sources to compile.
All paths are relative to the target's source directory.

| Key            | Description                                                             |
| -------------- | ----------------------------------------------------------------------- |
| `sources`      | List of Slice files or directories (non-recursive) to compile.          |
| `search_paths` | List of directories to search for included Slice files. Can be omitted. |

Example `slice-plugin.json`:

```json
{
    "sources": [ "Greeter.ice" ],
    "search_paths": [ "../Slice" ]
}
```

> [!NOTE]
> The `slice-plugin.json` file is discovered automatically by the plugin. It must be located in the target's source
> directory.
>
> Only one `slice-plugin.json` file is allowed per target.
