// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

 /* global
    __runEchoServerOptions__ : false,
    __test__ : false,
    Test : false,
    URI : false,
    current : false,
    TestCases : false,
    runTest: false
*/

$(document).foundation();
$(document).ready(
    function(){
        $("#console").height(120);

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

        window.onerror = function(msg, url, line, column, err)
        {
            var e = msg + " at " + url + ":" + line + ":" + column;
            if(err)
            {
                e += "\n" + err.stack;
            }
            out.writeLine(e);
            return false;
        };

        var query = new URI(document.location.href).search(true);

        $("#language").val(query.language !== undefined ? query.language : "cpp");
        $("#protocol").val(document.location.protocol == "http:" ? "ws" : "wss");
        $("#test").val("/test/" + current + "/index.html");
        $("#worker").prop("checked", query.worker == "true");
        $("#loop").prop("checked", query.loop == "true");

        function nextTest()
        {
            document.location.assign(new URI()
                .host(document.location.host)
                .pathname($("#test").val())
                .search(
                    {
                        language: $("#language").val(),
                        worker: $("#worker").is(":checked"),
                        loop: $("#loop").is(":checked"),
                        next:"true"
                    }).toString());
        }

        function next(success)
        {
            if($("#loop").is(":checked"))
            {
                if(success)
                {
                    nextTest(success);
                }
            }
            else if(query.loop == "true")
            {
                updateLocation();
            }
        }

        function setRunning(running)
        {
            if(running)
            {
                $("#console").val("");
                $("#run").addClass("disabled");
                $("#test").prop("disabled", "disabled");
                $("#protocol").prop("disabled", "disabled");
                $("#language").prop("disabled", "disabled");
                $("#worker").prop("disabled", "disabled");
            }
            else
            {
                $("#test").prop("disabled", false);
                $("#protocol").prop("disabled", false);
                $("#language").prop("disabled", false);
                $("#worker").prop("disabled", false);
                $("#run").removeClass("disabled");
            }
        }

        function updateLocation()
        {
            document.location.assign(new URI()
                .host(document.location.host)
                .pathname($("#test").val())
                .search(
                    {
                        language: $("#language").val(),
                        worker: $("#worker").is(":checked"),
                        loop: $("#loop").is(":checked")
                    }).toString());
        }

        $("#run").click(function(){
            if(!$(this).hasClass("disabled"))
            {
                setRunning(true);
                if($("#worker").is(":checked"))
                {
                    var worker = new Worker("/test/Common/Worker.js");
                    worker.onmessage = function(e)
                    {
                        if(e.data.type == "Write")
                        {
                            out.write(e.data.message);
                        }
                        else if(e.data.type == "WriteLine")
                        {
                            out.writeLine(e.data.message);
                        }
                        else if(e.data.type == "TestFinished")
                        {
                            worker.terminate();
                            setRunning(false);
                            next(e.data.success);
                        }
                    };
                    worker.postMessage(
                        {
                            type: "RunTest",
                            test:
                            {
                                name: current,
                                language: $("#language").val(),
                                defaultHost: document.location.hostname || "127.0.0.1",
                                protocol: $("#protocol").val(),
                                configurations: TestCases[current].configurations,
                                files: TestCases[current].files
                            }
                        });

                    worker.onerror = function(e)
                    {
                        console.log(e);
                    };
                }
                else
                {
                    runTest(current, $("#language").val(), document.location.hostname || "127.0.0.1",
                            $("#protocol").val(), TestCases[current].configurations, out
                    ).finally(
                        function()
                        {
                            setRunning(false);
                        }
                    ).then(
                        function(success)
                        {
                            next(success);
                        });
                }
            }
            return false;
        });

        $("#test").on("change",
                      function(e)
                      {
                          updateLocation();
                          return false;
                      });

        $("#language").on("change",
                          function(e)
                          {
                              updateLocation();
                              return false;
                          });

        $("#worker").on("change",
                          function(e)
                          {
                              updateLocation();
                              return false;
                          });

        $("#protocol").on("change",
                          function(e)
                          {
                              if((document.location.protocol == "http:" && $(this).val() == "wss") ||
                                 (document.location.protocol == "https:" && $(this).val() == "ws"))
                              {
                                  document.location.assign(
                                      new URI()
                                        .protocol($(this).val() == "ws" ? "http" : "https")
                                        .hostname(document.location.hostname)
                                        .port($(this).val() == "ws" ? 8080 : 9090)
                                        .search(
                                            {
                                                language: $("#language").val(),
                                                worker: $("#worker").is(":checked")
                                            }));
                                  return false;
                              }
                          });

        if($("#loop").is(":checked"))
        {
            $("#loop").prop("checked", true);
            $("#run").click();
        }
    });
