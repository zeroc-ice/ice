// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

//
// Only works with simple, single endpoint proxy strings. Not super
// efficient either. Works for endpoint configurations too.
//
public class ProxyStringHelper
{
    private String 
    getPart(String s, String p)
    {
        //
        // We can't use lastIndexOf because it is not part of MIDP.
        //
        if(s == null || s.length() == 0 || s.length() < p.length())
        {
            return "";
        }

        int last = 0;
        int i = s.indexOf(p, last);
        int next = s.indexOf(p, i + 1);
        while(next != -1)
        {
            last = next;
            next = s.indexOf(p, last + 1);
        }

        if(last > 0)
        {
            i = last;
        }

        if(i != -1)
        {
            int start = i + p.length() + 1;
            String t = s.substring(i + p.length() +1);
            t = t.trim();
            int end = t.indexOf(' ');
            if(end != -1)
            {
                return t.substring(0, end);
            }
            else
            {
                return t;
            }
        }
        return "";
    }

    ProxyStringHelper(String s)
    {
        if(s != null && s.length() > 0)
        {
            _host = getPart(s, "-h");
            _port = getPart(s, "-p");
            _timeout = getPart(s, "-t");
        }
    }

    public String
    host()
    {
        if(_host == null || _host.equals(""))
        {
            return "127.0.0.1";
        }
        return _host;
    }

    public String
    port()
    {
        if(_port == null || _port.equals(""))
        {
            return "12010";
        }
        return _port;
    }

    public String
    timeout()
    {
        if(_timeout == null || _timeout.equals(""))
        {
            return "10000";
        }
        return _timeout;
    }

    private String _host;
    private String _port;
    private String _timeout;
};

