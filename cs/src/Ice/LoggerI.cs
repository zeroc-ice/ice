// **********************************************************************
//
// Copyright (c) 2003
// ZeroC, Inc.
// Billerica, MA, USA
//
// All Rights Reserved.
//
// Ice is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License version 2 as published by
// the Free Software Foundation.
//
// **********************************************************************
namespace Ice
{

    using System.Globalization;

    public sealed class LoggerI : LocalObjectImpl, Logger
    {
	public LoggerI(string prefix, bool timestamp)
	{
	    if(prefix.Length > 0)
	    {
		_prefix = prefix + ": ";
	    }
	    
	    if(timestamp)
	    {
		_date = "G";
	    }
	}
	
	public void trace(string category, string message)
	{
	    lock(_globalMutex)
	    {
		System.Text.StringBuilder s = new System.Text.StringBuilder("[ ");
		if(_date != null)
		{
		    s.Append(System.DateTime.Now.ToString(_date, DateTimeFormatInfo.InvariantInfo));
		    s.Append(' ');
		}
		s.Append(_prefix);
		s.Append(category);
		s.Append(": ");
		s.Append(message);
		s.Append(" ]");
		s.Replace("\n", "\n ");
		System.Console.Error.WriteLine(s.ToString());
	    }
	}
	
	public void warning(string message)
	{
	    lock(_globalMutex)
	    {
		System.Text.StringBuilder s = new System.Text.StringBuilder();
		if(_date != null)
		{
		    s.Append(System.DateTime.Now.ToString(_date, DateTimeFormatInfo.InvariantInfo));
		    s.Append(' ');
		}
		s.Append(_prefix);
		s.Append("warning: ");
		s.Append(message);
		System.Console.Error.WriteLine(s.ToString());
	    }
	}
	
	public void error(string message)
	{
	    lock(_globalMutex)
	    {
		System.Text.StringBuilder s = new System.Text.StringBuilder();
		if(_date != null)
		{
		    s.Append(System.DateTime.Now.ToString(_date, DateTimeFormatInfo.InvariantInfo));
		    s.Append(' ');
		}
		s.Append(_prefix);
		s.Append("error: ");
		s.Append(message);
		System.Console.Error.WriteLine(s.ToString());
	    }
	}
	
	internal string _prefix = "";
	internal static object _globalMutex;
	internal string _date = null;
	static LoggerI()
	{
	    _globalMutex = new object();
	}
    }

}
