//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package com.zeroc.IceInternal;

public final class DefaultsAndOverrides {
  DefaultsAndOverrides(com.zeroc.Ice.Properties properties, com.zeroc.Ice.Logger logger) {
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
        throw new com.zeroc.Ice.InitializationException(
            "invalid IP address set for Ice.Default.SourceAddress: `" + value + "'");
      }
    } else {
      defaultSourceAddress = null;
    }

    value = properties.getIceProperty("Ice.Override.Timeout");
    if (!value.isEmpty()) {
      overrideTimeout = true;
      intValue = properties.getIcePropertyAsInt("Ice.Override.Timeout");
      if (intValue < 0 && intValue != -1) {
        overrideTimeoutValue = -1;
        StringBuffer msg = new StringBuffer("invalid value for Ice.Override.Timeout `");
        msg.append(properties.getIceProperty("Ice.Override.Timeout"));
        msg.append("': defaulting to -1");
        logger.warning(msg.toString());
      } else {
        overrideTimeoutValue = intValue;
      }
    } else {
      overrideTimeout = false;
      overrideTimeoutValue = -1;
    }

    value = properties.getIceProperty("Ice.Override.ConnectTimeout");
    if (!value.isEmpty()) {
      overrideConnectTimeout = true;
      intValue = properties.getIcePropertyAsInt("Ice.Override.ConnectTimeout");
      if (intValue < 0 && intValue != -1) {
        overrideConnectTimeoutValue = -1;
        StringBuffer msg = new StringBuffer("invalid value for Ice.Override.ConnectTimeout `");
        msg.append(properties.getIceProperty("Ice.Override.ConnectTimeout"));
        msg.append("': defaulting to -1");
        logger.warning(msg.toString());
      } else {
        overrideConnectTimeoutValue = intValue;
      }
    } else {
      overrideConnectTimeout = false;
      overrideConnectTimeoutValue = -1;
    }

    value = properties.getIceProperty("Ice.Override.CloseTimeout");
    if (!value.isEmpty()) {
      overrideCloseTimeout = true;
      intValue = properties.getIcePropertyAsInt("Ice.Override.CloseTimeout");
      if (intValue < 0 && intValue != -1) {
        overrideCloseTimeoutValue = -1;
        StringBuffer msg = new StringBuffer("invalid value for Ice.Override.CloseTimeout `");
        msg.append(properties.getIceProperty("Ice.Override.CloseTimeout"));
        msg.append("': defaulting to -1");
        logger.warning(msg.toString());
      } else {
        overrideCloseTimeoutValue = intValue;
      }
    } else {
      overrideCloseTimeout = false;
      overrideCloseTimeoutValue = -1;
    }

    value = properties.getIceProperty("Ice.Override.Compress");
    if (!value.isEmpty()) {
      overrideCompress = true;
      boolean b = properties.getIcePropertyAsInt("Ice.Override.Compress") > 0;
      if (b && !BZip2.supported()) {
        System.err.println("warning: bzip2 support not available, Ice.Override.Compress ignored");
        b = false;
      }
      overrideCompressValue = b;
    } else {
      overrideCompress = false;
      overrideCompressValue = false;
    }

    value = properties.getIceProperty("Ice.Override.Secure");
    if (!value.isEmpty()) {
      overrideSecure = true;
      overrideSecureValue = properties.getIcePropertyAsInt("Ice.Override.Secure") > 0;
    } else {
      overrideSecure = false;
      overrideSecureValue = false;
    }

    defaultCollocationOptimization =
        properties.getIcePropertyAsInt("Ice.Default.CollocationOptimized") > 0;

    value = properties.getIceProperty("Ice.Default.EndpointSelection");
    if (value.equals("Random")) {
      defaultEndpointSelection = com.zeroc.Ice.EndpointSelectionType.Random;
    } else if (value.equals("Ordered")) {
      defaultEndpointSelection = com.zeroc.Ice.EndpointSelectionType.Ordered;
    } else {
      com.zeroc.Ice.EndpointSelectionTypeParseException ex =
          new com.zeroc.Ice.EndpointSelectionTypeParseException();
      ex.str = "illegal value `" + value + "'; expected `Random' or `Ordered'";
      throw ex;
    }

    intValue = properties.getIcePropertyAsInt("Ice.Default.Timeout");
    if (intValue < 1 && intValue != -1) {
      defaultTimeout = 60000;
      StringBuffer msg = new StringBuffer("invalid value for Ice.Default.Timeout `");
      msg.append(properties.getProperty("Ice.Default.Timeout"));
      msg.append("': defaulting to 60000");
      logger.warning(msg.toString());
    } else {
      defaultTimeout = intValue;
    }

    intValue = properties.getIcePropertyAsInt("Ice.Default.LocatorCacheTimeout");
    if (intValue < -1) {
      defaultLocatorCacheTimeout = -1;
      StringBuffer msg = new StringBuffer("invalid value for Ice.Default.LocatorCacheTimeout `");
      msg.append(properties.getIceProperty("Ice.Default.LocatorCacheTimeout"));
      msg.append("': defaulting to -1");
      logger.warning(msg.toString());
    } else {
      defaultLocatorCacheTimeout = intValue;
    }

    intValue = properties.getIcePropertyAsInt("Ice.Default.InvocationTimeout");
    if (intValue < 1 && intValue != -1 && intValue != -2) {
      defaultInvocationTimeout = -1;
      StringBuffer msg = new StringBuffer("invalid value for Ice.Default.InvocationTimeout `");
      msg.append(properties.getIceProperty("Ice.Default.InvocationTimeout"));
      msg.append("': defaulting to -1");
      logger.warning(msg.toString());
    } else {
      defaultInvocationTimeout = intValue;
    }

    defaultPreferSecure = properties.getIcePropertyAsInt("Ice.Default.PreferSecure") > 0;

    value =
        properties.getPropertyWithDefault(
            "Ice.Default.EncodingVersion",
            com.zeroc.Ice.Util.encodingVersionToString(Protocol.currentEncoding));
    defaultEncoding = com.zeroc.Ice.Util.stringToEncodingVersion(value);
    Protocol.checkSupportedEncoding(defaultEncoding);

    boolean slicedFormat = properties.getIcePropertyAsInt("Ice.Default.SlicedFormat") > 0;
    defaultFormat =
        slicedFormat
            ? com.zeroc.Ice.FormatType.SlicedFormat
            : com.zeroc.Ice.FormatType.CompactFormat;
  }

  public final String defaultHost;
  public final java.net.InetSocketAddress defaultSourceAddress;
  public final String defaultProtocol;
  public final boolean defaultCollocationOptimization;
  public final com.zeroc.Ice.EndpointSelectionType defaultEndpointSelection;
  public final int defaultTimeout;
  public final int defaultLocatorCacheTimeout;
  public final int defaultInvocationTimeout;
  public final boolean defaultPreferSecure;
  public final com.zeroc.Ice.EncodingVersion defaultEncoding;
  public final com.zeroc.Ice.FormatType defaultFormat;

  public final boolean overrideTimeout;
  public final int overrideTimeoutValue;
  public final boolean overrideConnectTimeout;
  public final int overrideConnectTimeoutValue;
  public final boolean overrideCloseTimeout;
  public final int overrideCloseTimeoutValue;
  public final boolean overrideCompress;
  public final boolean overrideCompressValue;
  public final boolean overrideSecure;
  public final boolean overrideSecureValue;
}
