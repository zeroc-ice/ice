// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

namespace IceUtil
{

using System.Collections;
using System.IO;
using System.Diagnostics;

public class XMLOutput : OutputBase
{
    public XMLOutput()
	: base()
    {
	_elementStack = new Stack();
	_se = false;
	_text = false;
	_sgml = false;
	_escape = false;
    }
    
    public XMLOutput(StreamWriter writer)
	: base(writer)
    {
	    _elementStack = new Stack();
	    _se = false;
	    _text = false;
	    _sgml = false;
	    _escape = false;
    }
    
    public XMLOutput(string s)
	: base(s)
    {
	    _elementStack = new Stack();
	    _se = false;
	    _text = false;
	    _sgml = false;
	    _escape = false;
    }
    
    virtual public void 
    setSGML(bool sgml)
    {
	_sgml = true;
    }

    public override void
    print(string s)
    {
	if(_se)
	{
	    _out.Write(">");
	    _se = false;
	}
	_text = true;
	
	if(_escape)
	{
	    string escaped = escape(s);
	    base.print(escaped);
	}
	else
	{
	    base.print(s);
	}
    }
    
    public virtual XMLOutput
    write(string s)
    {
	print(s);
	return this;
    }
    
    public override void
    nl()
    {
	if(_se)
	{
	    _se = false;
	    _out.Write(">");
	}
	base.nl();
    }
    
    public virtual XMLOutput
    se(string element)
    {
	nl();
	
	//
	// If we're not in SGML mode the output of the '>' character is
	// deferred until either the end-element (in which case a /> is
	// emitted) or until something is displayed.
	//
	if(_escape)
	{
	    _out.Write('<');
	    _out.Write(escape(element));
	}
	else
	{
	    _out.Write('<');
	    _out.Write(element);
	}
	_se = true;
	_text = false;
	
	int pos = element.IndexOf(' ');
	if (pos == - 1)
	{
	    pos = element.IndexOf('\t');
	}
	if (pos == - 1)
	{
	    _elementStack.Push(element);
	}
	else
	{
	    _elementStack.Push(element.Substring(0, pos - 1));
	}
	
	++_pos; // TODO: ???
	inc();
	_separator = false;
	return this;
    }
    
    public virtual XMLOutput
    ee()
    {
	string element = (string)_elementStack.Pop();
        
	dec();
	if(_se)
	{
	    //
	    // SGML (docbook) doesn't support <foo/>
	    //
	    if(_sgml)
	    {
		_out.Write("></");
		_out.Write(element);
		_out.Write(">");
	    }
	    else
	    {
		_out.Write("/>");
	    }
	}
	else
	{
	    if(!_text)
	    {
		nl();
	    }
	    _out.Write("</");
	    _out.Write(element);
	    _out.Write(">");
	}
	--_pos; // TODO: ???
        
	_se = false;
	_text = false;
	return this;
    }
    
    public virtual XMLOutput
    attr(string name, string val)
    {
	//
	// Precondition: Attributes can only be attached to elements.
	//
	Debug.Assert(_se);
	_out.Write(" ");
	_out.Write(name);
	_out.Write("=\"");
	_out.Write(escape(val));
	_out.Write("\"");
	return this;
    }
    
    public virtual XMLOutput
    startEscapes()
    {
	_escape = true;
	return this;
    }
    
    public virtual XMLOutput
    endEscapes()
    {
	_escape = false;
	return this;
    }
    
    public virtual string
    currentElement()
    {
	if(_elementStack.Count > 0)
	{
	    return (string)_elementStack.Peek();
	}
	else
	{
	    return "";
	}
    }
    
    private string
    escape(string input)
    {
	string v = input;
	
	//
	// Find out whether there is a reserved character to avoid
	// conversion if not necessary.
	//
	string allReserved = "<>'\"&";
	bool hasReserved = false;
	char[] arr = input.ToCharArray();
	for(int i = 0; i < arr.Length; i++)
	{
	    if(allReserved.IndexOf((char)arr[i]) != - 1)
	    {
		hasReserved = true;
		break;
	    }
	}
	if(hasReserved)
	{
	    int index;

	    //
	    // First convert all & to &amp;
	    //
	    index = v.IndexOf('&');
	    if(index != - 1)
	    {
		v = v.Insert(index, "amp;");
	    }
	    
	    //
	    // Next convert remaining reserved characters.
	    //
	    index = v.IndexOf('>');
	    if(index != - 1)
	    {
		string tmp = v.Substring(0, index);
		tmp += "&gt";
		tmp += v.Substring(index + 1);
		v = tmp;
	    }
	    index = v.IndexOf('<');
	    if(index != -1)
	    {
		string tmp = v.Substring(0, index);
		tmp += "&lt";
		tmp += v.Substring(index + 1);
		v = tmp;
	    }
	    index = v.IndexOf('\'');
	    if(index != -1)
	    {
		string tmp = v.Substring(0, index);
		tmp += "&apos;";
		tmp += v.Substring(index + 1);
		v = tmp;
	    }
	    index = v.IndexOf('"');
	    if(index != -1)
	    {
		string tmp = v.Substring(0, index);
		tmp += "&quot;";
		tmp += v.Substring(index + 1);
	    }
	}
	return v;
    }
    
    private Stack _elementStack;
    
    internal bool _se;
    internal bool _text;
    
    private bool _sgml;
    private bool _escape;
}

}
