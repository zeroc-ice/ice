# slice2js

Compiles Slice files to JavaScript.

## Install

```bash
npm install slice2js --save-dev
```

## Usage

```js
var slice2js = require('slice2js');
```

### Methods

#### `slice2js.compile(args [, options])`

* Returns a [ChildProcess](https://nodejs.org/api/child_process.html#child_process_class_childprocess) object.

##### args `Array`

List of arguments passed to the `slice2js` compiler.

| Option              | Description                                                  |
| ------------------- | ------------------------------------------------------------ |
| -h                  | Show usage information.                                      |
| -v, --version       | Display the Ice version.                                     |
| -DNAME              | Define NAME as 1.                                            |
| -DNAME=DEF          | Define NAME as DEF.                                          |
| -UNAME              | Remove any definition for NAME.                              |
| -IDIR               | Add DIR to the include path for Slice files.                 |
| -E                  | Print preprocessor output on stdout.                         |
| --stdout            | Print generated code to stdout.                              |
| --output-dir DIR    | Create files in the directory DIR.                           |
| --depend            | Print dependency information for Slice files.                |
| --depend-json       | Print dependency information for Slice files in JSON format. |
| --depend-xml        | Print dependency information for Slice files in XML format.  |
| --depend-file FILE  | Write dependencies to FILE instead of standard output.       |
| -d, --debug         | Print debug messages.                                        |

Additional documentation can be found [here](https://archive.zeroc.com/ice/3.7/language-mappings/javascript-mapping/client-side-slice-to-javascript-mapping/slice2js-command-line-options).

The `slice2js` module includes all the Ice Slice files and adds these files to the include file search path.

##### options `Object`

Object `options` is passed directly to [child_process.spawn(command[, args][, options])](https://nodejs.org/api/child_process.html#child_process_child_process_spawn_command_args_options) as the options parameter. This can be used to control things such as stdio, environment, and working directory.

```js
var slice2js = require('slice2js');
slice2js.compile(['Hello.ice'], { stdio: 'inherit' });
```

#### `slice2js.sliceDir`

Returns the absolute path of the included Ice Slice files.

### Example

```js
var slice2js = require('slice2js');
slice2js.compile(['Hello.ice']).on('close', function(code) {
    if (code !== 0) {
        console.log('slice2js exited with code ' + code);
    } else {
        console.log('slice2js finished successfully');
    }
});
```

### Command Line

slice2js can also be installed globally and used from the command line.

```bash
npm install -g slice2js
slice2js Hello.ice
```

## Gulp

For gulp integration refer to the [gulp-ice-builder package](https://github.com/zeroc-ice/gulp-ice-builder).
