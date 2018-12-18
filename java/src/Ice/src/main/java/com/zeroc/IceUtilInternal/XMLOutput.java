// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package com.zeroc.IceUtilInternal;

public class XMLOutput extends OutputBase
{
    public
    XMLOutput()
    {
        super();
        _se = false;
        _text = false;
        _sgml = false;
        _escape = false;
    }

    public
    XMLOutput(java.io.PrintWriter writer)
    {
        super(writer);
        _se = false;
        _text = false;
        _sgml = false;
        _escape = false;
    }

    public
    XMLOutput(String s)
    {
        super(s);
        _se = false;
        _text = false;
        _sgml = false;
        _escape = false;
    }

    public void
    setSGML(boolean sgml)
    {
        _sgml = true;
    }

    @Override
    public void
    print(String s)
    {
        if(_se)
        {
            _out.print('>');
            _se = false;
        }
        _text = true;

        if(_escape)
        {
            String escaped = escape(s);
            super.print(escaped);
        }
        else
        {
            super.print(s);
        }
    }

    public XMLOutput
    write(String s)
    {
        print(s);
        return this;
    }

    @Override
    public void
    nl()
    {
        if(_se)
        {
            _se = false;
            _out.print('>');
        }
        super.nl();
    }

    public XMLOutput
    se(String element)
    {
        nl();

        //
        // If we're not in SGML mode the output of the '>' character is
        // deferred until either the end-element (in which case a /> is
        // emitted) or until something is displayed.
        //
        if(_escape)
        {
            _out.print('<');
            _out.print(escape(element));
        }
        else
        {
            _out.print('<');
            _out.print(element);
        }
        _se = true;
        _text = false;

        int pos = element.indexOf(' ');
        if(pos == -1)
        {
            pos = element.indexOf('\t');
        }
        if(pos == -1)
        {
            _elementStack.addFirst(element);
        }
        else
        {
            _elementStack.addFirst(element.substring(0, pos));
        }

        ++_pos; // TODO: ???
        inc();
        _separator = false;
        return this;
    }

    public XMLOutput
    ee()
    {
        String element = _elementStack.removeFirst();

        dec();
        if(_se)
        {
            //
            // SGML (docbook) doesn't support <foo/>
            //
            if(_sgml)
            {
                _out.print("></");
                _out.print(element);
                _out.print(">");
            }
            else
            {
                _out.print("/>");
            }
        }
        else
        {
            if(!_text)
            {
                nl();
            }
            _out.print("</");
            _out.print(element);
            _out.print(">");
        }
        --_pos; // TODO: ???

        _se = false;
        _text = false;
        return this;
    }

    public XMLOutput
    attr(String name, String value)
    {
        //
        // Precondition: Attributes can only be attached to elements.
        //
        assert(_se);
        _out.print(" ");
        _out.print(name);
        _out.print("=\"");
        _out.print(escape(value));
        _out.print("\"");
        return this;
    }

    public XMLOutput
    startEscapes()
    {
        _escape = true;
        return this;
    }

    public XMLOutput
    endEscapes()
    {
        _escape = false;
        return this;
    }

    public String
    currentElement()
    {
        if(_elementStack.size() > 0)
        {
            return _elementStack.getFirst();
        }
        else
        {
            return "";
        }
    }

    private String
    escape(String input)
    {
        String v = input;

        //
        // Find out whether there is a reserved character to avoid
        // conversion if not necessary.
        //
        final String allReserved = "<>'\"&";
        boolean hasReserved = false;
        char[] arr = input.toCharArray();
        for(int i = 0; i < arr.length; i++)
        {
            if(allReserved.indexOf(arr[i]) != -1)
            {
                hasReserved = true;
                break;
            }
        }
        if(hasReserved)
        {
            //
            // First convert all & to &amp;
            //
            if(v.indexOf('&') != -1)
            {
                v = v.replaceAll("&", "&amp;");
            }

            //
            // Next convert remaining reserved characters.
            //
            if(v.indexOf('>') != -1)
            {
                v = v.replaceAll(">", "&gt;");
            }
            if(v.indexOf('<') != -1)
            {
                v = v.replaceAll("<", "&lt;");
            }
            if(v.indexOf('\'') != -1)
            {
                v = v.replaceAll("'", "&apos;");
            }
            if(v.indexOf('"') != -1)
            {
                v = v.replaceAll("\"", "&quot;");
            }
        }
        return v;
    }

    private java.util.LinkedList<String> _elementStack = new java.util.LinkedList<>();

    boolean _se;
    boolean _text;

    private boolean _sgml;
    private boolean _escape;
}
