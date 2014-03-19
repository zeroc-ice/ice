// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

var communicator = Ice.initialize();

$(document).foundation();
    $(document).ready(
        function(){
            $("#console").height(120);
            $("#protocol").val(document.location.protocol == "https:" ? "wss" : "ws");
            for(var name in TestCases)
            {
                $("#test").append("<option value=\"" + basePath + name + "/index.html\">" + name + "</option>");
            }
            $("#test").val(basePath + current + "/index.html");
            

            var out = 
            {
                write: function(msg)
                {
                    var text = $("#console").val();
                    $("#console").val((text == "") ? msg : (text + msg));
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
                    var defaultHost = document.location.hostname || "127.0.0.1";;
                    
                    protocol = $("#protocol").val();
                    var id = new Ice.InitializationData();
                    id.properties = Ice.createProperties();
                    id.properties.setProperty("Ice.Default.Host", defaultHost);
                    id.properties.setProperty("Ice.Default.Protocol", protocol);

                    var str;
                    if(protocol == "ws")
                    {
                        str = "controller:ws -h " + defaultHost + " -p 12009";
                    }
                    else
                    {
                        str = "controller:wss -h " + defaultHost + " -p 12008";
                    }
                    var controller = Test.ControllerPrx.uncheckedCast(communicator.stringToProxy(str));
                    
                    var p;
                    var server;
                    if(typeof(__runServer__) !== "undefined" || typeof(__runEchoServer__) !== "undefined")
                    {
                        if(typeof(__runEchoServer__) !== "undefined")
                        {
                            srv = "Ice/echo"
                        }
                        else
                        {
                            srv = current
                        }
                        out.write("starting " + srv + " server... ");
                        p = controller.runServer("cpp", srv, protocol, defaultHost).then(
                            function(proxy)
                            {
                                var ref = proxy.ice_getIdentity().name + ":" + protocol + " -h " + defaultHost + 
                                            " -p " + (protocol == "ws" ? "12009" : "12008");
                                out.writeLine("ok");
                                server = Test.ServerPrx.uncheckedCast(communicator.stringToProxy(ref));
                                return __test__(out, id);
                            },
                            function(ex)
                            {
                                out.writeLine("failed! (" + ex.ice_name() + ")");
                                return __test__(out, id);
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
                            }
                        );
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
                            $("#run").removeClass("disabled");
                        }
                    ).then(
                        function()
                        {
                            if($("#loop").is(":checked"))
                            {
                                var location = document.location;
                                var href = location.protocol + "//" + location.hostname;
                                if(protocol == "wss")
                                {
                                    href += ":9090";
                                }
                                else
                                {
                                    href += ":8080";
                                }
                                href += location.pathname.replace(current, next) + "?loop=true";
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
            
            (function(){
                
                if(basePath == "../../../")
                {
                    $(".title-area a").attr("href", "../../../../index.html");
                    $(".breadcrumbs li:first a").attr("href", "../../../../index.html");
                }
                
                //
                // Check if we should start the test loop=true
                //
                var href = document.location.href;
                var i = href.indexOf("?");
                var autoStart = i !== -1 && href.substr(i).indexOf("loop=true") !== -1;
                if(autoStart)
                {
                    $("#loop").prop("checked", true);
                    $("#run").click();
                }
            }());
            
            //
            // Test case
            //
            $("#test").on("change",
                          function(e)
                          {
                              document.location.assign($(this).val());
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
                                    var href = document.location.protocol + "//" + document.location.host + document.location.pathname;
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
                                    document.location.assign(href);
                                }
                            });
        });
