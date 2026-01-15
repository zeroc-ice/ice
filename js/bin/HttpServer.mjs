//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

import crypto from "crypto";
import fs from "fs";
import http from "http";
import https from "https";
import path from "path";
import url from "url";
import hogan from "hogan.js";
import { fileURLToPath } from "url";
import { createRequire } from "module";

const __filename = fileURLToPath(import.meta.url);
const __dirname = path.dirname(__filename);
const require = createRequire(import.meta.url);

function Init() {
    const MimeTypes = {
        css: "text/css",
        html: "text/html",
        ico: "image/x-icon",
        js: "text/javascript",
        json: "application/json",
    };

    const TestData = {
        cssDeps: [
            "https://cdnjs.cloudflare.com/ajax/libs/foundation/5.5.3/css/foundation.min.css"
        ],
        jsDeps: [
            "https://cdnjs.cloudflare.com/ajax/libs/jquery/2.2.4/jquery.min.js",
            "https://cdnjs.cloudflare.com/ajax/libs/modernizr/2.8.3/modernizr.min.js",
            "https://cdnjs.cloudflare.com/ajax/libs/foundation/5.5.3/js/foundation.min.js",
            "https://cdnjs.cloudflare.com/ajax/libs/spin.js/2.3.2/spin.min.js",
            "https://cdnjs.cloudflare.com/ajax/libs/URI.js/1.18.12/URI.min.js",
            "/assets/jquery.spin.js"
        ]
    };

    const libraries = [
        "/lib/Ice.js", "/lib/Ice.min.js",
        "/lib/Glacier2.js", "/lib/Glacier2.min.js",
        "/lib/IceStorm.js", "/lib/IceStorm.min.js",
        "/lib/IceGrid.js", "/lib/IceGrid.min.js"
    ];

    const commonPath = path.join(__dirname, "..", "test", "Common");
    TestData.TestSuites = fs.readFileSync(path.join(commonPath, "TestSuites.json"), "utf8");
    const TestSuites = JSON.parse(TestData.TestSuites);
    TestData.tests = Object.keys(TestSuites);
    const controller = hogan.compile(fs.readFileSync(path.join(commonPath, "controller.html"), "utf8"));
    const libraryMaps = libraries.map(f => f + ".map");

    class HttpServer {
        constructor() {
            this._basePath = path.resolve(path.join(__dirname, ".."));
        }

        processRequest(req, res) {
            const matchController = req.url.pathname.match("^\/test/(.*)/controller\\.html");
            if (matchController) {
                const testpath = path.resolve(path.join(this._basePath, "test", matchController[1]));
                const worker = req.url.query.worker == "True";
                const scripts = [
                    "/lib/Ice.js",
                    "/test/Common/TestHelper.js",
                    "/test/Common/Controller.js",
                    "/test/Common/ControllerI.js",
                ];

                const testSuite = TestSuites[matchController[1]];
                if (testSuite) {
                    TestData.scripts = scripts.concat(testSuite.files.map(f => {
                        if (f.indexOf("/") === -1) {
                            return "/test/" + matchController[1] + "/" + f;
                        } else {
                            return f;
                        }
                    }));
                } else {
                    TestData.scripts = scripts.concat(
                        fs.readdirSync(testpath).filter(f => path.extname(f) === ".js")
                    );
                }

                if (worker) {
                    TestData.workerScripts = TestData.scripts;
                }

                res.writeHead(200, { "Content-Type": "text/html" });
                res.end(controller.render(TestData));
                console.log("HTTP/200 (Ok) " + req.method + " " + req.url.pathname);
            } else if (req.url.pathname === '/start') {
                res.writeHead(302, { "Location": "/test/Ice/acm/controller.html&port=15002" });
                res.end();
            } else {
                const iceLib = libraries.indexOf(req.url.pathname) !== -1;
                const iceLibMap = libraryMaps.indexOf(req.url.pathname) !== -1;

                const useBinDist = (process.env.ICE_BIN_DIST || "").split(" ")
                    .find(dist => dist == "js" || dist == "all") !== undefined;

                let basePath;
                if (useBinDist && (iceLib || iceLibMap)) {
                    basePath = path.resolve(path.join(require.resolve("ice"), "..", ".."));
                } else {
                    basePath = this._basePath;
                }

                let filePath = req.url.pathname;
                if (filePath.indexOf("..") != -1 ||
                    !["/test/", "/assets/", "/lib", "/node_modules"].some(prefix => filePath.startsWith(prefix))) {
                    res.writeHead(403);
                    res.end("403 Forbidden");
                    console.log("HTTP/403 (Forbidden) " + req.method + " " + req.url.pathname + " -> " + filePath);
                    return;
                }

                let sourceMap;
                filePath = path.resolve(path.join(basePath, filePath));
                if (iceLib) {
                    sourceMap = req.url.pathname.replace(".js", ".js.map");
                }

                // If OPTIMIZE is set resolve Ice libraries to the corresponding minified versions.
                if (process.env.OPTIMIZE == "yes") {
                    if (iceLib && !filePath.endsWith(".min.js")) {
                        filePath = filePath.replace(".js", ".min.js");
                    } else if (iceLibMap && !filePath.endsWith(".min.js.map")) {
                        filePath = filePath.replace(".js.map", ".min.js.map");
                    }
                }

                const ext = path.extname(filePath).slice(1);

                const doRequest = (err, stats, requestPath, srcMap) => {
                    if (err) {
                        if (err.code === "ENOENT") {
                            res.writeHead(404);
                            res.end("404 Page Not Found");
                            console.log("HTTP/404 (Page Not Found) " + req.method + " " + req.url.pathname + " -> " + requestPath);
                        } else {
                            res.writeHead(500);
                            res.end("500 Internal Server Error");
                            console.log("HTTP/500 (Internal Server Error) " + req.method + " " + req.url.pathname + " -> " + requestPath);
                        }
                    } else {
                        if (!stats.isFile()) {
                            res.writeHead(403);
                            res.end("403 Forbidden");
                            console.log("HTTP/403 (Forbidden) " + req.method + " " + req.url.pathname + " -> " + requestPath);
                        } else {
                            // Create a md5 using the stats attributes to be used as Etag header.
                            const hash = crypto.createHash("md5");
                            hash.update(stats.ino.toString());
                            hash.update(stats.mtime.toString());
                            hash.update(stats.size.toString());

                            const headers = {
                                "Content-Type": MimeTypes[ext] || "text/plain",
                                "Content-Length": stats.size,
                                "Etag": hash.digest("hex")
                            };

                            if (srcMap) {
                                headers["X-SourceMap"] = srcMap;
                            }

                            if (path.extname(requestPath).slice(1) == "gz") {
                                headers["Content-Encoding"] = "gzip";
                            }

                            // Check for conditional request header if-none-match.
                            let modified = true;
                            if (req.headers["if-none-match"] !== undefined) {
                                modified = req.headers["if-none-match"].split(" ")
                                    .every(element => element !== headers.Etag);
                            }

                            if (!modified) {
                                res.writeHead(304, headers);
                                res.end();
                                console.log("HTTP/304 (Not Modified) " + req.method + " " + req.url.pathname + " -> " + requestPath);
                            } else {
                                res.writeHead(200, headers);
                                if (req.method === "HEAD") {
                                    res.end();
                                } else {
                                    fs.createReadStream(requestPath, { "bufferSize": 4 * 1024 }).pipe(res);
                                }
                                console.log("HTTP/200 (Ok) " + req.method + " " + req.url.pathname + " -> " + requestPath);
                            }
                        }
                    }
                };

                if (req.url.pathname === '/') {
                    res.writeHead(302, { "Location": "/test/Ice/acm/index.html" });
                    res.end();
                    console.log("HTTP/302 (Found) " + req.method + " " + req.url.pathname + " -> /test/Ice/acm/index.html");
                } else if ((ext == "js" || ext == "css" || ext == "map") &&
                           req.headers["accept-encoding"] &&
                           req.headers["accept-encoding"].indexOf("gzip") !== -1) {
                    // When the browser asks for a .js or .css file and it has support for gzip content
                    // check if a gzip version (.js.gz or .css.gz) of the file exists and use that instead.
                    fs.stat(filePath + ".gz", (err, stats) => {
                        if (err || !stats.isFile()) {
                            fs.stat(filePath, (err, stats) => {
                                doRequest(err, stats, filePath, sourceMap);
                            });
                        } else {
                            doRequest(err, stats, filePath + ".gz", sourceMap);
                        }
                    });
                } else {
                    fs.stat(filePath, (err, stats) => {
                        doRequest(err, stats, filePath, sourceMap);
                    });
                }
            }
        }

        start() {
            const httpServer = http.createServer();
            const httpsServer = https.createServer({
                passphrase: "password",
                pfx: fs.readFileSync(path.join("..", "certs", "server.p12")),
            });

            const createServerHandler = (server, port, host) => {
                server.on("request", (req, res) => {
                    // Dummy data callback required so request end event is emitted.
                    req.on("data", () => {});
                    req.on("end", () => {
                        req.url = url.parse(req.url, true);
                        req.url.protocol = port == 8080 ? "http" : "https";
                        this.processRequest(req, res);
                    });
                });
                server.listen(port, host);
            };

            createServerHandler(httpServer, 8080, "0.0.0.0");
            createServerHandler(httpsServer, 9090, "0.0.0.0");
            console.log("listening on ports 8080 (http) and 9090 (https)...");
        }
    }

    new HttpServer().start();
}

export default Init;
