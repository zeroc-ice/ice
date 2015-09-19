// **********************************************************************
//
// Copyright (c) 2003-2015 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

 /* global
    __runEchoServerOptions__ : false,
    __test__ : false,
    Test : false,
*/

var communicator = Ice.initialize();


$(document).foundation();
$(document).ready(
    function(){

        /* jshint browser:true, jquery:true */
        var TestCases = {
            "Ice/acm":
                {
                    files: ["Test.js", "Client.js"]
                },
            "Ice/ami":
                {
                    files: ["Test.js", "Client.js"]
                },
            "Ice/binding":
                {
                    files: ["Test.js", "Client.js"]
                },
            "Ice/defaultValue":
                {
                    files: ["Test.js", "Client.js"]
                },
            "Ice/enums":
                {
                    files: ["Test.js", "Client.js"],
                    configurations:
                    [
                        {
                            name: "1.0 encoding", args: ["--Ice.Default.EncodingVersion=1.0"]
                        },
                        {
                            name: "1.1 encoding"
                        },
                    ]
                },
            "Ice/exceptions":
                {
                    files: ["Test.js", "Client.js"],
                    configurations:
                    [
                        {
                            name: "compact (default) format"
                        },
                        {
                            name: "sliced format", args: ["--Ice.Default.SlicedFormat"]
                        },
                        {
                            name: "1.0 encoding", args: ["--Ice.Default.EncodingVersion=1.0"]
                        },
                        {
                            name: "compact (default) format and AMD server"
                        },
                        {
                            name: "sliced format and AMD server.", args: ["--Ice.Default.SlicedFormat"]
                        },
                        {
                            name: "1.0 encoding and AMD server", args: ["--Ice.Default.EncodingVersion=1.0"]
                        }
                    ]
                },
            "Ice/exceptionsBidir":
                {
                    files: ["Test.js", "TestAMD.js", "ThrowerI.js", "AMDThrowerI.js",
                            "../exceptions/Client.js", "Client.js"],
                },
            "Ice/facets":
                {
                    files: ["Test.js", "Client.js"]
                },
            "Ice/facetsBidir":
                {
                    files: ["Test.js", "TestI.js", "../facets/Client.js", "Client.js"]
                },
            "Ice/hold":
                {
                    files: ["Test.js", "Client.js"]
                },
            "Ice/info":
                {
                    files: ["Test.js", "Client.js"]
                },
            "Ice/inheritance":
                {
                    files: ["Test.js", "Client.js"]
                },
            "Ice/inheritanceBidir":
                {
                    files: ["Test.js", "InitialI.js", "../inheritance/Client.js", "Client.js"],
                },
            "Ice/operations":
                {
                    files: ["Test.js", "Twoways.js", "Oneways.js", "BatchOneways.js", "Client.js"],
                    configurations:
                    [
                        {
                            name: "regular server"
                        },
                        {
                            name: "AMD server"
                        },
                        {
                            name: "TIE server", langs: ["java", "csharp"]
                        },
                        {
                            name: "AMD TIE server", langs: ["java", "csharp"]
                        }
                    ]
                },
            "Ice/operationsBidir":
                {
                    files: ["Test.js", "TestAMD.js", "../operations/Twoways.js", "../operations/Oneways.js",
                            "../operations/BatchOneways.js", "MyDerivedClassI.js", "AMDMyDerivedClassI.js",
                            "../operations/Client.js", "Client.js"]
                },
            "Ice/objects":
                {
                    files: ["Test.js", "Client.js"],
                    configurations:
                    [
                        {
                            name: "compact (default) format"
                        },
                        {
                            name: "sliced format", args: ["--Ice.Default.SlicedFormat"]
                        },
                        {
                            name: "1.0 encoding", args: ["--Ice.Default.EncodingVersion=1.0"]
                        }
                    ]
                },
            "Ice/optional":
                {
                    files: ["Test.js", "Client.js"],
                    configurations:
                    [
                        {
                            name: "compact (default) format"
                        },
                        {
                            name: "sliced format", args: ["--Ice.Default.SlicedFormat"]
                        },
                        {
                            name: "AMD server"
                        }
                    ]
                },
            "Ice/optionalBidir":
                {
                    files: ["Test.js", "TestAMD.js", "InitialI.js", "AMDInitialI.js", "../optional/Client.js",
                            "Client.js"]
                },
            "Ice/promise":
                {
                    files: ["Client.js"]
                },
            "Ice/properties":
                {
                    files: ["Client.js"]
                },
            "Ice/proxy":
                {
                    files: ["Test.js", "Client.js"],
                    configurations:
                    [
                        {
                            name: "regular server"
                        },
                        {
                            name: "AMD server"
                        }
                    ]
                },
            "Ice/retry":
                {
                    files: ["Test.js", "Client.js"]
                },
            "Ice/slicing/exceptions":
                {
                    files: ["Test.js", "Client.js"],
                    configurations:
                    [
                        {
                            name: "sliced format"
                        },
                        {
                            name: "1.0 encoding", args: ["--Ice.Default.EncodingVersion=1.0"]
                        },
                        {
                            name: "sliced format and AMD server"
                        },
                        {
                            name: "1.0 encoding and AMD server", args: ["--Ice.Default.EncodingVersion=1.0"]
                        }
                    ]
                },
            "Ice/slicing/objects":
                {
                    files: ["Test.js", "Client.js"],
                    configurations:
                    [
                        {
                            name: "sliced format"
                        },
                        {
                            name: "1.0 encoding", args: ["--Ice.Default.EncodingVersion=1.0"]
                        },
                        {
                            name: "sliced format and AMD server"
                        },
                        {
                            name: "1.0 encoding and AMD server", args: ["--Ice.Default.EncodingVersion=1.0"]
                        }
                    ]
                },
            "Ice/timeout":
                {
                    files: ["Test.js", "Client.js"]
                },
            "Ice/number":
                {
                    files: ["Client.js"]
                },
        };

        var current, next;
        for(var testName in TestCases)
        {
            if(current)
            {
                next = testName;
                break;
            }

            if(document.location.pathname.indexOf(testName + "/index.html") !== -1)
            {
                current = testName;
            }
        }

        $("#console").height(120);
        $("#protocol").val(document.location.protocol == "https:" ? "wss" : "ws");
        for(var name in TestCases)
        {
            $("#test").append("<option value=\"/test/" + name + "/index.html\">" + name + "</option>");
        }
        $("#test").val("/test/" + current + "/index.html");

        var nextLanguage;
        $.ajax(
            {
                url: "/server-languages.json",
                dataType: "json"
            }
        ).done(
            function(data)
            {
                data.languages.forEach(
                    function(lang)
                    {
                        $("#language").append("<option value=\"" + lang.value + "\">" + lang.name + "</option>");
                    });

                nextLanguage = function(language)
                {
                    var i = 0;
                    for(; i < data.languages.length; ++i)
                    {
                        if(data.languages[i].value == language)
                        {
                            break;
                        }
                    }
                    return data.languages[i < data.languages.length - 1 ? i + 1 : 0].value;
                };
            });

        var out =
        {
            write: function(msg)
            {
                var text = $("#console").val();
                $("#console").val((text === "") ? msg : (text + msg));
            },
            writeLine: function(msg)
            {
                out.write(msg + "\n");
                $("#console").scrollTop($("#console").get(0).scrollHeight);
            }
        };

        var protocol;

        $("#run").click(function(){
            if(!$(this).hasClass("disabled"))
            {
                $("#console").val("");
                $(this).addClass("disabled");
                $("#test").prop("disabled", "disabled");
                $("#protocol").prop("disabled", "disabled");
                $("#language").prop("disabled", "disabled");
                var defaultHost = document.location.hostname || "127.0.0.1";

                protocol = $("#protocol").val();
                var id = new Ice.InitializationData();
                id.properties = Ice.createProperties();
                id.properties.setProperty("Ice.Default.Host", defaultHost);
                id.properties.setProperty("Ice.Default.Protocol", protocol);

                var language = $("#language").val();

                var str;
                if(protocol == "ws")
                {
                    str = "controller:ws -h " + defaultHost + " -p 15002";
                }
                else
                {
                    str = "controller:wss -h " + defaultHost + " -p 15003";
                }
                var controller = Test.Common.ControllerPrx.uncheckedCast(communicator.stringToProxy(str));

                var p;
                var server;
                var options = [];
                if(typeof(__runServer__) !== "undefined" || typeof(__runEchoServer__) !== "undefined")
                {
                    var srv;
                    if(typeof(__runEchoServer__) !== "undefined")
                    {
                        srv = "Ice/echo";
                        if(typeof(__runEchoServerOptions__) !== "undefined")
                        {
                            options = options.concat(__runEchoServerOptions__);
                        }

                    }
                    else
                    {
                        srv = current;
                    }
                    out.write("starting " + srv + " server... ");
                    p = controller.runServer(language, srv, protocol, defaultHost, false, options).then(
                        function(proxy)
                        {
                            var ref = proxy.ice_getIdentity().name + ":" + protocol + " -h " + defaultHost + " -p " +
                                (protocol == "ws" ? "15002" : "15003");
                            out.writeLine("ok");
                            server = Test.Common.ServerPrx.uncheckedCast(communicator.stringToProxy(ref));

                            var testCase = TestCases[current];
                            if(testCase.configurations === undefined)
                            {
                                return server.waitForServer().then(
                                    function()
                                    {
                                        return __test__(out, id);
                                    });
                            }
                            else
                            {
                                var prev = new Ice.Promise().succeed();
                                testCase.configurations.forEach(
                                    function(configuration)
                                    {
                                        if(configuration.langs && configuration.langs.indexOf(language) == -1)
                                        {
                                            return prev;
                                        }
                                        prev = prev.then(
                                            function()
                                            {
                                                out.writeLine("Running test with " + configuration.name + ".");
                                                return server.waitForServer().then(
                                                    function()
                                                    {
                                                        var initData = id.clone();
                                                        if(configuration.args !== undefined)
                                                        {
                                                            initData.properties = Ice.createProperties(configuration.args, id.properties);
                                                        }
                                                        return __test__(out, initData);
                                                    });
                                            });
                                    });
                                return prev;
                            }
                        },
                        function(ex)
                        {
                            out.writeLine("failed! (" + ex + ")");
                            throw ex;
                        }
                    ).then(
                        function()
                        {
                            if(server)
                            {
                                return server.waitTestSuccess();
                            }
                        }
                    ).exception(
                        function(ex)
                        {
                            if(server)
                            {
                                return server.terminate().then(
                                    function()
                                    {
                                        throw ex;
                                    },
                                    function()
                                    {
                                        throw ex;
                                    });
                            }
                            else
                            {
                                throw ex;
                            }
                        });
                }
                else
                {
                    p = __test__(out, id);
                }

                p.finally(
                    function()
                    {
                        $("#test").prop("disabled", false);
                        $("#protocol").prop("disabled", false);
                        $("#language").prop("disabled", false);
                        $("#run").removeClass("disabled");
                    }
                ).then(
                    function()
                    {
                        if($("#loop").is(":checked"))
                        {
                            var href = document.location.protocol + "//" + document.location.host;
                            if(!next)
                            {
                                next = "Ice/acm";
                                if(protocol == "ws")
                                {
                                    protocol = "wss";
                                    href = href.replace("http", "https");
                                    href = href.replace("8080", "9090");
                                }
                                else
                                {
                                    protocol = "ws";
                                    href = href.replace("https", "http");
                                    href = href.replace("9090", "8080");
                                    language = nextLanguage(language);
                                }
                            }

                            href += document.location.pathname.replace(current, next);
                            href += "?loop=true&language=" + language;
                            document.location.assign(href);
                        }
                    }
                ).exception(
                    function(ex, r)
                    {
                        out.writeLine("");
                        if(r instanceof Ice.AsyncResult)
                        {
                            out.writeLine("exception occurred in call to " + r.operation);
                        }
                        out.writeLine(ex.toString());
                        if(ex.stack)
                        {
                            out.writeLine(ex.stack);
                        }
                    });
            }
            return false;
        });

        //
        // Test case
        //
        $("#test").on("change",
            function(e)
            {
                document.location.assign($(this).val() + "?language=" + $("#language").val());
                return false;
            });

        $("#language").on("change",
            function(e)
            {
                document.location.assign(document.location.pathname + "?language=" + $("#language").val());
                return false;
            });

        //
        // Protocol
        //
        $("#protocol").on("change",
            function(e)
            {
                var newProtocol = $(this).val();
                if(protocol !== newProtocol)
                {
                    var href = document.location.protocol + "//" + document.location.host +
                        document.location.pathname;
                    if(newProtocol == "ws")
                    {
                        href = href.replace("https", "http");
                        href = href.replace("9090", "8080");
                    }
                    else
                    {
                        href = href.replace("http", "https");
                        href = href.replace("8080", "9090");
                    }
                    href += "?language=" + $("#language").val();
                    document.location.assign(href);
                }
            });


        $(this).addClass("disabled");
        $("#test").prop("disabled", "disabled");
        $("#protocol").prop("disabled", "disabled");
        $("#language").prop("disabled", "disabled");

        var scripts = TestCases[current].files.slice(0);
        var head = document.getElementsByTagName("head")[0];
        function loadScript()
        {
            if(scripts.length === 0)
            {
                $("#test").prop("disabled", false);
                $("#protocol").prop("disabled", false);
                $("#language").prop("disabled", false);
                $("#run").removeClass("disabled");

                //
                // Check if we should start the test loop=true
                //
                var href = document.location.href;
                var i = href.indexOf("?");

                var languageIdx = i !== -1 ? href.substr(i).indexOf("language=") : -1;
                if(languageIdx !== -1)
                {
                    $("#language").val(href.substr(i + languageIdx + 9));
                }
                else
                {
                    $("#language").val("cpp");
                }

                var autoStart = i !== -1 && href.substr(i).indexOf("loop=true") !== -1;
                if(autoStart)
                {
                    $("#loop").prop("checked", true);
                    $("#run").click();
                }
                return;
            }

            var script = document.createElement("script");
            script.type = "text/javascript";
            script.src = "/test/" + current + "/" + scripts.shift();

            var loaded = false;
            script.onload = script.onreadystatechange = function()
            {
                if(!loaded && (!this.readyState || this.readyState == "complete" || this.readyState == "loaded"))
                {
                    loaded = true;
                    loadScript();
                }
            };

            head.appendChild(script);
        }
        loadScript();
    });
