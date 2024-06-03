//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

const Ice = require("../Ice/ModuleRegistry").Ice;

class Logger
{
    constructor(prefix)
    {
        if(prefix !== undefined && prefix.length > 0)
        {
            this._prefix = prefix + ": ";
        }
        else
        {
            this._prefix = "";
        }

        this._dateformat =
        {
            year: 'numeric',
            month: 'numeric',
            day: 'numeric',
            hour: 'numeric',
            minute: 'numeric',
            second: 'numeric',
            hour12: false
        };
    }

    print(message)
    {
        this.write(message, false);
    }

    trace(category, message)
    {
        const s = [];
        s.push("-- ");
        s.push(this.timestamp());
        s.push(' ');
        s.push(this._prefix);
        s.push(category);
        s.push(": ");
        s.push(message);
        this.write(s.join(""), true);
    }

    warning(message)
    {
        const s = [];
        s.push("-! ");
        s.push(this.timestamp());
        s.push(' ');
        s.push(this._prefix);
        s.push("warning: ");
        s.push(message);
        this.write(s.join(""), true);
    }

    error(message)
    {
        const s = [];
        s.push("!! ");
        s.push(this.timestamp());
        s.push(' ');
        s.push(this._prefix);
        s.push("error: ");
        s.push(message);
        this.write(s.join(""), true);
    }

    cloneWithPrefix(prefix)
    {
        return new Logger(prefix);
    }

    write(message, indent)
    {
        if(indent)
        {
            message = message.replace(/\n/g, "\n   ");
        }

        console.log(message);
    }

    timestamp()
    {
        const d = new Date();
        return d.toLocaleString("en-US", this._dateformat) + "." + d.getMilliseconds();
    }
}

Ice.Logger = Logger;
module.exports.Ice = Ice;
