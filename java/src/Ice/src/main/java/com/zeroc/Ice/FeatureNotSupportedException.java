//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package com.zeroc.Ice;

/**
 * This exception is raised if an unsupported feature is used. The
 * unsupported feature string contains the name of the unsupported
 * feature
 **/
public class FeatureNotSupportedException extends LocalException
{
    public FeatureNotSupportedException()
    {
        this.unsupportedFeature = "";
    }

    public FeatureNotSupportedException(Throwable cause)
    {
        super(cause);
        this.unsupportedFeature = "";
    }

    public FeatureNotSupportedException(String unsupportedFeature)
    {
        this.unsupportedFeature = unsupportedFeature;
    }

    public FeatureNotSupportedException(String unsupportedFeature, Throwable cause)
    {
        super(cause);
        this.unsupportedFeature = unsupportedFeature;
    }

    public String ice_id()
    {
        return "::Ice::FeatureNotSupportedException";
    }

    /**
     * The name of the unsupported feature.
     **/
    public String unsupportedFeature;

    /** @hidden */
    public static final long serialVersionUID = -4629958372080397318L;
}
