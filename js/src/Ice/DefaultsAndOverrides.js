// Copyright (c) ZeroC, Inc.

import { FormatType } from "./FormatType.js";
import { EndpointSelectionType } from "./EndpointSelectionType.js";
import { Protocol, stringToEncodingVersion } from "./Protocol.js";
import { ParseException } from "./LocalExceptions.js";
import { TcpTransceiver } from "./TcpTransceiver.js";

export class DefaultsAndOverrides {
    constructor(properties, logger) {
        this.defaultProtocol = properties.getPropertyWithDefault(
            "Ice.Default.Protocol",
            TcpTransceiver !== null ? "tcp" : "ws",
        );

        let value = properties.getIceProperty("Ice.Default.Host");
        this.defaultHost = value.length > 0 ? value : null;

        value = properties.getIceProperty("Ice.Default.SourceAddress");
        this.defaultSourceAddress = value.length > 0 ? value : null;

        this.overrideSecure = false;

        value = properties.getIceProperty("Ice.Default.EndpointSelection");
        if (value === "Random") {
            this.defaultEndpointSelection = EndpointSelectionType.Random;
        } else if (value === "Ordered") {
            this.defaultEndpointSelection = EndpointSelectionType.Ordered;
        } else {
            throw new ParseException(
                `illegal value '${value}' in property Ice.Default.EndpointSelection; expected 'Random' or 'Ordered'`,
            );
        }

        this.defaultLocatorCacheTimeout = properties.getIcePropertyAsInt("Ice.Default.LocatorCacheTimeout");
        if (this.defaultLocatorCacheTimeout < -1) {
            throw new InitializationException(
                `invalid value for Ice.Default.LocatorCacheTimeout: ${this.defaultLocatorCacheTimeout}`,
            );
        }

        this.defaultInvocationTimeout = properties.getIcePropertyAsInt("Ice.Default.InvocationTimeout");
        if (this.defaultInvocationTimeout < 1 && this.defaultInvocationTimeout !== -1) {
            throw new InitializationException(
                `invalid value for Ice.Default.InvocationTimeout: ${this.defaultInvocationTimeout}`,
            );
        }

        this.defaultPreferSecure = properties.getIcePropertyAsInt("Ice.Default.PreferSecure") > 0;

        value = properties.getIceProperty("Ice.Default.EncodingVersion");
        this.defaultEncoding = stringToEncodingVersion(value);
        Protocol.checkSupportedEncoding(this.defaultEncoding);

        const slicedFormat = properties.getIcePropertyAsInt("Ice.Default.SlicedFormat") > 0;
        this.defaultFormat = slicedFormat ? FormatType.SlicedFormat : FormatType.CompactFormat;
    }
}
