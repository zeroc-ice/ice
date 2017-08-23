# Ice for JavaScript
Ice is a comprehensive RPC framework with support for JavaScript, C++, .NET, Java, Python and more.

## Install
Using npm:

```bash
npm install ice --save
```

## Usage

```js
var Ice      = require('ice').Ice;
var Glacier2 = require('ice').Glacier2;
var IceStorm = require('ice').IceStorm;
var IceGrid  = require('ice').IceGrid;

var communicator = Ice.initialize(process.argv);
var proxy = communicator.stringToProxy("hello:tcp -h localhost -p 10000");
```

The npm package also includes the browser version of Ice for JavaScript. For example:

```html
<script src="/node_modules/ice/lib/Ice.js"></script>
<script src="/node_modules/ice/lib/Glacier2.js"></script>
<script src="/node_modules/ice/lib/IceStorm.js"></script>
<script src="/node_modules/ice/lib/IceGrid.js"></script>
<script type="text/javascript">
    var communicator = Ice.initialize();
    var proxy = communicator.stringToProxy("hello:ws -h localhost -p 10002");
</script>
```

For older browsers that do not support all of the required EcmaScript 6 features
used by Ice for JavaScript, we provide a pre-compiled version of the libraries
using the Babel JavaScript compiler. These libraries depend on the
[babel-polyfill](https://www.npmjs.com/package/babel-polyfill) run time and are
available in the `es5` subdirectory with the same names as the main libraries:

```html
<script src="/node_modules/babel-polyfill/dist/polyfill.min.js"></script>
<script src="/node_modules/ice/lib/es5/Ice.js"></script>
<script src="/node_modules/ice/lib/es5/Glacier2.js"></script>
<script src="/node_modules/ice/lib/es5/IceStorm.js"></script>
<script src="/node_modules/ice/lib/es5/IceGrid.js"></script>
<script type="text/javascript">
    var communicator = Ice.initialize();
    var proxy = communicator.stringToProxy("hello:ws -h localhost -p 10002");
</script>
```

Minified versions are available with the `.min.js` extension.

## Documentation

See the [Ice Manual](https://doc.zeroc.com/display/Ice37/Ice+Manual).

## Compiling Slice Files to JavaScript

You can use `slice2js` directly or through the Ice Builder for Gulp to compile Slice files to JavaScript:
- [slice2js](https://github.com/zeroc-ice/npm-slice2js)
- [gulp-ice-builder](https://github.com/zeroc-ice/gulp-ice-builder)

## Demos

The [ice-demos](https://github.com/zeroc-ice/ice-demos) repository provides a collection of demos for
JavaScript and other programming languages.
