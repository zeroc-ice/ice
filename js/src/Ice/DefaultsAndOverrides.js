//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

const Ice = require("../Ice/ModuleRegistry").Ice;

require("../Ice/EndpointTypes");
require("../Ice/FormatType");
require("../Ice/LocalException");
require("../Ice/Protocol");

const FormatType = Ice.FormatType;
const EndpointSelectionType = Ice.EndpointSelectionType;
const Protocol = Ice.Protocol;

class DefaultsAndOverrides
{
    constructor(properties, logger)
    {
        this.defaultProtocol = properties.getPropertyWithDefault("Ice.Default.Protocol",
                                                                 Ice.TcpTransceiver !== null ? "tcp" : "ws");

        let value = properties.getProperty("Ice.Default.Host");
        this.defaultHost = value.length > 0 ? value : null;

        value = properties.getProperty("Ice.Default.SourceAddress");
        this.defaultSourceAddress = value.length > 0 ? value : null;

        value = properties.getProperty("Ice.Override.Timeout");
        if(value.length > 0)
        {
            this.overrideTimeout = true;
            this.overrideTimeoutValue = properties.getPropertyAsInt("Ice.Override.Timeout");
            if(this.overrideTimeoutValue < 1 && this.overrideTimeoutValue !== -1)
            {
                this.overrideTimeoutValue = -1;
                logger.warning("invalid value for Ice.Override.Timeout `" +
                                properties.getProperty("Ice.Override.Timeout") + "': defaulting to -1");
            }
        }
        else
        {
            this.overrideTimeout = false;
            this.overrideTimeoutValue = -1;
        }

        value = properties.getProperty("Ice.Override.ConnectTimeout");
        if(value.length > 0)
        {
            this.overrideConnectTimeout = true;
            this.overrideConnectTimeoutValue = properties.getPropertyAsInt("Ice.Override.ConnectTimeout");
            if(this.overrideConnectTimeoutValue < 1 && this.overrideConnectTimeoutValue !== -1)
            {
                this.overrideConnectTimeoutValue = -1;
                logger.warning("invalid value for Ice.Override.ConnectTimeout `" +
                                properties.getProperty("Ice.Override.ConnectTimeout") + "': defaulting to -1");
            }
        }
        else
        {
            this.overrideConnectTimeout = false;
            this.overrideConnectTimeoutValue = -1;
        }

        value = properties.getProperty("Ice.Override.CloseTimeout");
        if(value.length > 0)
        {
            this.overrideCloseTimeout = true;
            this.overrideCloseTimeoutValue = properties.getPropertyAsInt("Ice.Override.CloseTimeout");
            if(this.overrideCloseTimeoutValue < 1 && this.overrideCloseTimeoutValue !== -1)
            {
                this.overrideCloseTimeoutValue = -1;
                logger.warning("invalid value for Ice.Override.CloseTimeout `" +
                                properties.getProperty("Ice.Override.CloseTimeout") + "': defaulting to -1");
            }
        }
        else
        {
            this.overrideCloseTimeout = false;
            this.overrideCloseTimeoutValue = -1;
        }

        this.overrideSecure = false;

        value = properties.getPropertyWithDefault("Ice.Default.EndpointSelection", "Random");
        if(value === "Random")
        {
            this.defaultEndpointSelection = EndpointSelectionType.Random;
        }
        else if(value === "Ordered")
        {
            this.defaultEndpointSelection = EndpointSelectionType.Ordered;
        }
        else
        {
            const ex = new Ice.EndpointSelectionTypeParseException();
            ex.str = "illegal value `" + value + "'; expected `Random' or `Ordered'";
            throw ex;
        }

        this.defaultTimeout = properties.getPropertyAsIntWithDefault("Ice.Default.Timeout", 60000);
        if(this.defaultTimeout < 1 && this.defaultTimeout !== -1)
        {
            this.defaultTimeout = 60000;
            logger.warning("invalid value for Ice.Default.Timeout `" + properties.getProperty("Ice.Default.Timeout") +
                        "': defaulting to 60000");
        }

        this.defaultLocatorCacheTimeout = properties.getPropertyAsIntWithDefault("Ice.Default.LocatorCacheTimeout", -1);
        if(this.defaultLocatorCacheTimeout < -1)
        {
            this.defaultLocatorCacheTimeout = -1;
            logger.warning("invalid value for Ice.Default.LocatorCacheTimeout `" +
                        properties.getProperty("Ice.Default.LocatorCacheTimeout") + "': defaulting to -1");
        }

        this.defaultInvocationTimeout = properties.getPropertyAsIntWithDefault("Ice.Default.InvocationTimeout", -1);
        if(this.defaultInvocationTimeout < 1 && this.defaultInvocationTimeout !== -1)
        {
            this.defaultInvocationTimeout = -1;
            logger.warning("invalid value for Ice.Default.InvocationTimeout `" +
                        properties.getProperty("Ice.Default.InvocationTimeout") + "': defaulting to -1");
        }

        this.defaultPreferSecure = properties.getPropertyAsIntWithDefault("Ice.Default.PreferSecure", 0) > 0;

        value = properties.getPropertyWithDefault("Ice.Default.EncodingVersion",
                                                Ice.encodingVersionToString(Protocol.currentEncoding));
        this.defaultEncoding = Ice.stringToEncodingVersion(value);
        Protocol.checkSupportedEncoding(this.defaultEncoding);

        const slicedFormat = properties.getPropertyAsIntWithDefault("Ice.Default.SlicedFormat", 0) > 0;
        this.defaultFormat = slicedFormat ? FormatType.SlicedFormat : FormatType.CompactFormat;
    }
}

Ice.DefaultsAndOverrides = DefaultsAndOverrides;
module.exports.Ice = Ice;
