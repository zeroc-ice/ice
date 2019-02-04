//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

const Ice = require("../Ice/ModuleRegistry").Ice;

Ice.TraceLevels = function(properties)
{
    const networkCat = "Network";
    const protocolCat = "Protocol";
    const retryCat = "Retry";
    const locationCat = "Locator";
    const slicingCat = "Slicing";

    const keyBase = "Ice.Trace.";

    const network = properties.getPropertyAsInt(keyBase + networkCat);
    const protocol = properties.getPropertyAsInt(keyBase + protocolCat);
    const retry = properties.getPropertyAsInt(keyBase + retryCat);
    const location = properties.getPropertyAsInt(keyBase + locationCat);
    const slicing = properties.getPropertyAsInt(keyBase + slicingCat);

    properties.getPropertyAsInt(keyBase + "ThreadPool"); // Avoid an "unused property" warning.

    return class
    {
        static get network()
        {
            return network;
        }

        static get networkCat()
        {
            return networkCat;
        }

        static get protocol()
        {
            return protocol;
        }

        static get protocolCat()
        {
            return protocolCat;
        }

        static get retry()
        {
            return retry;
        }

        static get retryCat()
        {
            return retryCat;
        }

        static get location()
        {
            return location;
        }

        static get locationCat()
        {
            return locationCat;
        }

        static get slicing()
        {
            return slicing;
        }

        static get slicingCat()
        {
            return slicingCat;
        }
    };
};
module.exports.Ice = Ice;
