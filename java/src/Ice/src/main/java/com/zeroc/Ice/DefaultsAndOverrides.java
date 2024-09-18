//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package com.zeroc.Ice;

final class DefaultsAndOverrides {
    DefaultsAndOverrides(Properties properties, Logger logger) {
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
                throw new InitializationException("invalid IP address set for Ice.Default.SourceAddress: `" + value + "'");
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
            overrideCompress = java.util.Optional.of(b);
        } else {
            overrideCompress = java.util.Optional.empty();
        }

        value = properties.getIceProperty("Ice.Override.Secure");
        if (!value.isEmpty()) {
            boolean overrideSecureValue = properties.getIcePropertyAsInt("Ice.Override.Secure") > 0;
            overrideSecure = java.util.Optional.of(overrideSecureValue);
        } else {
            overrideSecure = java.util.Optional.empty();
        }

        defaultCollocationOptimization = properties.getIcePropertyAsInt("Ice.Default.CollocationOptimized") > 0;

        value = properties.getIceProperty("Ice.Default.EndpointSelection");
        if (value.equals("Random")) {
            defaultEndpointSelection = EndpointSelectionType.Random;
        } else if (value.equals("Ordered")) {
            defaultEndpointSelection = EndpointSelectionType.Ordered;
        } else {
            throw new ParseException("illegal value '" + value + "' in property Ice.Default.EndpointSelection; expected 'Random' or 'Ordered'");
        }

        intValue = properties.getIcePropertyAsInt("Ice.Default.LocatorCacheTimeout");
        if (intValue < -1) {
            defaultLocatorCacheTimeout = -1;
            StringBuffer msg =
                    new StringBuffer("invalid value for Ice.Default.LocatorCacheTimeout `");
            msg.append(properties.getIceProperty("Ice.Default.LocatorCacheTimeout"));
            msg.append("': defaulting to -1");
            logger.warning(msg.toString());
        } else {
            defaultLocatorCacheTimeout = intValue;
        }

        intValue = properties.getIcePropertyAsInt("Ice.Default.InvocationTimeout");
        if (intValue < 1 && intValue != -1) {
            defaultInvocationTimeout = -1;
            StringBuffer msg =
                    new StringBuffer("invalid value for Ice.Default.InvocationTimeout `");
            msg.append(properties.getIceProperty("Ice.Default.InvocationTimeout"));
            msg.append("': defaulting to -1");
            logger.warning(msg.toString());
        } else {
            defaultInvocationTimeout = intValue;
        }

        defaultPreferSecure = properties.getIcePropertyAsInt("Ice.Default.PreferSecure") > 0;

        value = properties.getPropertyWithDefault("Ice.Default.EncodingVersion", Util.encodingVersionToString(Protocol.currentEncoding));
        defaultEncoding = Util.stringToEncodingVersion(value);
        Protocol.checkSupportedEncoding(defaultEncoding);

        boolean slicedFormat = properties.getIcePropertyAsInt("Ice.Default.SlicedFormat") > 0;
        defaultFormat = slicedFormat ? FormatType.SlicedFormat : FormatType.CompactFormat;
    }

    public final String defaultHost;
    public final java.net.InetSocketAddress defaultSourceAddress;
    public final String defaultProtocol;
    public final boolean defaultCollocationOptimization;
    public final EndpointSelectionType defaultEndpointSelection;
    public final int defaultLocatorCacheTimeout;
    public final int defaultInvocationTimeout;
    public final boolean defaultPreferSecure;
    public final EncodingVersion defaultEncoding;
    public final FormatType defaultFormat;

    public final java.util.Optional<Boolean> overrideCompress;
    public final java.util.Optional<Boolean> overrideSecure;
}
