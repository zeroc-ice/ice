// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceInternal;

public final class ACMConfig implements java.lang.Cloneable
{
    ACMConfig(boolean server)
    {
        timeout = 60 * 1000;
        heartbeat = Ice.ACMHeartbeat.HeartbeatOnDispatch;
        close = server ? Ice.ACMClose.CloseOnInvocation : Ice.ACMClose.CloseOnInvocationAndIdle;
    }

    public ACMConfig(Ice.Properties p, Ice.Logger l, String prefix, ACMConfig dflt)
    {
        assert(prefix != null);

        String timeoutProperty;
        if((prefix.equals("Ice.ACM.Client") || prefix.equals("Ice.ACM.Server")) &&
           p.getProperty(prefix + ".Timeout").isEmpty())
        {
            timeoutProperty = prefix; // Deprecated property.
        }
        else
        {
            timeoutProperty = prefix + ".Timeout";
        }

        timeout = p.getPropertyAsIntWithDefault(timeoutProperty, dflt.timeout / 1000) * 1000; // To milliseconds
        if(timeout < 0)
        {
            l.warning("invalid value for property `" + timeoutProperty + "', default value will be used instead");
            timeout = dflt.timeout;
        }

        int hb = p.getPropertyAsIntWithDefault(prefix + ".Heartbeat", dflt.heartbeat.ordinal());
        Ice.ACMHeartbeat[] heartbeatValues = Ice.ACMHeartbeat.values();
        if(hb >= 0 && hb < heartbeatValues.length)
        {
            heartbeat = heartbeatValues[hb];
        }
        else
        {
            l.warning("invalid value for property `" + prefix + ".Heartbeat" + "', default value will be used instead");
            heartbeat = dflt.heartbeat;
        }

        Ice.ACMClose[] closeValues = Ice.ACMClose.values();
        int cl = p.getPropertyAsIntWithDefault(prefix + ".Close", dflt.close.ordinal());
        if(cl >= 0 && cl < closeValues.length)
        {
            close = closeValues[cl];
        }
        else
        {
            l.warning("invalid value for property `" + prefix + ".Close" + "', default value will be used instead");
            close = dflt.close;
        }
    }

    @Override
    public ACMConfig
    clone()
    {
        ACMConfig c = null;
        try
        {
            c = (ACMConfig)super.clone();
        }
        catch(CloneNotSupportedException ex)
        {
            assert false; // impossible
        }
        return c;
    }

    public int timeout;
    public Ice.ACMHeartbeat heartbeat;
    public Ice.ACMClose close;
}
