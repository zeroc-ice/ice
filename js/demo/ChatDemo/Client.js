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
var ChatRoomCallbackPrx = Chat.ChatRoomCallbackPrx;
var ChatSessionPrx = Chat.ChatSessionPrx;

//
// Chat client state
//
var State = {Disconnected: 0, Connecting: 1, Connected: 2};
var maxMessageSize = 1024;
var communicator;
var username;
var state;
var hasError = false;

var hostname = document.location.hostname || "127.0.0.1";

//
// Servant that implements the ChatCallback interface.
// The message operation just writes the received data
// to the output textarea.
//
var ChatCallbackI = Ice.Class(Chat.ChatRoomCallback, {
    init: function(users)
    {
        users.forEach(
            function(name)
            {
                userJoined(name);
            });
    },
    send: function(timestamp, name, message)
    {
        if(name != username)
        {
            writeLine(formatDate(timestamp.toNumber()) + " - <" + name + "> - " + unescapeHtml(message));
        }
    },
    join: function(timestamp, name)
    {
        writeLine(formatDate(timestamp.toNumber()) + " - <system-message> - " + name + " joined.");
        userJoined(name);
    },
    leave: function(timestamp, name)
    {
        writeLine(formatDate(timestamp.toNumber()) + " - <system-message> - " + name + " left.");
        userLeft(name);
    }
});

function userJoined(name)
{
    if(name == username)
    {
        $("#users").append("<li id=\"" + name + "\"><b>" + name + "</b></li>");
    }
    else
    {
        $("#users").append("<li id=\"" + name + "\"><a href=\"#\">" + name + "</a></li>");
        $("#users #" + name).click(
            function()
            {
                var s = $("#input").val();
                if(s.length > 0)
                {
                    s += " ";
                }
                s += "@" + name + " ";
                $("#input").val(s);
                $("#input").focus();
                return false;
            });
    }
    $("#users").append("<li class=\"divider\"></li>");
}

function userLeft(name)
{
    $("#users #" + name).off("click");
    $("#users #" + name).next().remove();
    $("#users #" + name).remove();
}

var signin = function()
{
    assert(state === State.Disconnected);
    setState(State.Connecting).then(
        function()
        {
            //
            // Initialize the communicator with the Ice.Default.Router property
            // set to the chat demo Glacier2 router.
            //
            var id = new Ice.InitializationData();
            id.properties = Ice.createProperties();
            id.properties.setProperty("Ice.Default.Router",
                                      "Glacier2/router:wss -p 9090 -h " + hostname + " -r /chatwss");
            communicator = Ice.initialize(id);

            //
            // Get a proxy to the Glacier2 router using checkedCast to ensure
            // the Glacier2 server is available.
            //
            return RouterPrx.checkedCast(communicator.getDefaultRouter()).then(
                function(router)
                {
                    //
                    // Create a session with the Glacier2 router.
                    //
                    return router.createSession(
                        $("#username").val(), $("#password").val()).then(
                            function(session)
                            {
                                run(router, ChatSessionPrx.uncheckedCast(session));
                            });
                });
        }
    ).exception(
        function(ex)
        {
            //
            // Handle any exceptions that occurred during session creation.
            //
            if(ex instanceof Glacier2.PermissionDeniedException)
            {
                setState(State.Disconnected, "permission denied:\n" + ex.reason);
            }
            else if(ex instanceof Glacier2.CannotCreateSessionException)
            {
                setState(State.Disconnected, "cannot create session:\n" + ex.reason);
            }
            else if(ex instanceof Ice.ConnectFailedException)
            {
                setState(State.Disconnected, "connection to server failed");
            }
            else
            {
                setState(State.Disconnected, ex.toString());
            }
        });
};

var run = function(router, session)
{
    assert(state === State.Connecting);
    //
    // The chat promise is used to wait for the completion of chatting
    // state. The completion could happen because the user signed out,
    // or because there is an exception.
    //
    var chat = new Promise();
    //
    // Get the session timeout and the router client category, then
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
        function()
        {
            //
            // The results of each promise are provided in an array.
            //
            var timeout = arguments[0][0];
            var category = arguments[1][0];
            var adapter = arguments[2][0];

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
            var callback = ChatRoomCallbackPrx.uncheckedCast(
                adapter.add(new ChatCallbackI(),
                            new Ice.Identity("callback", category)));

            //
            // Set the chat session callback.
            //
            return session.setCallback(callback);
        }
    ).then(
        function()
        {
            return setState(State.Connected);
        }
    ).then(
        function()
        {
            //
            // Process input events in the input textbox until
            // the chat promise is completed.
            //
            $("#input").keypress(
                function(e)
                {
                    if(!chat.completed())
                    {
                        //
                        // When enter key is pressed, we send a new message
                        // using the session's say operation and then reset
                        // the textbox contents.
                        //
                        if(e.which === 13)
                        {
                            var msg = $(this).val();
                            if(msg.length > 0)
                            {
                                $(this).val("");
                                if(msg.length > maxMessageSize)
                                {
                                    writeLine("<system-message> - Message length exceeded, " +
                                              "maximum length is " + maxMessageSize + " characters.");
                                }
                                else
                                {
                                    session.send(msg).then(
                                        function(timestamp)
                                        {
                                            writeLine(formatDate(timestamp.toNumber()) + " - <" +
                                                    username + "> - " + msg);
                                        },
                                        function(ex)
                                        {
                                            if(ex instanceof Chat.InvalidMessageException)
                                            {
                                                writeLine("<system-message> - " + ex.reason);
                                            }
                                            else
                                            {
                                                chat.fail(ex);
                                            }
                                        });
                                }
                            }
                            return false;
                        }
                    }
                });

            //
            // Exit the chat loop accepting the chat promise.
            //
            $("#signout").click(
                function(){
                    chat.succeed();
                    return false;
                });

            return chat;
        }
    ).finally(
        function()
        {
            //
            // Reset the input text box and chat output textarea.
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
            setState(State.Disconnected);
        }
    ).exception(
        function(ex)
        {
            //
            // Handle any exceptions that occurred while running.
            //
            setState(State.Disconnected, ex.toString());
        });
};

//
// Do a transition from "from" screen to "to" screen. Return
// a promise that allows us to wait for the transition to
// complete. If to screen is undefined just animate out the
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
                $(to).css("display", "block")
                    .animo({ animation: "flipInX", keep: true },
                        function(){ p.succeed(); });
            }
            else
            {
                p.succeed();
            }
        });
    return p;
};

//
// Set default height of output textarea
//
$("#output").height(300);

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
                w = w >= 100 ? 0 : w + 1;
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

//
// Dismiss error message on click.
//
function dismissError()
{
    transition("#signin-alert");
    hasError = false;
    return false;
}

//
// Switch the state and return a promise that is fulfilled
// when state change completes.
//
function setState(newState, error)
{
    assert(state !== newState);
    switch(newState)
    {
        case State.Disconnected:
        {
            assert(state === undefined ||
                   state === State.Connecting ||
                   state === State.Connected);

            $("#users a").off("click");
            $("#users").html("");
            $(window).off("beforeunload");

            //
            // First destroy the communicator if needed then do
            // the screen transition.
            //
            return Promise.try(
                function()
                {
                    if(communicator)
                    {
                        var c = communicator;
                        communicator = null;
                        return c.destroy();
                    }
                }
            ).finally(
                function()
                {
                    if(state !== undefined)
                    {
                        if(error)
                        {
                            hasError = true;
                            stopProgress(false);
                            $("#signin-alert span").text(error);
                            return transition("#loading", "#signin-alert").then(
                                function(){
                                    $("#loading .meter").css("width", "0%");
                                    $("#signin-form").css("display", "block")
                                        .animo({ animation: "flipInX", keep: true });
                                });
                        }
                        else
                        {
                            return transition("#chat-form", "#signin-form");
                        }
                    }
                }
            ).then(
                function()
                {
                    $("#username").focus();
                    $("#username").keypress(
                        function(e)
                        {
                            //
                            // After enter key is pressed in the username input,
                            // switch focus to password input.
                            //
                            if(e.which === 13)
                            {
                                $("#password").focus();
                                return false;
                            }
                        });

                    $("#password").keypress(
                        function(e)
                        {
                            //
                            // After enter key is pressed in the password input,
                            // sign-in.
                            //
                            if(e.which === 13)
                            {
                                signin();
                                return false;
                            }
                        });

                    $("#signin").click(function(){
                        signin();
                        return false;
                    });

                    state = State.Disconnected;
                });
        }
        case State.Connecting:
        {
            assert(state === State.Disconnected);
            username = formatUsername($("#username").val());

            //
            // Remove the signin form event handlers.
            //
            $("#username").off("keypress");
            $("#password").off("keypress");
            $("#signin").off("click");

            //
            // Dismiss any previous error message.
            //
            if(hasError)
            {
                dismissError();
            }

            //
            // Setup a before unload handler to prevent accidentally navigating
            // away from the page while the user is connected to the chat server.
            //
            $(window).on("beforeunload",
                function()
                {
                    return "If you navigate away from this page, the current chat session will be lost.";
                });

            //
            // Transition to loading screen
            //
            return transition("#signin-form", "#loading").then(
                function()
                {
                    startProgress();
                    state = State.Connecting;
                });
        }
        case State.Connected:
        {
            //
            // Stop animating the loading progress bar and
            // transition to the chat screen.
            //
            assert(state === State.Connecting);
            stopProgress(true);
            return transition("#loading", "#chat-form").then(
                function()
                {
                    $("#loading .meter").css("width", "0%");
                    $("#input").focus();
                    state = State.Connected;
                });
        }
    }
}
//
// Switch to initial state.
//
setState(State.Disconnected);

function formatDate(timestamp)
{
    var d = new Date();
    d.setTime(timestamp);
    return d.toLocaleTimeString().trim();
}

function formatUsername(s)
{
    return s.length < 2 ?
        s.toUpperCase() :
        s.substring(0, 1).toUpperCase() + s.substring(1, s.length).toLowerCase();
}

function writeLine(s)
{
    $("#output").val($("#output").val() + s + "\n");
    $("#output").scrollTop($("#output").get(0).scrollHeight);
}

var entities = [
    {entity: /&quot;/g, value: "\""},
    {entity: /&#39;/g, value: "'"},
    {entity: /&lt;/g, value: "<"},
    {entity: /&gt;/g, value: ">"},
    {entity: /&amp;/g, value: "&"}];

function unescapeHtml(msg)
{
    var e;
    for(var i = 0; i < entities.length; ++i)
    {
        e = entities[i];
        msg = msg.replace(e.entity, e.value);
    }
    return msg;
}

function assert(v)
{
    if(!v)
    {
        throw new Error("Assertion failed");
    }
}

}());
