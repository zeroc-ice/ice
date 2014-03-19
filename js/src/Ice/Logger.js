// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

(function(global){
    var Ice = global.Ice || {};
    
    require("Ice/Class");
    
    var Logger = Ice.Class({
        __init__: function(prefix)
        {
            if(prefix !== undefined && prefix.length > 0)
            {
                this._prefix = prefix + ": ";
            }
            else
            {
                this._prefix = "";
            }
        },
        print: function(message)
        {
            this.write(message, false);
        },
        trace: function(category, message)
        {
            var s = [];
            var d = new Date();
            s.push("-- ");
            s.push(this.timestamp());
            s.push(' ');
            s.push(this._prefix);
            s.push(category);
            s.push(": ");
            s.push(message);
            this.write(s.join(""), true);
        },
        warning: function(message)
        {
            var s = [];
            var d = new Date();
            s.push("-! ");
            s.push(this.timestamp());
            s.push(' ');
            s.push(this._prefix);
            s.push("warning: ");
            s.push(message);
            this.write(s.join(""), true);
        },
        error: function(message)
        {
            var s = [];
            var d = new Date();
            s.push("!! ");
            s.push(this.timestamp());
            s.push(' ');
            s.push(this._prefix);
            s.push("error: ");
            s.push(message);
            this.write(s.join(""), true);
        },
        cloneWithPrefix: function(prefix)
        {
            return new Logger(prefix);
        },
        write: function(message, indent)
        {
            if(indent)
            {
                message = message.replace(/\n/g, "\n   ");
            }

            console.log(message);
        },
        timestamp: function()
        {
            var d = new Date();
            var mon = d.getMonth() + 1;
            mon = mon < 10 ? "0" + mon : mon;
            var day = d.getDate();
            day = day < 10 ? "0" + day : day;
            return mon + "-" + day + "-" + d.getFullYear() + " " + d.toLocaleTimeString() + "." + d.getMilliseconds();
        }
    });
    Ice.Logger = Logger;
    global.Ice = Ice;
}(typeof (global) === "undefined" ? window : global));
