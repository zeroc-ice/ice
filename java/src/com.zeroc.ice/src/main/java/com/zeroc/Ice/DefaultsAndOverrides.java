// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

import java.net.InetSocketAddress;
import java.time.Duration;
import java.util.Optional;

final class DefaultsAndOverrides {
    DefaultsAndOverrides(Properties properties) {
        String value;
        int intValue;

        defaultProtocol = properties.getIceProperty("Ice.Default.Protocol");

        value = properties.getIceProperty("Ice.Default.Host");
        if (!value.isEmpty()) {
            defaultHost = value;
        } else {
            defaultHost = null;
        }

        value = properties.getIceProperty("Ice.Default.SourceAddress");
        if (!value.isEmpty()) {
            defaultSourceAddress = Network.getNumericAddress(value);
            if (defaultSourceAddress == null) {
                throw new InitializationException(
                    "invalid IP address set for Ice.Default.SourceAddress: `" + value + "'");
            }
        } else {
            defaultSourceAddress = null;
        }

        value = properties.getIceProperty("Ice.Override.Compress");
        if (!value.isEmpty()) {
            boolean b = properties.getIcePropertyAsInt("Ice.Override.Compress") > 0;
            if (b && !BZip2.supported()) {
                System.err.println(
                    "warning: bzip2 support not available, Ice.Override.Compress ignored");
                b = false;
            }
            overrideCompress = Optional.of(b);
        } else {
            overrideCompress = Optional.empty();
        }

        defaultCollocationOptimization =
            properties.getIcePropertyAsInt("Ice.Default.CollocationOptimized") > 0;

        value = properties.getIceProperty("Ice.Default.EndpointSelection");
        if ("Random".equals(value)) {
            defaultEndpointSelection = EndpointSelectionType.Random;
        } else if ("Ordered".equals(value)) {
            defaultEndpointSelection = EndpointSelectionType.Ordered;
        } else {
            throw new ParseException(
                "illegal value '"
                    + value
                    + "' in property Ice.Default.EndpointSelection; expected 'Random' or 'Ordered'");
        }

        intValue = properties.getIcePropertyAsInt("Ice.Default.LocatorCacheTimeout");
        if (intValue < -1) {
            throw new InitializationException(
                "invalid value for Ice.Default.LocatorCacheTimeout: " + intValue);
        }
        defaultLocatorCacheTimeout = Duration.ofSeconds(intValue);

        intValue = properties.getIcePropertyAsInt("Ice.Default.InvocationTimeout");
        if (intValue < 1 && intValue != -1) {
            throw new InitializationException(
                "invalid value for Ice.Default.InvocationTimeout: " + intValue);
        }
        defaultInvocationTimeout = Duration.ofMillis(intValue);

        value = properties.getIceProperty("Ice.Default.EncodingVersion");
        defaultEncoding = Util.stringToEncodingVersion(value);
        Protocol.checkSupportedEncoding(defaultEncoding);

        boolean slicedFormat = properties.getIcePropertyAsInt("Ice.Default.SlicedFormat") > 0;
        defaultFormat = slicedFormat ? FormatType.SlicedFormat : FormatType.CompactFormat;
    }

    public final String defaultHost;
    public final InetSocketAddress defaultSourceAddress;
    public final String defaultProtocol;
    public final boolean defaultCollocationOptimization;
    public final EndpointSelectionType defaultEndpointSelection;
    public final Duration defaultLocatorCacheTimeout;
    public final Duration defaultInvocationTimeout;
    public final EncodingVersion defaultEncoding;
    public final FormatType defaultFormat;

    public final Optional<Boolean> overrideCompress;
}
