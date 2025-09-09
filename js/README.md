# Ice For JavaScript

[Examples] | [NPM packages] | [Documentation] | [API Reference] | [Building from source]

The [Ice framework] provides everything you need to build networked applications,
including RPC, pub/sub, server deployment, and more.

Ice for JavaScript is the JavaScript implementation of the Ice framework.

## Sample Code

```slice
// Slice definitions (Hello.ice)

module Demo
{
    interface Hello
    {
        void sayHello();
    }
}
```

```javascript
// Client application (client.js)
let communicator;
try
{
    communicator = Ice.initialize(process.argv);
    const hello = await Demo.HelloPrx.checkedCast(
        communicator.stringToProxy("hello:tcp -h localhost -p 10000"));
    await hello.sayHello();
}
catch(ex)
{
    console.log(ex.toString());
}
finally
{
   if(communicator)
   {
      await communicator.destroy();
   }
}
```

## Development

The JavaScript code uses `console.assert` statements for debugging, labeled `DEV:`. You should configure your production
builds to remove such statements. Different module bundlers provide different options, for example:

- If using [esbuild], see [drop-labels option][esbuild-drop-labels].
- If using rollup, see the [@rollup/plugin-strip] plugin and the [drop-labels option][strip-drop-labels].
- If using WebPack the [TerserWebpackPlugin] plugin can be configured to drop `console.assert`; see `drop_console` in [terser options].

[Examples]: https://github.com/zeroc-ice/ice-demos/tree/main/js
[NPM Packages]: https://www.npmjs.com/~zeroc
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
