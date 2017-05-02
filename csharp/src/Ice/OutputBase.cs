// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

namespace IceUtilInternal
{

using System.Collections.Generic;
using System.IO;
using System.Diagnostics;

public class OutputBase
{
    public
    OutputBase()
    {
        out_ = null;
        pos_ = 0;
        indent_ = 0;
        indentSize_ = 4;
        useTab_ = true;
        indentSave_ = new Stack<int>();
        separator_ = true;
    }

    public
    OutputBase(TextWriter writer)
    {
        out_ = writer;
        pos_ = 0;
        indent_ = 0;
        indentSize_ = 4;
        useTab_ = true;
        indentSave_ = new Stack<int>();
        separator_ = true;
    }

    public
    OutputBase(string s)
    {
        out_ = new StreamWriter(s);
        pos_ = 0;
        indent_ = 0;
        indentSize_ = 4;
        useTab_ = true;
        indentSave_ = new Stack<int>();
        separator_ = true;
    }

    virtual public void
    setIndent(int indentSize)
    {
        indentSize_ = indentSize;
    }

    virtual public void
    setUseTab(bool useTab)
    {
        useTab_ = useTab;
    }

    public virtual void
    open(string s)
    {
        try
        {
            out_ = new StreamWriter(s);
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
                pos_ = 0;
            }
            else
            {
            }
        }

        out_.Write(s);
    }

    public virtual void
    inc()
    {
        indent_ += indentSize_;
    }

    public virtual void
    dec()
    {
        Debug.Assert(indent_ >= indentSize_);
        indent_ -= indentSize_;
    }

    public virtual void
    useCurrentPosAsIndent()
    {
        indentSave_.Push(indent_);
        indent_ = pos_;
    }

    public virtual void
    zeroIndent()
    {
        indentSave_.Push(indent_);
        indent_ = 0;
    }

    public virtual void
    restoreIndent()
    {
        Debug.Assert(indentSave_.Count != 0);
        indent_ = (int)indentSave_.Pop();
    }

    public virtual void
    nl()
    {
        out_.WriteLine();
        pos_ = 0;
        separator_ = true;

        int indent = indent_;

        if(useTab_)
        {
            while(indent >= 8)
            {
                indent -= 8;
                out_.Write('\t');
                pos_ += 8;
            }
        }
        else
        {
            while(indent >= indentSize_)
            {
                indent -= indentSize_;
                out_.Write("    ");
                pos_ += indentSize_;
            }
        }

        while(indent > 0)
        {
            --indent;
            out_.Write(" ");
            ++pos_;
        }

        out_.Flush();
    }

    public virtual void
    sp()
    {
        if(separator_)
        {
            out_.WriteLine();
        }
    }

    public virtual bool
    valid()
    {
        return out_ != null;
    }

    protected internal TextWriter out_;
    protected internal int pos_;
    protected internal int indent_;
    protected internal int indentSize_;
    protected internal Stack<int> indentSave_;
    protected internal bool useTab_;
    protected internal bool separator_;
}

}
