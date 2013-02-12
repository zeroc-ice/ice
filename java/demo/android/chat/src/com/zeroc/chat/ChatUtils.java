// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package com.zeroc.chat;

import java.text.DateFormat;
import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.TimeZone;

public class ChatUtils
{
    //
    // This function unescapes HTML entities in the data string
    // and return the unescaped string.
    //
    public static String unstripHtml(String data)
    {
        data = data.replace("&quot;", "\"");
        data = data.replace("&#39;", "'");
        data = data.replace("&lt;", "<");
        data = data.replace("&gt;", ">");
        data = data.replace("&amp;", "&");
        return data;
    }

    public static String formatTimestamp(long timestamp)
    {
        DateFormat dtf = new SimpleDateFormat("HH:mm:ss");
        dtf.setTimeZone(TimeZone.getDefault());
        return dtf.format(new Date(timestamp));
    }

    public static String formatUsername(String in)
    {
        try
        {
            in = in.substring(0, 1).toUpperCase() + in.substring(1, in.length()).toLowerCase();
        }
        catch(IndexOutOfBoundsException ex)
        {
        }
        return in;
    }
};

