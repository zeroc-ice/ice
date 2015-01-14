# zeroc-slice2js
Compiles Slice files to Javascript.

## Install
```bash
$ npm install zeroc-slice2js --save-dev
```

_See below for command line usage._

## Usage
```js
var slice2js = require('zeroc-slice2js');
slice2js(["Hello.ice"]);
```

_The zeroc-slice2js module includes all of the Ice Slice definitions and automatically adds the slice directory to the include file search path._

## Options

### args `Array`

The list of arguments passed to slice2js

```bash
-h, --help              Show this message.
-v, --version           Display the Ice version.
-DNAME                  Define NAME as 1.
-DNAME=DEF              Define NAME as DEF.
-UNAME                  Remove any definition for NAME.
-IDIR                   Put DIR in the include file search path.
-E                      Print preprocessor output on stdout.
--stdout                Print genreated code to stdout.
--output-dir DIR        Create files in the directory DIR.
--depend                Generate Makefile dependencies.
--depend-json           Generate Makefile dependencies in JSON format.
-d, --debug             Print debug messages.
--ice                   Permit `Ice` prefix (for building Ice source code only).
--underscore            Permit underscores in Slice identifiers.
--icejs                 Build icejs module
```

Additional documentation can be found [here](https://doc.zeroc.com/display/Ice36/slice2js+Command-Line+Options).

## Command Line
Slice2js can also be installed globally and used from the command line.

```bash
$ npm install -g zeroc-slice2js
$ slice2js Hello.ice
```

## Gulp

For gulp integration refer to the [gulp-zeroc-slice2js package](https://github.com/ZeroC-Inc/gulp-zeroc-slice2js).
