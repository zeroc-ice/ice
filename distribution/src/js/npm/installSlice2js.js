// **********************************************************************
// Copyright (c) 2003-2015 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in th
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

var execFile  = require('child_process').execFile;
var fs        = require('fs');
var http      = require('http');
var os        = require('os');
var path      = require('path');
var url       = require('url');
var util      = require('util');
var zlib      = require('zlib');
var platform  = os.platform();
var arch      = os.arch();

var BIN_DIR   = path.join(__dirname, 'bin');
var slice2js  = platform === 'win32' ? 'slice2js.exe' : 'slice2js';
slice2js      = path.join(BIN_DIR, slice2js);

var pkgVer    = require('./package.json').slice2js;

function downloadSlice2js(redirectUrl)
{
  if(fs.existsSync(slice2js))
  {
    fs.unlinkSync(slice2js);
  }

  var SLICE2JS_URL = redirectUrl ||
                     process.env.SLICE2JS_URL ||
                     process.env.npm_config_SLICE2JS_URL ||
                     'http://zeroc.com/download/3.6/';

  var dlPath =
  {
    'darwin' :
    {
      'x64' : util.format('slice2js-%s-osx.gz', pkgVer)
    },
    'linux':
    {
      'x64' : util.format('slice2js-%s-linux-%s.gz', pkgVer, arch),
      'x86' : util.format('slice2js-%s-linux-%s.gz', pkgVer, arch)
    },
    'win32':
    {
      'x64' : util.format('slice2js-%s-win-%s.exe.gz', pkgVer, arch),
      'x86' : util.format('slice2js-%s-win-%s.exe.gz', pkgVer, arch)
    }
  };

  var slice2js_url = url.resolve(SLICE2JS_URL, dlPath[platform][arch]);
  console.log('Downloading slice2js from: ' + slice2js_url);
  var req = http.get(slice2js_url,
    function(res)
    {
      if(res.statusCode !== 200)
      {
        if(res.statusCode === 302 && !redirectUrl)
        {
          return downloadSlice2js(res.headers.location);
        }
        else if (res.statusCode === 404)
        {
          console.log('Unable to find slice2js at %s. Proceeding without it.', SLICE2JS_URL);
          process.exit(0);
        }
        else
        {
          consle.log('There was an error downloading slice2js.');
          process.exit(0);
        }
      }

      var progress = 0;
      var dlStream = fs.createWriteStream(slice2js + '.gz');
      res.pipe(dlStream);

      res.on('end',
        function()
        {
          console.log('Slice2js downloaded.');
          dlStream.end();

          var zipSteam = fs.createReadStream(slice2js+'.gz');
          var fileStream = fs.createWriteStream(slice2js, {flags : 'w', mode: 33261});
          zipSteam.pipe(zlib.createGunzip()).pipe(fileStream);
          fileStream.on('close',
            function()
            {
              fs.unlinkSync(slice2js+'.gz');
            });
        });
    });

  req.on('error',
    function(err)
    {
      console.log('There was an error retrieving slice2js. ' + err.message);
    });
}

if(fs.existsSync(slice2js) === true)
{

  execFile(slice2js, ['--version'],
    function(err, stdout, stderr)
    {
      if (err)
      {
        return downloadSlice2js();
      }

      var version = stdout.trim() || stderr.trim();
      if(version === pkgVer)
      {
        return;
      }
      else
      {
        downloadSlice2js();
      }
    });
}
else
{
  downloadSlice2js();
}

module.exports.path = slice2js;
module.exports.version = pkgVer;
