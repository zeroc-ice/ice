// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

(function(){

var Promise = Ice.Promise;
var RouterPrx = Glacier2.RouterPrx;
var ChatSessionPrx = Demo.ChatSessionPrx;
var ChatCallbackPrx = Demo.ChatCallbackPrx;

//
// Servant that implements the ChatCallback interface.
// The message operation just writes the received data
// to the output textarea.
//
var ChatCallbackI = Ice.Class(Demo.ChatCallback, {
    message: function(data)
    {
        $("#output").val($("#output").val() + data + "\n");
        $("#output").scrollTop($("#output").get(0).scrollHeight);
    }
});

//
// Chat client state
//
var State = {
    Disconnected: 0,
    Connecting: 1,
    Connected:2
};

var state = State.Disconnected;
var hasError = false;

var signin = function()
{
    var communicator;
    var router;
    Promise.try(
        function()
        {
            state = State.Connecting;
            //
            // Dismiss any previous error message.
            //
            if(hasError)
            {
                dismissError();
            }
            //
            // Transition to loading screen
            //
            return transition("#signin-form", "#loading");
        }
    ).then(
        function()
        {
            //
            // Start animating the loading progress bar.
            //
            startProgress();

            var hostname = document.location.hostname || "127.0.0.1";
            //
            // If the demo is accessed vi https, use a secure (WSS) endpoint, otherwise
            // use a non-secure (WS) endpoint.
            //
            // The web server will act as a reverse proxy for WebSocket connections. This
            // facilitates the setup of WSS with self-signed certificates because Firefox
            // and Internet Explorer certificate exceptions are only valid for the same
            // port and host.
            //
            var secure = document.location.protocol.indexOf("https") != -1;
            var router = secure ? "DemoGlacier2/router:wss -p 9090 -h " + hostname + " -r /chatwss" :
                                  "DemoGlacier2/router:ws -p 8080 -h " + hostname + " -r /chatws";

            //
            // Initialize the communicator with the Ice.Default.Router property
            // set to the chat demo Glacier2 router.
            //
            var id = new Ice.InitializationData();
            id.properties = Ice.createProperties();
            id.properties.setProperty("Ice.Default.Router", router);
            communicator = Ice.initialize(id);

            //
            // Get a proxy to the Glacier2 router using checkedCast to ensure
            // the Glacier2 server is available.
            //
            return RouterPrx.checkedCast(communicator.getDefaultRouter());
        }
    ).then(
        function(r)
        {
            router = r;

            //
            // Create a session with the Glacier2 router.
            //
            return router.createSession($("#username").val(), $("#password").val());
        }
    ).then(
        function(session)
        {
            run(communicator, router, ChatSessionPrx.uncheckedCast(session));
        }
    ).exception(
        function(ex)
        {
            //
            // Handle any exceptions that occurred during session creation.
            //
            if(ex instanceof Glacier2.PermissionDeniedException)
            {
                error("permission denied:\n" + ex.reason);
            }
            else if(ex instanceof Glacier2.CannotCreateSessionException)
            {
                error("cannot create session:\n" + ex.reason);
            }
            else if(ex instanceof Ice.ConnectFailedException)
            {
                error("connection to server failed");
            }
            else
            {
                error(ex.toString());
            }

            if(communicator)
            {
                communicator.destroy();
            }
        });
};

var run = function(communicator, router, session)
{
    //
    // The chat promise is used to wait for the completion of chatting
    // state. The completion could happen because the user signed out,
    // or because an exception was raised.
    //
    var chat = new Promise();

    //
    // Get the session timeout and the router client category, and
    // create the client object adapter.
    //
    // Use Ice.Promise.all to wait for the completion of all the
    // calls.
    //
    Promise.all(
        router.getSessionTimeout(),
        router.getCategoryForClient(),
        communicator.createObjectAdapterWithRouter("", router)
    ).then(
        function(timeoutArgs, categoryArgs, adapterArgs)
        {
            var timeout = timeoutArgs[0];
            var category = categoryArgs[0];
            var adapter = adapterArgs[0];

            //
            // Call refreshSession in a loop to keep the
            // session alive.
            //
            var refreshSession = function()
            {
                router.refreshSession().exception(
                    function(ex)
                    {
                        chat.fail(ex);
                    }
                ).delay(timeout.toNumber() * 500).then(
                    function()
                    {
                        if(!chat.completed())
                        {
                            refreshSession();
                        }
                    });
            };
            refreshSession();

            //
            // Create the ChatCallback servant and add it to the
            // ObjectAdapter.
            //
            var callback = ChatCallbackPrx.uncheckedCast(adapter.add(new ChatCallbackI(),
                                                                     new Ice.Identity("callback", category)));

            //
            // Set the chat session callback.
            //
            return session.setCallback(callback);
        }
    ).then(
        function()
        {
            //
            // Stop animating the loading progress bar and
            // transition to the chat screen.
            //
            stopProgress(true);
            return transition("#loading", "#chat-form");
        }
    ).then(
        function()
        {
            $("#loading .meter").css("width", "0%");
            state = State.Connected;
            $("#input").focus();

            //
            // Process input events in the input textbox until the chat
            // promise is completed.
            //
            $("#input").keypress(
                function(e)
                {
                    if(!chat.completed())
                    {
                        //
                        // When the enter key is pressed, we send a new
                        // message using the session say operation and
                        // reset the textbox contents.
                        //
                        if(e.which === 13)
                        {
                            var msg = $(this).val();
                            $(this).val("");
                            session.say(msg).exception(
                                function(ex)
                                {
                                    chat.fail(ex);
                                });
                            return false;
                        }
                    }
                });

            //
            // Exit the chat loop by accepting the chat
            // promise.
            //
            $("#signout").click(
                function()
                {
                    chat.succeed();
                    return false;
                }
            );

            return chat;
        }
    ).finally(
        function()
        {
            //
            // Reset the input text box and chat output
            // textarea.
            //
            $("#input").val("");
            $("#input").off("keypress");
            $("#signout").off("click");
            $("#output").val("");

            //
            // Destroy the session.
            //
            return router.destroySession();
        }
    ).then(
        function()
        {
            //
            // Destroy the communicator and go back to the
            // disconnected state.
            //
            communicator.destroy().finally(
                function()
                {
                    transition("#chat-form", "#signin-form").finally(
                        function()
                        {
                            $("#username").focus();
                            state = State.Disconnected;
                        });
                });
        }
    ).exception(
        function(ex)
        {
            //
            // Handle any exceptions that occurred while running.
            //
            error(ex);
            communicator.destroy();
        });
};

//
// Switch to Disconnected state and display the error
// message.
//
var error = function(message)
{
    stopProgress(false);
    hasError = true;
    var current = state === State.Connecting ? "#loading" : "#chat-form";
    $("#signin-alert span").text(message);

    //
    // Transition the screen
    //
    transition(current, "#signin-alert").then(
        function()
        {
            $("#loading .meter").css("width", "0%");
            $("#signin-form").css("display", "block").animo({ animation: "flipInX", keep: true });
            state = State.Disconnected;
        }
    );
};

//
// Do a transition from "from" screen to "to" screen, return
// a promise that allows us to wait for the transition
// to complete. If to screen is undefined just animate out the
// from screen.
//
var transition = function(from, to)
{
    var p = new Ice.Promise();

    $(from).animo({ animation: "flipOutX", keep: true },
        function()
        {
            $(from).css("display", "none");
            if(to)
            {
                $(to).css("display", "block").animo({ animation: "flipInX", keep: true },
                                                    function()
                                                    {
                                                        p.succeed();
                                                    });
            }
            else
            {
                p.succeed();
            }
        });
    return p;
};

//
// Event handler for Sign in button
//
$("#signin").click(function()
                   {
                       signin();
                       return false;
                   });

//
// Dismiss error message.
//
function dismissError()
{
    transition("#signin-alert");
    hasError = false;
    return false;
};

//
// Animate the loading progress bar.
//
var w = 0;
var progress;

var startProgress = function()
{
    if(!progress)
    {
        progress = setInterval(
            function()
            {
                w = w === 100 ? 0 : w + 5;
                $("#loading .meter").css("width", w.toString() + "%");
            },
            20);
    }
};

var stopProgress = function(completed)
{
    if(progress)
    {
        clearInterval(progress);
        progress = null;
        if(completed)
        {
            $("#loading .meter").css("width", "100%");
        }
    }
};

$("#username").focus();

}());
