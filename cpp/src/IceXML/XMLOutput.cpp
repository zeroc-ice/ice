// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#include <IceXML/XMLOutput.h>

using namespace std;
using namespace IceXML;

namespace IceXML
{

NextLine nl;
StartBlock sb;
EndBlock eb;
Separator sp;

}

// ----------------------------------------------------------------------
// Output
// ----------------------------------------------------------------------

IceXML::Output::Output() :
    _out(_fout),
    _pos(0),
    _indent(0),
    _separator(true),
    _blockStart("{"),
    _blockEnd("}"),
    _useTab(true),
    _indentSize(4)
{
}

IceXML::Output::Output(ostream& os) :
    _out(os),
    _pos(0),
    _indent(0),
    _separator(true),
    _blockStart("{"),
    _blockEnd("}"),
    _useTab(true),
    _indentSize(4)
{
}

IceXML::Output::Output(const char* s) :
    _out(_fout),
    _pos(0),
    _indent(0),
    _separator(true),
    _blockStart("{"),
    _blockEnd("}"),
    _useTab(true),
    _indentSize(4)
{
    open(s);
}

void
IceXML::Output::open(const char* s)
{
    _fout.open(s);
}

void
IceXML::Output::print(const char* s)
{
    for (unsigned int i = 0; i < strlen(s); ++i)
    {
	if (s[i] == '\n')
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
IceXML::Output::inc()
{
    _indent += _indentSize;
}

void
IceXML::Output::dec()
{
    assert(_indent >= _indentSize);
    _indent -= _indentSize;
}

void
IceXML::Output::useCurrentPosAsIndent()
{
    _indentSave.push(_indent);
    _indent = _pos;
}

void
IceXML::Output::zeroIndent()
{
    _indentSave.push(_indent);
    _indent = 0;
}

void
IceXML::Output::restoreIndent()
{
    assert(!_indentSave.empty());
    _indent = _indentSave.top();
    _indentSave.pop();
}

void 
IceXML::Output::setBeginBlock(const char *bb)
{
    _blockStart = bb;
}

void 
IceXML::Output::setEndBlock(const char *eb)
{
    _blockEnd = eb;
}

void 
IceXML::Output::setIndent(int indentSize)
{
    _indentSize = indentSize; 
}

void 
IceXML::Output::setUseTab(bool useTab)
{
    _useTab = useTab;
}

void
IceXML::Output::nl()
{
    _out << '\n';
    _pos = 0;
    _separator = true;

    int indent = _indent;

    if (_useTab)
    {
        while (indent >= 8)
        {
            indent -= 8;
            _out << '\t';
            _pos += 8;
        }
    }
    else
    {
        while (indent >= _indentSize)
        {
            indent -= _indentSize;
            _out << "    ";
            _pos += _indentSize;
        }
    }

    while (indent > 0)
    {
        --indent;
        _out << ' ';
        ++_pos;
    }

    _out.flush();
}

void
IceXML::Output::sb()
{
    if (_blockStart.length())
    {
        nl();
        _out << _blockStart;
    }
    ++_pos;
    inc();
    _separator = false;
}

void
IceXML::Output::eb()
{
    dec();
    if (_blockEnd.length())
    {
        nl();
        _out << _blockEnd;
    }
    --_pos;
}

void
IceXML::Output::sp()
{
    if (_separator)
    {
        _out << '\n';
    }
}

bool
IceXML::Output::operator!() const
{
    return !_out;
}

Output&
IceXML::operator<<(Output& out, ios_base& (*val)(ios_base&))
{
    ostringstream s;
    s << val;
    out.print(s.str().c_str());
    return out;
}
