# Ice For JavaScript

[Examples] | [NPM packages] | [Documentation] | [API Reference] | [Building from source]

The [Ice framework] provides everything you need to build networked applications,
including RPC, pub/sub, server deployment, and more.

Ice for JavaScript is the JavaScript implementation of the Ice framework, with support for JavaScript and TypeScript.

## Sample Code

```slice
// Slice definitions (Greeter.ice)

#pragma once

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

```typescript
// Client application for Node.js (client.ts)

import { Ice } from "@zeroc/ice";
import { VisitorCenter } from "./Greeter.js";
import process from "node:process";

await using communicator = Ice.initialize(process.argv);

const greeter = new VisitorCenter.GreeterPrx(communicator, "greeter:tcp -h hello.zeroc.com -p 4061");

// Retrieve my name
const name = process.env.USER || process.env.USERNAME || "masked user";

const greeting = await greeter.greet(name);

console.log(greeting);
```

## Development

The JavaScript code uses `console.assert` statements for debugging, labeled `DEV:`. You should configure your production
builds to remove such statements. Different module bundlers provide different options, for example:

- If using [esbuild], see [drop-labels option][esbuild-drop-labels].
- If using rollup, see the [@rollup/plugin-strip] plugin and the [drop-labels option][strip-drop-labels].
- If using WebPack the [TerserWebpackPlugin] plugin can be configured to drop `console.assert`; see `drop_console` in
[terser options].

[Examples]: https://github.com/zeroc-ice/ice-demos/tree/main/js
[NPM Packages]: ../NIGHTLY.md#ice-for-javascript
[Documentation]: https://docs.zeroc.com/ice/latest/javascript/
[API Reference]: https://code.zeroc.com/ice/main/api/javascript/index.html
[Building from source]: ./BUILDING.md
[Ice framework]: https://github.com/zeroc-ice/ice
[esbuild]: https://esbuild.github.io/
[esbuild-drop-labels]: https://esbuild.github.io/api/#drop-labels
[@rollup/plugin-strip]: https://github.com/rollup/plugins/tree/master/packages/strip
[strip-drop-labels]: https://github.com/rollup/plugins/tree/master/packages/strip#labels
[TerserWebpackPlugin]: https://webpack.js.org/plugins/terser-webpack-plugin/
[terser options]: https://terser.org/docs/options/
