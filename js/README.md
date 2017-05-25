# Internet Communications Engine (Ice)
Ice is a comprehensive RPC framework with support for C++, .NET, Java, Python, JavaScript and more.

## Install
Using either npm or yarn:

```bash
$ npm install ice --save
$ yarn add ice
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

The npm package also includes the browser version of Ice for JavaScript. Add the necessary `<script>` tags to your html to include the Ice for JavaScript components you require. `node_modules` must server out from the root directory of your web server.

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

Minified versions are available with the `.min.js` extension.

## Documentation

See the [Ice Documentation](https://doc.zeroc.com/display/Ice37/JavaScript+Mapping).

## Slice2js Compiler

To compile [Slice](https://doc.zeroc.com/display/Ice37/The+Slice+Language) files to JavaScript see the following:
- [slice2js](https://github.com/zeroc-ice/npm-slice2js)
- [gulp-ice-builder](https://github.com/zeroc-ice/gulp-ice-builder)

## Demos

A collection of demos for Ice for JavaScript (and other language mappings) can be found [here](https://github.com/zeroc-ice/ice-demos).
