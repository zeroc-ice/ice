//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

import { FormatType } from "./FormatType.js";
import { EndpointSelectionType } from "./EndpointSelectionType.js";
import { Protocol, stringToEncodingVersion, encodingVersionToString } from "./Protocol.js";
import { EndpointSelectionTypeParseException } from "./LocalExceptions.js";
import { TcpTransceiver } from "./TcpTransceiver.js";

export class DefaultsAndOverrides {
    constructor(properties, logger) {
        this.defaultProtocol = properties.getPropertyWithDefault(
            "Ice.Default.Protocol",
            TcpTransceiver !== null ? "tcp" : "ws",
        );

        let value = properties.getProperty("Ice.Default.Host");
        this.defaultHost = value.length > 0 ? value : null;

        value = properties.getProperty("Ice.Default.SourceAddress");
        this.defaultSourceAddress = value.length > 0 ? value : null;

        this.overrideSecure = false;

        value = properties.getPropertyWithDefault("Ice.Default.EndpointSelection", "Random");
        if (value === "Random") {
            this.defaultEndpointSelection = EndpointSelectionType.Random;
        } else if (value === "Ordered") {
            this.defaultEndpointSelection = EndpointSelectionType.Ordered;
        } else {
            const ex = new EndpointSelectionTypeParseException();
            ex.str = "illegal value `" + value + "'; expected 'Random' or 'Ordered'";
            throw ex;
        }

        this.defaultLocatorCacheTimeout = properties.getPropertyAsIntWithDefault("Ice.Default.LocatorCacheTimeout", -1);
        if (this.defaultLocatorCacheTimeout < -1) {
            this.defaultLocatorCacheTimeout = -1;
            logger.warning(
                "invalid value for Ice.Default.LocatorCacheTimeout `" +
                    properties.getProperty("Ice.Default.LocatorCacheTimeout") +
                    "': defaulting to -1",
            );
        }

        this.defaultInvocationTimeout = properties.getPropertyAsIntWithDefault("Ice.Default.InvocationTimeout", -1);
        if (this.defaultInvocationTimeout < 1 && this.defaultInvocationTimeout !== -1) {
            this.defaultInvocationTimeout = -1;
            logger.warning(
                "invalid value for Ice.Default.InvocationTimeout `" +
                    properties.getProperty("Ice.Default.InvocationTimeout") +
                    "': defaulting to -1",
            );
        }

        this.defaultPreferSecure = properties.getPropertyAsIntWithDefault("Ice.Default.PreferSecure", 0) > 0;

        value = properties.getPropertyWithDefault(
            "Ice.Default.EncodingVersion",
            encodingVersionToString(Protocol.currentEncoding),
        );
        this.defaultEncoding = stringToEncodingVersion(value);
        Protocol.checkSupportedEncoding(this.defaultEncoding);

        const slicedFormat = properties.getPropertyAsIntWithDefault("Ice.Default.SlicedFormat", 0) > 0;
        this.defaultFormat = slicedFormat ? FormatType.SlicedFormat : FormatType.CompactFormat;
    }
}
