// **********************************************************************
//
// Copyright (c) 2002
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

package IceUtil;

public class XMLOutput extends OutputBase
{
    public
    XMLOutput()
    {
        super();
        _printed = true;
        _sgml = false;
    }

    public
    XMLOutput(java.io.PrintWriter writer)
    {
        super(writer);
        _printed = true;
        _sgml = false;
    }

    public
    XMLOutput(String s)
    {
        super(s);
        _printed = true;
        _sgml = false;
    }

    public void
    setSGML(boolean sgml)
    {
        _sgml = true;
    }

    public void
    print(String s)
    {
        if (!_printed)
        {
            _out.print('>');
            _printed = true;
        }
        super.print(s);
    }

    public void
    printEscaped(String s)
    {
        String v = s;

        //
        // Find out whether there is a reserved character to avoid
        // conversion if not necessary.
        //
        final String allReserved = "<>'\"&";
        boolean hasReserved = false;
        char[] arr = s.toCharArray();
        for (int i = 0; i < arr.length; i++)
        {
            if (allReserved.indexOf(arr[i]) != -1)
            {
                hasReserved = true;
                break;
            }
        }
        if (hasReserved)
        {
            //
            // First convert all & to &amp;
            //
            if (v.indexOf('&') != -1)
            {
                v = v.replaceAll("&", "&amp;");
            }

            //
            // Next convert remaining reserved characters.
            //
            if (v.indexOf('>') != -1)
            {
                v = v.replaceAll(">", "&gt;");
            }
            if (v.indexOf('<') != -1)
            {
                v = v.replaceAll("<", "&lt;");
            }
            if (v.indexOf('\'') != -1)
            {
                v = v.replaceAll("'", "&apos;");
            }
            if (v.indexOf('"') != -1)
            {
                v = v.replaceAll("\"", "&quot;");
            }
        }
        print(v);
    }

    public void
    nl()
    {
        if (!_printed)
        {
            _out.print('>');
            _printed = true;
        }
        super.nl();
    }

    public void
    se(String element)
    {
        nl();

        //
        // If we're not in SGML mode the output of the '>' character is
        // deferred until either the end-element (in which case a /> is
        // emitted) or until something is displayed.
        //
        _out.print('<');
        _out.print(element);
        if (_sgml)
        {
            _out.print('>');
        }
        else
        {
            _printed = false;
        }

        int pos = element.indexOf(' ');
        if (pos == -1)
        {
            pos = element.indexOf('\t');
        }
        if (pos == -1)
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
    }

    public void
    ee()
    {
        String element = (String)_elementStack.removeFirst();

        dec();
        if (!_printed)
        {
            _out.print("/>");
        }
        else
        {
            nl();
            _out.print("</");
            _out.print(element);
            _out.print('>');
        }
        --_pos; // TODO: ???
        _printed = true;
    }

    private java.util.LinkedList _elementStack = new java.util.LinkedList();
    private boolean _printed;
    private boolean _sgml;
}
