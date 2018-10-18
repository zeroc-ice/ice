//
// Base on https://github.com/plgregoire/gulp-typescript-formatter/blob/master/index.js
//

var gutil       = require("gulp-util");
var PluginError = gutil.PluginError;
var PLUGIN_NAME = "ts-formatter";
var through     = require("through2");
var formatter   = require('typescript-formatter');

function format(options)
{
    return through.obj((file, enc, cb) =>
                       {

                           if(file.isNull())
                           {
                               return cb(null, file);
                           }

                           if(file.isBuffer())
                           {
                               formatter.processString(file.path, String(file.contents), options).then(
                                   result =>
                                       {
                                           file.contents = new Buffer(result.dest);
                                           cb(null, file);
                                       });
                           }

                           if(file.isStream())
                           {
                               return cb(new PluginError('ts-formatter', 'Streaming not supported'));
                           }
                       });

}

module.exports = format;
