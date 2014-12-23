// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

var http = require("http");
var https = require("https");
var url = require("url");
var crypto = require("crypto");
var fs = require("fs");
var path = require("path");

var httpProxy = null;
try
{
    httpProxy = require("http-proxy");
}
catch(e)
{
    console.warn("Warning: couldn't find http-proxy module, it's necessary to run the demos,\n" +
                 "you can run the following command to install it: npm install http-proxy\n");
}

var MimeTypes =
{
    css: "text/css",
    html: "text/html",
    ico: "image/x-icon",
    jpeg: "image/jpeg",
    jpg: "image/jpeg",
    js: "text/javascript",
    png: "image/png",
};

var iceHome = process.env.ICE_HOME;
var iceLibDir;
var useBinDist = process.env.USE_BIN_DIST && process.env.USE_BIN_DIST == "yes";
var srcDist;
try
{
    srcDist = fs.statSync(path.join(__dirname, "..", "lib")).isDirectory();
}
catch(e)
{
}

var iceJs = process.env.OPTIMIZE == "yes" ? "Ice.min.js" : "Ice.js";

//
// If this is a source distribution and ICE_HOME isn't set ensure
// that Ice libraries has been build.
//
if(srcDist && !iceHome && !useBinDist)
{
    var build;
    try
    {
        iceLibDir = path.join(__dirname, "..", "lib");
        build = fs.statSync(path.join(__dirname, "..", "lib", iceJs)).isFile();
    }
    catch(e)
    {
    }

    if(!build)
    {
        console.error("error: unable to find `" + iceJs + "' in `" + path.join(__dirname, "..", "lib") + "',\n" + 
                      "please verify that the sources have been built or configure ICE_HOME\n" +
                      "to use a binary distribution");
        process.exit(1);
    }
}

//
// If this is a demo distribution ensure that ICE_HOME is set or install in a default location.
//
if(!srcDist || useBinDist)
{
    //
    // if ICE_HOME is not set check if it is install in the default location.
    //
    if(!process.env.ICE_HOME)
    {
        var dist = "Ice-3.6b";
        [
            "C:\\Program Files\\ZeroC",
            "C:\\Program Files (x86)\\ZeroC",
            "/Library/Developer",
            "/opt",
            "/usr"
        ].some(
            function(basePath)
            {
                try
                {
                    if(fs.statSync(path.join(basePath, dist, "lib", iceJs)).isFile())
                    {
                        iceHome = path.join(basePath, dist);
                        iceLibDir = path.join(basePath, dist, "lib");
                        return true;
                    }
                }
                catch(e)
                {
                }
                
                try
                {
                    if(fs.statSync(path.join(basePath, "share", "javascript", "ice-3.6b", iceJs)).isFile())
                    {
                        iceHome = path.join(basePath);
                        iceLibDir = path.join(basePath, "share", "javascript", "ice-3.6b"); 
                        return true;
                    }
                }
                catch(e)
                {
                }
                return false;
            });
    }

    if(!iceHome)
    {
        console.error("error Ice for JavaScript not found in the default installation directories\n" +
                      "ICE_HOME environment variable must be set, and point to the Ice installation directory.");
        process.exit(1);
    }
}

//
// If ICE_HOME is set ensure that Ice libraries exists in that location.
//
if(iceHome)
{
    var iceHomeValid;
    try
    {
        iceLibDir = path.join(iceHome, "lib"); 
        iceHomeValid = fs.statSync(path.join(iceLibDir, iceJs)).isFile();
    }
    catch(e)
    {
    }
    
    if(!iceHomeValid)
    {
        try
        {
            iceLibDir = path.join(iceHome, "share", "javascript", "ice-3.6b"); 
            iceHomeValid = fs.statSync(path.join(iceLibDir, iceJs)).isFile();
        }
        catch(e)
        {
        }
    }

    if(!iceHomeValid)
    {
        console.error("error: unable to find `" + iceJs + "' in `" + path.join(iceHome, "lib") + "',\n" + 
                      "please verify ICE_HOME is properly configured and Ice is correctly insetalled");
        process.exit(1);
    }
    console.log("Using Ice libraries from " + iceLibDir);
}

var libraries = ["/lib/Ice.js", "/lib/Ice.min.js",
                 "/lib/Glacier2.js", "/lib/Glacier2.min.js",
                 "/lib/IceStorm.js", "/lib/IceStorm.min.js",
                 "/lib/IceGrid.js", "/lib/IceGrid.min.js",];

var HttpServer = function(host, ports)
{
    this._host = host;
    this._ports = ports;
    this._basePath = path.resolve(path.join(__dirname, ".."));
};

HttpServer.prototype.processRequest = function(req, res)
{
    var filePath;

    var iceLib = libraries.indexOf(req.url.pathname) !== -1;
    //
    // If ICE_HOME has been set resolve Ice libraries paths into ICE_HOME.
    //
    if(iceHome && iceLib)
    {
        filePath = path.join(iceLibDir, req.url.pathname.substr(4));
    }
    else
    {
        filePath = path.resolve(path.join(this._basePath, req.url.pathname));
    }

    //
    // If OPTIMIZE is set resolve Ice libraries to the corresponding minified
    // versions.
    //
    if(process.env.OPTIMIZE == "yes" && iceLib && filePath.substr(-7) !== ".min.js")
    {
        filePath = filePath.replace(".js", ".min.js");
    }

    var ext = path.extname(filePath).slice(1);

    //
    // When the browser ask for a .js or .css file and it has support for gzip content
    // check if a gzip version (.js.gz or .css.gz) of the file exists and use that instead.
    //
    if((ext == "js" || ext == "css") && req.headers["accept-encoding"].indexOf("gzip") !== -1)
    {
        fs.stat(filePath + ".gz",
                function(err, stats)
                {
                    if(err || !stats.isFile())
                    {
                        fs.stat(filePath,
                                function(err, stats)
                                {
                                    doRequest(err, stats, filePath);
                                });
                    }
                    else
                    {
                        doRequest(err, stats, filePath + ".gz");
                    }
                });
    }
    else
    {
        fs.stat(filePath,
                    function(err, stats)
                    {
                        doRequest(err, stats, filePath);
                    });
    }

    var doRequest = function(err, stats, filePath)
    {
        if(err)
        {
            if(err.code === "ENOENT")
            {
                res.writeHead(404);
                res.end("404 Page Not Found");
                console.log("HTTP/404 (Page Not Found)" + req.method + " " + req.url.pathname + " -> " + filePath);
            }
            else
            {
                res.writeHead(500);
                res.end("500 Internal Server Error");
                console.log("HTTP/500 (Internal Server Error) " + req.method + " " + req.url.pathname + " -> " +
                            filePath);
            }
        }
        else
        {
            if(!stats.isFile())
            {
                res.writeHead(403);
                res.end("403 Forbiden");
                console.log("HTTP/403 (Forbiden) " + req.method + " " + req.url.pathname + " -> " + filePath);
            }
            else
            {
                //
                // Create a md5 using the stats attributes
                // to be used as Etag header.
                //
                var hash = crypto.createHash("md5");
                hash.update(stats.ino.toString());
                hash.update(stats.mtime.toString());
                hash.update(stats.size.toString());

                var headers =
                {
                    "Content-Type": MimeTypes[ext] || "text/plain",
                    "Content-Length": stats.size,
                    "Last-Modified": new Date(stats.mtime).toUTCString(),
                    "Etag": hash.digest("hex")
                };

                if(path.extname(filePath).slice(1) == "gz")
                {
                    headers["Content-Encoding"] = "gzip";
                }

                //
                // Check for conditional request headers, if-modified-since
                // and if-none-match.
                //
                var modified = true;
                if(Date.parse(req.headers["if-modified-since"]) == stats.mtime.getTime())
                {
                    modified = false;
                }
                else if(req.headers["if-none-match"] !== undefined)
                {
                    modified = req.headers["if-none-match"].split(" ").every(
                        function(element, index, array)
                        {
                            return element !== headers.Etag;
                        });
                }

                //
                // Not Modified
                //
                if(!modified)
                {
                    res.writeHead(304, headers);
                    res.end();
                    console.log("HTTP/304 (Not Modified) " + req.method + " " + req.url.pathname + " -> " + filePath);
                }
                else
                {
                    res.writeHead(200, headers);
                    if(req.method === "HEAD")
                    {
                        res.end();
                    }
                    else
                    {
                        fs.createReadStream(filePath, { "bufferSize": 4 * 1024 }).pipe(res);
                    }
                    console.log("HTTP/200 (Ok) " + req.method + " " + req.url.pathname + " -> " + filePath);
                }
            }
        }
    };
};

//
// Proxy configuration for the different demos.
//
var proxyConfig = [
    {resource: "/demows", target: "http://localhost:10002", protocol: "ws"},
    {resource: "/demowss", target: "https://localhost:10003", protocol: "wss"},
    {resource: "/chatws", target: "http://localhost:5063", protocol: "ws"},
    {resource: "/chatwss", target: "https://localhost:5064", protocol: "wss"}
];

var proxies = {};

HttpServer.prototype.start = function()
{
    var baseDir;
    if(!["../../certs", "../certs"].some(
        function(p)
        {
            return fs.existsSync(baseDir = path.join(__dirname, p));
        }))
    {
        console.error("Cannot find wss certificates directory");
        process.exit(1);
    }
    var options = {
        passphrase: "password",
        key: fs.readFileSync(path.join(baseDir, "s_rsa1024_priv.pem")),
        cert: fs.readFileSync(path.join(baseDir, "s_rsa1024_pub.pem"))
    };

    var httpServer = http.createServer();
    var httpsServer = https.createServer(options);

    if(httpProxy)
    {
        proxyConfig.forEach(
            function(conf)
            {
                proxies[conf.resource] = {
                    server: httpProxy.createProxyServer({target : conf.target, secure : false}),
                    protocol: conf.protocol };
            });
    }

    var self = this;
    [httpServer, httpsServer].forEach(function(server)
                    {
                        server.on("request", function(req, res)
                                  {
                                        //
                                        // Dummy data callback required so request end event is emitted.
                                        //
                                        var dataCB = function(data)
                                        {
                                        };

                                        var endCB = function()
                                        {
                                            req.url = url.parse(req.url);
                                            self.processRequest(req, res);
                                        };

                                        req.on("data", dataCB);
                                        req.on("end", endCB);
                                  });
                    });

    if(httpProxy)
    {
        var requestCB = function(protocols)
        {
            return function(req, socket, head)
            {
                var errCB = function(err)
                {
                    socket.end();
                };
                var proxy = proxies[req.url];
                if(proxy && protocols.indexOf(proxy.protocol) !== -1)
                {
                    proxy.server.ws(req, socket, head, errCB);
                }
                else
                {
                    socket.end();
                }
            };
        };

        httpServer.on("upgrade", requestCB(["ws"]));
        httpsServer.on("upgrade", requestCB(["ws", "wss"]));
    }

    httpServer.listen(8080, this._host);
    httpsServer.listen(9090, this._host);
    console.log("listening on ports 8080 (http) and 9090 (https)...");
};

var server = new HttpServer("0.0.0.0", [8080, 9090]);
server.start();

