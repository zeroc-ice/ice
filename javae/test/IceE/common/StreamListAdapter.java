// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

package Test;

//
// An OutputStream adapter that redirects stream output to a MIDP list box.
//

public class StreamListAdapter extends java.io.OutputStream
{
    public
    StreamListAdapter(javax.microedition.lcdui.List listui)
    {
	_list = listui;
    }

    public void
    close()
    {
	synchronized(this)
	{
	    _closed = true;
	}
    }

    public void
    flush()
	throws java.io.IOException
    {
	synchronized(this)
	{
	    if(_closed)
	    {
		throw new java.io.IOException("Stream closed.");
	    }
	    
	    if(_currentLine.length() != 0)
	    {
		writeCurrentLine();
		_replaceLine = true;
	    }
	}
    }

    public void
    write(int b)
	throws java.io.IOException
    {
	synchronized(this)
	{
	    if(_closed)
	    {
		throw new java.io.IOException("Stream closed.");
	    }
	
	    //
	    // If there is no associated list element then don't do anything.
	    //
	    if(_list == null)
	    {
		return;
	    }
	
	    if((char)b != '\n')
	    {
		_currentLine.append((char)b);
	    }
	    else
	    {
		writeCurrentLine();
		_currentLine.setLength(0);
		if(_list.size() > MAX_SCROLLBACK_LINES)
		{
		    _list.delete(0);
		}
		_replaceLine = false;
	    }
	}
    }

    private void
    writeCurrentLine()
    {
	if(_replaceLine)
	{
	    _list.set(_list.size() -1, _currentLine.toString(), null);
	}
	else
	{
	    _list.append(_currentLine.toString(), null);
	}
    }

    private boolean _closed = false;
    private boolean _replaceLine = false;
    private javax.microedition.lcdui.List _list;
    private StringBuffer _currentLine = new StringBuffer();
    private static final int MAX_SCROLLBACK_LINES = 200;
}
