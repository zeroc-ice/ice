// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

var Ice = require("../Ice/ModuleRegistry").Ice;
Ice.__M.require(module,
    [
        "../Ice/Class",
        "../Ice/Endpoint",
    ]);

var Class = Ice.Class;

var EndpointI = Class(Ice.Endpoint, {
    toString: function()
    {
        //
        // WARNING: Certain features, such as proxy validation in Glacier2,
        // depend on the format of proxy strings. Changes to toString() and
        // methods called to generate parts of the reference string could break
        // these features. Please review for all features that depend on the
        // format of proxyToString() before changing this and related code.
        //
        return this.protocol() + this.options();
    },
    initWithOptions: function(args)
    {
        var unknown = [];

        var i;
        var str = "`" + this.protocol();
        for(i = 0; i < args.length; ++i)
        {
            if(args[i].search(/[ \t\n\r]+/) !== -1)
            {
                str += " \"" + args[i] + "\"";
            }
            else
            {
                str += " " + args[i];
            }
        }
        str += "'";

        i = 0;
        while(i < args.length)
        {
            var option = args[i++];
            if(option.length < 2 || option.charAt(0) != '-')
            {
                unknown.push(option);
                continue;
            }

            var argument = null;
            if(i < args.length && args[i].charAt(0) != '-')
            {
                argument = args[i++];
            }

            if(!this.checkOption(option, argument, str))
            {
                unknown.push(option);
                if(argument !== null)
                {
                    unknown.push(argument);
                }
            }
        }

        args.length = 0;
        for(i = 0; i < unknown.length; i++)
        {
            args.push(unknown[i]);
        }
    },
    //
    // Compare endpoints for sorting purposes
    //
    equals: function(p)
    {
        if(!(p instanceof EndpointI))
        {
            return false;
        }
        return this.compareTo(p) === 0;
    },
    checkOption: function()
    {
        return false;
    }
});

Ice.EndpointI = EndpointI;
module.exports.Ice = Ice;
