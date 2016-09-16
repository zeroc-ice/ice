// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceUtil/OutputUtil.h>
#include <IceUtil/FileUtil.h>
#include <cstring>

using namespace std;
using namespace IceUtil;
using namespace IceUtilInternal;

namespace IceUtilInternal
{

NextLine nl;
StartBlock sb;
EndBlock eb;
StartPar spar;
EndPar epar;
Separator sp;
EndElement ee;
StartEscapes startEscapes;
EndEscapes endEscapes;

}

string
IceUtilInternal::int64ToString(Int64 val)
{
    char buf[64];

#if defined(_WIN32)
    sprintf_s(buf, sizeof(buf), "%I64d", val);
#elif defined(ICE_64)
    sprintf(buf, "%ld", val); // Avoids a format warning from GCC.
#else
    sprintf(buf, "%lld", val);
#endif
    return string(buf);
}

// ----------------------------------------------------------------------
// OutputBase
// ----------------------------------------------------------------------

IceUtilInternal::OutputBase::OutputBase() :
    _out(_fout),
    _pos(0),
    _indent(0),
    _indentSize(4),
    _useTab(false),
    _separator(true)
{
}

IceUtilInternal::OutputBase::OutputBase(ostream& os) :
    _out(os),
    _pos(0),
    _indent(0),
    _indentSize(4),
    _useTab(false),
    _separator(true)
{
}


IceUtilInternal::OutputBase::OutputBase(const string& s) :
    _out(_fout),
    _pos(0),
    _indent(0),
    _indentSize(4),
    _useTab(false),
    _separator(true)
{
    open(s);
}

IceUtilInternal::OutputBase::~OutputBase()
{
}

void
IceUtilInternal::OutputBase::open(const string& s)
{
    //
    // Remove any existing file first. This prevents file name
    // mismatches on case-insensitive OSs.
    //
    IceUtilInternal::unlink(s);
    _fout.open(IceUtilInternal::streamFilename(s).c_str());
}

void
IceUtilInternal::OutputBase::close()
{
    if(_fout.is_open())
    {
        _fout.close();
    }
}

bool
IceUtilInternal::OutputBase::isOpen()
{
    return _fout.is_open();
}

void
IceUtilInternal::OutputBase::print(const string& s)
{
    size_t len = s.size();
    for(unsigned int i = 0; i < len; ++i)
    {
        if(s[i] == '\n')
        {
            _pos = 0;
        }
        else
        {
            ++_pos;
        }
    }
    _out << s;
}

void
IceUtilInternal::OutputBase::inc()
{
    _indent += _indentSize;
}

void
IceUtilInternal::OutputBase::dec()
{
    assert(_indent >= _indentSize);
    _indent -= _indentSize;
}

void
IceUtilInternal::OutputBase::useCurrentPosAsIndent()
{
    _indentSave.push(_indent);
    _indent = _pos;
}

void
IceUtilInternal::OutputBase::zeroIndent()
{
    _indentSave.push(_indent);
    _indent = 0;
}

void
IceUtilInternal::OutputBase::restoreIndent()
{
    assert(!_indentSave.empty());
    _indent = _indentSave.top();
    _indentSave.pop();
}

int
IceUtilInternal::OutputBase::currIndent()
{
    return _indent;
}

void
IceUtilInternal::OutputBase::setIndent(int indentSize)
{
    _indentSize = indentSize;
}

void
IceUtilInternal::OutputBase::setUseTab(bool useTab)
{
    _useTab = useTab;
}

void
IceUtilInternal::OutputBase::newline()
{
    _out << '\n';
    _pos = 0;
    _separator = true;

    int indent = _indent;

    if(_useTab)
    {
        while(indent >= 8)
        {
            indent -= 8;
            _out << '\t';
            _pos += 8;
        }
    }
    else
    {
        while(indent >= _indentSize)
        {
            indent -= _indentSize;
            _out << "    ";
            _pos += _indentSize;
        }
    }

    while(indent > 0)
    {
        --indent;
        _out << ' ';
        ++_pos;
    }

    _out.flush();
}

void
IceUtilInternal::OutputBase::separator()
{
    if(_separator)
    {
        _out << '\n';
    }
}

bool
IceUtilInternal::OutputBase::operator!() const
{
    return !_out;
}

// ----------------------------------------------------------------------
// Output
// ----------------------------------------------------------------------

IceUtilInternal::Output::Output() :
    OutputBase(),
    _blockStart("{"),
    _blockEnd("}"),
    _par(-1)
{
}

IceUtilInternal::Output::Output(ostream& os) :
    OutputBase(os),
    _blockStart("{"),
    _blockEnd("}"),
    _par(-1)
{
}

IceUtilInternal::Output::Output(const char* s) :
    OutputBase(s),
    _blockStart("{"),
    _blockEnd("}"),
    _par(-1)
{
}

void
IceUtilInternal::Output::print(const string& s)
{
    if(_par >= 0)
    {
        if(++_par > 1) // No comma for the first parameter.
        {
            _out << ", ";
        }
    }
    OutputBase::print(s);
}

void
IceUtilInternal::Output::sb()
{
    if(_blockStart.length())
    {
        newline();
        _out << _blockStart;
    }
    ++_pos;
    inc();
    _separator = false;
}

void
IceUtilInternal::Output::eb()
{
    dec();
    if(_blockEnd.length())
    {
        newline();
        _out << _blockEnd;
    }
    --_pos;
}

void
IceUtilInternal::Output::spar()
{
    _out << '(';
    _par = 0;
}

void
IceUtilInternal::Output::epar()
{
    _par = -1;
    _out << ')';
}

Output&
IceUtilInternal::operator<<(Output& out, ios_base& (*val)(ios_base&))
{
    ostringstream s;
    s << val;
    out.print(s.str());
    return out;
}

// ----------------------------------------------------------------------
// XMLOutput
// ----------------------------------------------------------------------

IceUtilInternal::XMLOutput::XMLOutput() :
    OutputBase(),
    _se(false),
    _text(false),
    _escape(false)
{
}

IceUtilInternal::XMLOutput::XMLOutput(ostream& os) :
    OutputBase(os),
    _se(false),
    _text(false),
    _escape(false)
{
}

IceUtilInternal::XMLOutput::XMLOutput(const char* s) :
    OutputBase(s),
    _se(false),
    _text(false),
    _escape(false)
{
}

void
IceUtilInternal::XMLOutput::print(const string& s)
{
    if(_se)
    {
        _out << '>';
        _se = false;
    }
    _text = true;

    if(_escape)
    {
        OutputBase::print(escape(s));
    }
    else
    {
        OutputBase::print(s);
    }
}

void
IceUtilInternal::XMLOutput::newline()
{
    if(_se)
    {
        _se = false;
        _out << '>';
    }
    OutputBase::newline();
}

void
IceUtilInternal::XMLOutput::startElement(const string& element)
{
    newline();

    //
    // If we're not in SGML mode the output of the '>' character is
    // deferred until either the //end-element (in which case a /> is
    // emitted) or until something //is displayed.
    //
    if(_escape)
    {
        _out << '<' << escape(element);
    }
    else
    {
        _out << '<' << element;
    }
    _se = true;
    _text = false;

    string::size_type pos = element.find_first_of(" \t");
    if(pos == string::npos)
    {
        _elementStack.push(element);
    }
    else
    {
        _elementStack.push(element.substr(0, pos));
    }

    ++_pos; // TODO: ???
    inc();
    _separator = false;
}

void
IceUtilInternal::XMLOutput::endElement()
{
    string element = _elementStack.top();
    _elementStack.pop();

    dec();
    if(_se)
    {
        _out << "></" << element << '>';
    }
    else
    {
        if(!_text)
        {
            newline();
        }
        _out << "</" << element << '>';
    }
    --_pos; // TODO: ???

    _se = false;
    _text = false;
}

void
IceUtilInternal::XMLOutput::attr(const string& name, const string& value)
{
    //
    // Precondition: Attributes can only be attached to elements.
    //
    assert(_se);
    _out << " " << name << "=\"" << escape(value) << '"';
}

void
IceUtilInternal::XMLOutput::startEscapes()
{
    _escape = true;
}

void
IceUtilInternal::XMLOutput::endEscapes()
{
    _escape = false;
}

string
IceUtilInternal::XMLOutput::currentElement() const
{
    if(_elementStack.size() > 0)
    {
        return _elementStack.top();
    }
    else
    {
        return string();
    }
}

string
IceUtilInternal::XMLOutput::escape(const string& input) const
{
    string v = input;

    //
    // Find out whether there is a reserved character to avoid
    // conversion if not necessary.
    //
    const string allReserved = "<>'\"&";
    if(v.find_first_of(allReserved) != string::npos)
    {
        //
        // First convert all & to &amp;
        //
        size_t pos = 0;
        while((pos = v.find_first_of('&', pos)) != string::npos)
        {
            v.insert(pos+1, "amp;");
            pos += 4;
        }

        //
        // Next convert remaining reserved characters.
        //
        const string reserved = "<>'\"";
        pos = 0;
        while((pos = v.find_first_of(reserved, pos)) != string::npos)
        {
            string replace;
            switch(v[pos])
            {
            case '>':
                replace = "&gt;";
                break;

            case '<':
                replace = "&lt;";
                break;

            case '\'':
                replace = "&apos;";
                break;

            case '"':
                replace = "&quot;";
                break;

            default:
                assert(false);
            }

            v.erase(pos, 1);
            v.insert(pos, replace);
            pos += replace.size();
        }
    }
    return v;
}

XMLOutput&
IceUtilInternal::operator<<(XMLOutput& out, ios_base& (*val)(ios_base&))
{
    ostringstream s;
    s << val;
    out.print(s.str());
    return out;
}

IceUtilInternal::StartElement::StartElement(const string& name) :
    _name(name)
{
}

const string&
IceUtilInternal::StartElement::getName() const
{
    return _name;
}

IceUtilInternal::Attribute::Attribute(const string& name, const string& value) :
    _name(name),
    _value(value)
{
}

const string&
IceUtilInternal::Attribute::getName() const
{
    return _name;
}

const string&
IceUtilInternal::Attribute::getValue() const
{
    return _value;
}
