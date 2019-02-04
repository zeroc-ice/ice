//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

const Ice = require("../Ice/Endpoint").Ice;

class EndpointI
{
    toString()
    {
        //
        // WARNING: Certain features, such as proxy validation in Glacier2,
        // depend on the format of proxy strings. Changes to toString() and
        // methods called to generate parts of the reference string could break
        // these features. Please review for all features that depend on the
        // format of proxyToString() before changing this and related code.
        //
        return this.protocol() + this.options();
    }

    initWithOptions(args)
    {
        const unknown = [];

        let str = "`" + this.protocol();
        for(let i = 0; i < args.length; ++i)
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

        for(let i = 0; i < args.length;)
        {
            const option = args[i++];
            if(option.length < 2 || option.charAt(0) != '-')
            {
                unknown.push(option);
                continue;
            }

            let argument = null;
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
        for(let i = 0; i < unknown.length; i++)
        {
            args.push(unknown[i]);
        }
    }

    //
    // Compare endpoints for sorting purposes
    //
    equals(p)
    {
        if(!(p instanceof EndpointI))
        {
            return false;
        }
        return this.compareTo(p) === 0;
    }

    checkOption()
    {
        return false;
    }
}

Ice.EndpointI = EndpointI;
module.exports.Ice = Ice;
