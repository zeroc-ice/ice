// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#include <IceXML/Output.h>

using namespace std;
using namespace IceXML;

namespace IceXML
{

NextLine nl;
//StartBlock sb;
//EndBlock eb;
EndElement ee;
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
    _printed(true),
    _useTab(true),
    _indentSize(4)
{
}

IceXML::Output::Output(ostream& os) :
    _out(os),
    _pos(0),
    _indent(0),
    _separator(true),
    _printed(true),
    _useTab(true),
    _indentSize(4)
{
}

IceXML::Output::Output(const char* s) :
    _out(_fout),
    _pos(0),
    _indent(0),
    _separator(true),
    _printed(true),
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
    if (!_printed)
    {
	_out << '>';
	_printed = true;
    }
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
    if (!_printed)
    {
	_printed = true;
	_out << '>';
    }

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
IceXML::Output::se(const std::string& element)
{
    nl();

    //
    // The output of the '>' character is deferred until either the
    //end-element (in which case a /> is emitted) or until something
    //is displayed.
    //
    _out << '<' << element;

    string::size_type pos = element.find_first_of(" \t");
    if (pos == string::npos)
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
    _printed = false;
}

void
IceXML::Output::ee()
{
    string element = _elementStack.top();
    _elementStack.pop();

    dec();
    if (!_printed)
    {
	_out << "/>";
    }
    else
    {
	nl();
	_out << "</" << element << '>';
    }
    --_pos; // TODO: ???
    _printed = true;
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
