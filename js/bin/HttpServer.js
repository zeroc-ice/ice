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

    var TestData = {
        cssDeps: [
            "https://cdnjs.cloudflare.com/ajax/libs/foundation/5.3.3/css/foundation.min.css",
            "https://cdnjs.cloudflare.com/ajax/libs/animo.js/1.0.3/animate-animo.min.css"
        ],
        jsDeps: [
            "https://cdnjs.cloudflare.com/ajax/libs/jquery/2.2.4/jquery.min.js",
            "https://cdnjs.cloudflare.com/ajax/libs/modernizr/2.8.3/modernizr.min.js",
            "https://cdnjs.cloudflare.com/ajax/libs/foundation/5.3.3/js/foundation.min.js",
            "https://cdnjs.cloudflare.com/ajax/libs/spin.js/2.3.2/spin.min.js",
            "https://cdnjs.cloudflare.com/ajax/libs/URI.js/1.18.10/URI.min.js",
            "https://cdnjs.cloudflare.com/ajax/libs/animo.js/1.0.3/animo.min.js",
            "/assets/jquery.spin.js"
        ]
    };

    var languages = [{value: "cpp", name: "C++"}, {value: "java", name: "Java"}]
    if(process.platform == "win32")
    {
        languages.push({value: "csharp", name: "C#"});
    }

    var libraries = ["/lib/Ice.js", "/lib/Ice.min.js",
                     "/lib/Glacier2.js", "/lib/Glacier2.min.js",
                     "/lib/IceStorm.js", "/lib/IceStorm.min.js",
                     "/lib/IceGrid.js", "/lib/IceGrid.min.js",
                     "/lib/es5/Ice.js", "/lib/es5/Ice.min.js",
                     "/lib/es5/Glacier2.js", "/lib/es5/Glacier2.min.js",
                     "/lib/es5/IceStorm.js", "/lib/es5/IceStorm.min.js",
                     "/lib/es5/IceGrid.js", "/lib/es5/IceGrid.min.js"];

    TestData.TestSuites = fs.readFileSync(path.join(__dirname, "..", "test", "Common", "TestSuites.json"), "utf8");
    var TestSuites = JSON.parse(TestData.TestSuites);
    TestData.tests = Object.keys(TestSuites);
    var template = hogan.compile(fs.readFileSync(path.join(__dirname, "..", "test", "Common", "index.html"), "utf8"));
    var controller = hogan.compile(fs.readFileSync(path.join(__dirname, "..", "test", "Common", "controller.html"), "utf8"));
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
        var matchController = req.url.pathname.match("^\/test/(.*)/controller\.html");
        if(match)
        {
            var es5 = match[1].indexOf("es5/") !== -1;
            var m = es5 ? match[1].replace("es5/", "") : match[1];

            // That is a test case
            var testSuite = TestSuites[m];
            if(testSuite === undefined)
            {
                res.writeHead(404);
                res.end("404 Page Not Found");
                console.log("HTTP/404 (Page Not Found)" + req.method + " " + req.url.pathname);
            }
            else
            {
                TestData.current = m;
                if(req.url.query.next == "true")
                {
                    var testSuite = TestData.tests[0];
                    var language = req.url.query.language !== undefined ? req.url.query.language : "cpp";
                    var protocol = req.url.protocol;
                    var i = TestData.tests.indexOf(TestData.current);
                    var worker = req.url.query.worker == "true";

                    if(i < TestData.tests.length - 1)
                    {
                        testSuite = TestData.tests[i + 1];
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
                        var lgs = languages.map(function(o) { return o.value; });
                        var j = lgs.indexOf(language);
                        language = lgs[j == lgs.length - 1 ? 0 : j + 1];
                        worker = false;
                        protocol = "http";
                    }

                    if(es5)
                    {
                        testSuite = "es5/" + testSuite;
                    }

                    var location = url.format(
                        {
                            protocol: protocol,
                            hostname: req.headers.host.split(":")[0],
                            port: (protocol == "http" ? 8080 : 9090),
                            pathname: ("/test/" + testSuite + "/index.html"),
                            query:{loop: "true", language: language, worker: worker}
                        });

                    res.writeHead(302, {"Location": location});
                    res.end();
                    console.log("HTTP/302 (Redirect) -> " + location);
                }
                else
                {
                    if(req.url.query.worker != "true")
                    {
                        TestData.scripts = [];

                        if(es5)
                        {
                            TestData.scripts =
                            [
                                "/node_modules/babel-polyfill/dist/polyfill.js",
                                "/lib/es5/Ice.js",
                                "/test/Common/TestRunner.js",
                                "/test/Common/TestSuite.js",
                                "/test/es5/Common/Controller.js"
                            ].concat(testSuite.files);

                            TestData.scripts = TestData.scripts.map(function(f)
                                {
                                    return f.replace("/lib/Glacier2.js", "/lib/es5/Glacier2.js");
                                });
                        }
                        else
                        {
                            TestData.scripts =
                                [
                                    "/lib/Ice.js",
                                    "/test/Common/TestRunner.js",
                                    "/test/Common/TestSuite.js",
                                    "/test/Common/Controller.js"
                                ].concat(testSuite.files);
                        }
                    }
                    else
                    {
                        TestData.scripts =
                            [
                                "/test/Common/TestSuite.js"
                            ];
                    }
                    TestData.languages = languages.slice();
                    if(testSuite.files.indexOf("Server.js") >= 0)
                    {
                        TestData.languages.push({value: "js", name: "JavaScript"});
                    }
                    res.writeHead(200, {"Content-Type": "text/html"});
                    res.end(template.render(TestData));
                    console.log("HTTP/200 (Ok)" + req.method + " " + req.url.pathname);
                }
            }
        }
        else if(matchController)
        {
            var es5 = matchController[1].indexOf("es5/") !== -1;
            var m = es5 ? matchController[1].replace("es5/", "") : matchController[1];
            var testpath = path.resolve(path.join(this._basePath, "test", matchController[1]))
            var scripts = es5 ? [
                "/node_modules/babel-polyfill/dist/polyfill.js",
                "/lib/es5/Ice.js",
                "/test/es5/Common/Controller.js",
                "/test/es5/Common/ControllerI.js",
            ] : [
                "/lib/Ice.js",
                "/test/Common/Controller.js",
                "/test/Common/ControllerI.js",
            ];
            var testSuite = TestSuites[m];
            if(testSuite)
            {
                TestData.scripts = scripts.concat(TestSuites[m].files.map(function(f) {
                    if(f.indexOf("/") === -1)
                    {
                        return "/test/" + matchController[1] + "/" + f;
                    }
                    else if(f.indexOf("/lib") === 0 && es5)
                    {
                        return f.replace("/lib", "/lib/es5");
                    }
                    else
                    {
                        return f;
                    }
                }))
            }
            else
            {
                TestData.scripts = scripts.concat(fs.readdirSync(testpath).filter(function(f) { return path.extname(f) === ".js"; }))
            }
            res.writeHead(200, {"Content-Type": "text/html"});
            res.end(controller.render(TestData))
            console.log("HTTP/200 (Ok) " + req.method + " " + req.url.pathname);
        }
        else
        {
            var iceLib = libraries.indexOf(req.url.pathname) !== -1;
            var iceLibMap = libraryMaps.indexOf(req.url.pathname) !== -1;

            var basePath;
            function checkIceBinDist(dist) {
                return dist == "js" || dist == "all"
            }
            var useBinDist = (process.env.ICE_BIN_DIST || "").split(" ").find(checkIceBinDist) !== undefined;
            if(useBinDist && (iceLib || iceLibMap))
            {
                basePath = path.resolve(path.join(require.resolve("ice"), "..", ".."));
            }
            else
            {
                basePath = this._basePath;
            }

            var filePath = req.url.pathname;
            var sourceMap;
            if(filePath.indexOf("es5/") !== -1 && path.extname(filePath) != ".js")
            {
                // We only host JS files in the es5 subdirectory, other files
                // (such as config/escapes.cfg are in test)
                filePath = filePath.replace("es5/", "")
            }
            filePath = path.resolve(path.join(basePath, filePath))
            if(iceLib)
            {
                sourceMap = req.url.pathname.replace(".js", ".js.map");
            }
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
            // When the browser asks for a .js or .css file and it has support for gzip content
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
                                            doRequest(err, stats, filePath, sourceMap);
                                        });
                            }
                            else
                            {
                                doRequest(err, stats, filePath + ".gz", sourceMap);
                            }
                        });
            }
            else
            {
                fs.stat(filePath,
                            function(err, stats)
                            {
                                doRequest(err, stats, filePath, sourceMap);
                            });
            }

            var doRequest = function(err, stats, filePath, sourceMap)
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

                        if(sourceMap)
                        {
                            headers["X-SourceMap"] = sourceMap;
                        }

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
