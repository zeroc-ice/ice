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

public class OutputBase
{
    public
    OutputBase()
    {
	_out = null;
	_pos = 0;
	_indent = 0;
	_indentSize = 4;
	_useTab = true;
	_indentSave = new Stack();
	_separator = true;
    }
    
    public
    OutputBase(TextWriter writer)
    {
	_out = writer;
	_pos = 0;
	_indent = 0;
	_indentSize = 4;
	_useTab = true;
	_indentSave = new Stack();
	_separator = true;
    }
    
    public
    OutputBase(string s)
    {
	_out = new StreamWriter(s);
	_pos = 0;
	_indent = 0;
	_indentSize = 4;
	_useTab = true;
	_indentSave = new Stack();
	_separator = true;
    }

    virtual public void
    setIndent(int indentSize)
    {
	_indentSize = indentSize;
    }
    
    virtual public void
    setUseTab(bool useTab)
    {
	_useTab = useTab;
    }    
    
    public virtual void
    open(string s)
    {
	try
	{
	    _out = new StreamWriter(s);
	}
	catch(IOException)
	{
	}
    }
    
    public virtual void
    print(string s)
    {
	char[] arr = s.ToCharArray();
	for(int i = 0; i < arr.Length; i++)
	{
	    if(arr[i] == '\n')
	    {
		_pos = 0;
	    }
	    else
	    {
	    }
	}
	
	_out.Write(s);
    }
    
    public virtual void
    inc()
    {
	_indent += _indentSize;
    }
    
    public virtual void
    dec()
    {
	Debug.Assert(_indent >= _indentSize);
	_indent -= _indentSize;
    }
    
    public virtual void
    useCurrentPosAsIndent()
    {
	_indentSave.Push(_indent);
	_indent = _pos;
    }
    
    public virtual void
    zeroIndent()
    {
	_indentSave.Push(_indent);
	_indent = 0;
    }
    
    public virtual void
    restoreIndent()
    {
	Debug.Assert(_indentSave.Count != 0);
	_indent = (int)_indentSave.Pop();
    }
    
    public virtual void
    nl()
    {
	_out.WriteLine();
	_pos = 0;
	_separator = true;
	
	int indent = _indent;
	
	if(_useTab)
	{
	    while(indent >= 8)
	    {
		indent -= 8;
		_out.Write('\t');
		_pos += 8;
	    }
	}
	else
	{
	    while(indent >= _indentSize)
	    {
		indent -= _indentSize;
		_out.Write("    ");
		_pos += _indentSize;
	    }
	}
	
	while(indent > 0)
	{
	    --indent;
	    _out.Write(" ");
	    ++_pos;
	}
	
	_out.Flush();
    }
    
    public virtual void
    sp()
    {
	if(_separator)
	{
	    _out.WriteLine();
	}
    }
    
    public virtual bool
    valid()
    {
	return _out != null;
    }
    
    protected internal TextWriter _out;
    protected internal int _pos;
    protected internal int _indent;
    protected internal int _indentSize;
    protected internal Stack _indentSave;
    protected internal bool _useTab;
    protected internal bool _separator;
}

}
