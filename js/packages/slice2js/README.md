# @zeroc/slice2js

The Slice-to-JavaScript compiler and build plugin for [Ice].

This package provides:

- The `slice2js` compiler, which compiles Slice definitions (`.ice` files) into JavaScript and TypeScript.
- Build plugins for [Vite], [Rollup], [webpack], and [esbuild] via [unplugin].
- A programmatic API for running the compiler.

## Installation

```bash
npm install --save-dev @zeroc/slice2js
```

## Build Plugin

The build plugin compiles `.ice` files automatically during your build. It supports glob patterns and watches `.ice`
files for changes in development mode.

### Vite

```js
// vite.config.js
import slice2js from "@zeroc/slice2js/unplugin/vite";

export default {
    plugins: [
        slice2js({
            inputs: ["slice/*.ice"],
            outputDir: "src/generated",
            args: ["--typescript"],
        }),
    ],
};
```

### Rollup

```js
// rollup.config.js
import slice2js from "@zeroc/slice2js/unplugin/rollup";

export default {
    plugins: [
        slice2js({
            inputs: ["slice/*.ice"],
            outputDir: "src/generated",
            args: ["--typescript"],
        }),
    ],
};
```

### webpack

```js
// webpack.config.js
import slice2js from "@zeroc/slice2js/unplugin/webpack";

export default {
    plugins: [
        slice2js({
            inputs: ["slice/*.ice"],
            outputDir: "src/generated",
            args: ["--typescript"],
        }),
    ],
};
```

### esbuild

```js
import esbuild from "esbuild";
import slice2js from "@zeroc/slice2js/unplugin/esbuild";

await esbuild.build({
    plugins: [
        slice2js({
            inputs: ["slice/*.ice"],
            outputDir: "src/generated",
            args: ["--typescript"],
        }),
    ],
});
```

### Plugin Options

| Option      | Type                 | Description                                                      |
|-------------|----------------------|------------------------------------------------------------------|
| `inputs`    | `string \| string[]` | Slice files or glob patterns (e.g. `"**/*.ice"`)                 |
| `outputDir` | `string`             | Output directory for generated `.js` and `.d.ts` files           |
| `include`   | `string \| string[]` | Additional Slice include directories (`-I` flags)                |
| `args`      | `string \| string[]` | Additional slice2js CLI arguments (e.g. `--typescript`)          |
| `cwd`       | `string`             | Base directory for resolving paths (defaults to `process.cwd()`) |

## Programmatic API

### `runSlice2js(options)`

Compiles Slice files with glob support and include directory resolution:

```js
import { runSlice2js } from "@zeroc/slice2js/unplugin";

await runSlice2js({
    inputs: ["slice/*.ice"],
    outputDir: "src/generated",
    args: ["--typescript"],
});
```

### `compile(args, options)`

Low-level function that spawns the `slice2js` compiler directly:

```js
import { compile } from "@zeroc/slice2js";

const exitCode = await compile(["--typescript", "--output-dir", "src/generated", "slice/Hello.ice"]);
```

[Ice]: https://github.com/zeroc-ice/ice
[Vite]: https://vitejs.dev
[Rollup]: https://rollupjs.org
[webpack]: https://webpack.js.org
[esbuild]: https://esbuild.github.io
[unplugin]: https://github.com/unjs/unplugin
