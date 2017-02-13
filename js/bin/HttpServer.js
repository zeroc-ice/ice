// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

var crypto    = require("crypto");
var fs        = require("fs");
var http      = require("http");
var https     = require("https");
var path      = require("path");
var url       = require("url");
var hogan     = require("hogan.js");

function Init()
{
    var MimeTypes =
    {
        css: "text/css",
        html: "text/html",
        ico: "image/x-icon",
        js: "text/javascript",
        json: "application/json",
    };

    var TestData =
        {
            languages: [{value: "cpp", name: "C++"}, {value: "java", name: "Java"}]
        };
    if(process.platform == "win32")
    {
        TestData.languages.push({value: "csharp", name: "C#"});
    }

    var libraries = ["/lib/Ice.js", "/lib/Ice.min.js",
                    "/lib/Glacier2.js", "/lib/Glacier2.min.js",
                    "/lib/IceStorm.js", "/lib/IceStorm.min.js",
                    "/lib/IceGrid.js", "/lib/IceGrid.min.js",];

    TestData.TestCases = fs.readFileSync(path.join(__dirname, "..", "test", "Common", "TestCases.json"), "utf8");
    var TestCases = JSON.parse(TestData.TestCases);
    TestData.tests = Object.keys(TestCases);
    var template = hogan.compile(fs.readFileSync(path.join(__dirname, "..", "test", "Common", "index.html"), "utf8"));
    var libraryMaps = libraries.map(
        function(f)
        {
            return f + ".map";
        });

    var HttpServer = function(host, ports)
    {
        this._basePath = path.resolve(path.join(__dirname, ".."));
    };

    HttpServer.prototype.processRequest = function(req, res)
    {
        var match = req.url.pathname.match("^\/test/(.*)/index\.html");
        if(match)
        {
            // That is a test case
            var testCase = TestCases[match[1]];
            if(testCase === undefined)
            {
                res.writeHead(404);
                res.end("404 Page Not Found");
                console.log("HTTP/404 (Page Not Found)" + req.method + " " + req.url.pathname);
            }
            else
            {
                TestData.current = match[1];
                if(req.url.query.next == "true")
                {
                    var testCase = TestData.tests[0];
                    var language = req.url.query.language !== undefined ? req.url.query.language : "cpp";
                    var protocol = req.url.protocol;
                    var i = TestData.tests.indexOf(TestData.current);
                    var worker = req.url.query.worker == "true";

                    if(i < TestData.tests.length - 1)
                    {
                        testCase = TestData.tests[i + 1];
                    }
                    else if(!worker)
                    {
                        worker = true;
                    }
                    else if(protocol == "http")
                    {
                        protocol = "https";
                    }
                    else
                    {
                        var languages = TestData.languages.map(function(o) { return o.value; });
                        var j = languages.indexOf(language);
                        language = languages[j == languages.length - 1 ? 0 : j + 1];
                        worker = false;
                        protocol = "http";
                    }

                    var location = url.format(
                        {
                            protocol: protocol,
                            hostname: req.headers.host.split(":")[0],
                            port: (protocol == "http" ? 8080 : 9090),
                            pathname: ("/test/" + testCase + "/index.html"),
                            query:{loop: "true", language: language, worker: worker}
                        });

                    res.writeHead(302, {"Location": location});
                    res.end();
                    console.log("HTTP/302 (Redierct) -> " + location);
                }
                else
                {
                    if(req.url.query.worker != "true")
                    {
                        TestData.scripts =
                            [
                                "/lib/Ice.js",
                                "/test/Common/TestRunner.js",
                                "/test/Common/TestSuite.js",
                                "/test/Common/Controller.js"
                            ].concat(testCase.files);
                    }
                    else
                    {
                        TestData.scripts =
                            [
                                "/test/Common/TestSuite.js"
                            ];
                    }
                    res.writeHead(200, {"Content-Type": "text/html"});
                    res.end(template.render(TestData));
                    console.log("HTTP/200 (Ok)" + req.method + " " + req.url.pathname);
                }
            }
        }
        else
        {
            var iceLib = libraries.indexOf(req.url.pathname) !== -1;
            var iceLibMap = libraryMaps.indexOf(req.url.pathname) !== -1;

            var basePath = (process.env.USE_BIN_DIST == "yes" && (iceLib || iceLibMap)) ?
                path.resolve(path.join(require.resolve("ice"), "..", "..")) : this._basePath;

            var filePath = path.resolve(path.join(basePath, req.url.pathname));

            //
            // If OPTIMIZE is set resolve Ice libraries to the corresponding minified
            // versions.
            //
            if(process.env.OPTIMIZE == "yes")
            {
                if(iceLib && filePath.substr(-7) !== ".min.js")
                {
                    filePath = filePath.replace(".js", ".min.js");
                }
                else if(iceLibMap && filePath.substr(-11) !== ".min.js.map")
                {
                    filePath = filePath.replace(".js.map", ".min.js.map");
                }
            }

            var ext = path.extname(filePath).slice(1);

            //
            // When the browser ask for a .js or .css file and it has support for gzip content
            // check if a gzip version (.js.gz or .css.gz) of the file exists and use that instead.
            //
            if((ext == "js" || ext == "css" || ext == "map") && req.headers["accept-encoding"].indexOf("gzip") !== -1)
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
                    if(req.url.pathname === '/')
                    {
                        res.writeHead(302,
                        {
                            "Location": "/test/Ice/acm/index.html"
                        });
                        res.end();
                        console.log("HTTP/302 (Found) " + req.method + " " + req.url.pathname + " -> " +
                                    "/test/Ice/acm/index.html");
                    }
                    else if(!stats.isFile())
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
                            "Etag": hash.digest("hex")
                        };

                        if(path.extname(filePath).slice(1) == "gz")
                        {
                            headers["Content-Encoding"] = "gzip";
                        }

                        //
                        // Check for conditional request header if-none-match.
                        //
                        var modified = true;
                        if(req.headers["if-none-match"] !== undefined)
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
        }
    };

    HttpServer.prototype.start = function()
    {
        var httpServer = http.createServer();
        var httpsServer = https.createServer({
            passphrase: "password",
            pfx: fs.readFileSync(path.join("..", "certs", "server.p12")),
        });

        var self = this;

        function createServerHandler(server, port, host)
        {
            server.on("request",
                      function(req, res)
                      {
                          //
                          // Dummy data callback required so request end event is emitted.
                          //
                          var dataCB = function(data)
                          {
                          };
                          var endCB = function()
                          {
                              req.url = url.parse(req.url, true);
                              req.url.protocol = port == 8080 ? "http" : "https";
                              self.processRequest(req, res);
                          };
                          req.on("data", dataCB);
                          req.on("end", endCB);
                      });
            server.listen(port, host);
        }

        createServerHandler(httpServer, 8080, "0.0.0.0");
        createServerHandler(httpsServer, 9090, "0.0.0.0");
        console.log("listening on ports 8080 (http) and 9090 (https)...");
    };

    new HttpServer().start();
}

module.exports = Init;
