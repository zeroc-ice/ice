// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
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
        elementStack_ = new Stack();
        se_ = false;
        text_ = false;
        sgml_ = false;
        escape_ = false;
    }
    
    public XMLOutput(StreamWriter writer)
        : base(writer)
    {
            elementStack_ = new Stack();
            se_ = false;
            text_ = false;
            sgml_ = false;
            escape_ = false;
    }
    
    public XMLOutput(string s)
        : base(s)
    {
            elementStack_ = new Stack();
            se_ = false;
            text_ = false;
            sgml_ = false;
            escape_ = false;
    }
    
    virtual public void 
    setSGML(bool sgml)
    {
        sgml_ = true;
    }

    public override void
    print(string s)
    {
        if(se_)
        {
            out_.Write(">");
            se_ = false;
        }
        text_ = true;
        
        if(escape_)
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
        if(se_)
        {
            se_ = false;
            out_.Write(">");
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
        if(escape_)
        {
            out_.Write('<');
            out_.Write(escape(element));
        }
        else
        {
            out_.Write('<');
            out_.Write(element);
        }
        se_ = true;
        text_ = false;
        
        int pos = element.IndexOf(' ');
        if (pos == - 1)
        {
            pos = element.IndexOf('\t');
        }
        if (pos == - 1)
        {
            elementStack_.Push(element);
        }
        else
        {
            elementStack_.Push(element.Substring(0, pos - 1));
        }
        
        ++pos_; // TODO: ???
        inc();
        separator_ = false;
        return this;
    }
    
    public virtual XMLOutput
    ee()
    {
        string element = (string)elementStack_.Pop();
        
        dec();
        if(se_)
        {
            //
            // SGML (docbook) doesn't support <foo/>
            //
            if(sgml_)
            {
                out_.Write("></");
                out_.Write(element);
                out_.Write(">");
            }
            else
            {
                out_.Write("/>");
            }
        }
        else
        {
            if(!text_)
            {
                nl();
            }
            out_.Write("</");
            out_.Write(element);
            out_.Write(">");
        }
        --pos_; // TODO: ???
        
        se_ = false;
        text_ = false;
        return this;
    }
    
    public virtual XMLOutput
    attr(string name, string val)
    {
        //
        // Precondition: Attributes can only be attached to elements.
        //
        Debug.Assert(se_);
        out_.Write(" ");
        out_.Write(name);
        out_.Write("=\"");
        out_.Write(escape(val));
        out_.Write("\"");
        return this;
    }
    
    public virtual XMLOutput
    startEscapes()
    {
        escape_ = true;
        return this;
    }
    
    public virtual XMLOutput
    endEscapes()
    {
        escape_ = false;
        return this;
    }
    
    public virtual string
    currentElement()
    {
        if(elementStack_.Count > 0)
        {
            return (string)elementStack_.Peek();
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
    
    private Stack elementStack_;
    
    internal bool se_;
    internal bool text_;
    
    private bool sgml_;
    private bool escape_;
}

}
