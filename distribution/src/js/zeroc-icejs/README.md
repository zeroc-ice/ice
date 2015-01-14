# zeroc-icejs
Ice for JavaScript runtime

## Install

You can install Ice for JavaScript with either `npm` or `bower`. 

### npm
```bash
$ npm install zeroc-icejs --save
```

This module also includes the browser version of Ice for JavaScript.

### bower

```bash
$ bower install zeroc-icejs --save
```

## Usage

### npm

```js
var Ice      = require('zeroc-icejs').Ice;
var Glacier2 = require('zeroc-icejs').Glacier2;
var IceStorm = require('zeroc-icejs').IceStorm;
var IceGrid  = require('zeroc-icejs').IceGrid;

var communicator = Ice.initialize(process.argv);
var proxy = communicator.stringToProxy("hello:tcp -h localhost -p 10000");
```

The npm package also includes the browser version of Ice for JavaScript. Refer to the `bower` documentation below, replacing `bower_components` with `node_modules`.

### bower

Add the necessary `<script>` tags to your html to include the Ice for JavaScript components you require.

```html
<script src="/bower_components/zeroc-icejs/lib/Ice.js"></script>
<script src="/bower_components/zeroc-icejs/lib/Glacier2.js"></script>
<script src="/bower_components/zeroc-icejs/lib/IceStorm.js"></script>
<script src="/bower_components/zeroc-icejs/lib/IceGrid.js"></script>
<script type="text/javascript">
    var communicator = Ice.initialize();
    var proxy = communicator.stringToProxy("hello:ws -h localhost -p 10002");
</script>
```

Minified versions are available with the `.min.js` extension.

## Documentation

See the [Ice Documentation](https://doc.zeroc.com/display/Ice36/JavaScript+Mapping).

## Slice2js Compiler

To compile [Slice](https://doc.zeroc.com/display/Ice36/The+Slice+Language) files to JavaScript see the following:
- [zeroc-slice2js](https://github.com/ZeroC-Inc/zeroc-slice2js)
- [gulp-zeroc-slice2js](https://github.com/ZeroC-Inc/gulp-zeroc-slice2js)

## Demos

A collection of demos for Ice for JavaScript can be found [here](https://github.com/ZeroC-Inc/icejs-demos).
