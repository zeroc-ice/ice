# zeroc-icejs-demos
Demos for [Ice for JavaScript](https://github.com/ZeroC-Inc/zeroc-icejs)

## Requirements
Compiling and running the Ice for JavaScript demos requires the following:
- [Node.js](https://nodejs.org)
- Dependencies for [node-gyp](https://github.com/TooTallNate/node-gyp) (Required to build the slice2js compiler).

## Install
```bash
$ git clone https://github.com/ZeroC-Inc/icejs-demos icejs-demos
$ cd icejs-demos
$ npm install
```

## Running the scripts
```bash
$ npm run gulp:build // Build the demos
$ npm run gulp:watch // Run demo web server; watch for files changes and reload
$ npm run gulp:clean // Clean the demos
```

## Running the demos
Running a demo requires running a corresponding (C++, Java, C#) server. For more information refer to the [documentation](https://doc.zeroc.com/display/Ice36/Sample+Programs).
