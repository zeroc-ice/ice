// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#include <IceUtil/OutputUtil.h>

using namespace std;
using namespace IceUtil;

namespace IceUtil
{

NextLine nl;
StartBlock sb;
EndBlock eb;
Separator sp;
EndElement ee;

}

// ----------------------------------------------------------------------
// OutputBase
// ----------------------------------------------------------------------

IceUtil::OutputBase::OutputBase() :
    _out(_fout),
    _pos(0),
    _indent(0),
    _indentSize(4),
    _useTab(true),
    _separator(true)
{
}

IceUtil::OutputBase::OutputBase(ostream& os) :
    _out(os),
    _pos(0),
    _indent(0),
    _indentSize(4),
    _useTab(true),
    _separator(true)
{
}


IceUtil::OutputBase::OutputBase(const char* s) :
    _out(_fout),
    _pos(0),
    _indent(0),
    _indentSize(4),
    _useTab(true),
    _separator(true)
{
    open(s);
}

IceUtil::OutputBase::~OutputBase()
{
}

void
IceUtil::OutputBase::open(const char* s)
{
    _fout.open(s);
}

void
IceUtil::OutputBase::print(const char* s)
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
IceUtil::OutputBase::inc()
{
    _indent += _indentSize;
}

void
IceUtil::OutputBase::dec()
{
    assert(_indent >= _indentSize);
    _indent -= _indentSize;
}

void
IceUtil::OutputBase::useCurrentPosAsIndent()
{
    _indentSave.push(_indent);
    _indent = _pos;
}

void
IceUtil::OutputBase::zeroIndent()
{
    _indentSave.push(_indent);
    _indent = 0;
}

void
IceUtil::OutputBase::restoreIndent()
{
    assert(!_indentSave.empty());
    _indent = _indentSave.top();
    _indentSave.pop();
}

void 
IceUtil::OutputBase::setIndent(int indentSize)
{
    _indentSize = indentSize; 
}

void 
IceUtil::OutputBase::setUseTab(bool useTab)
{
    _useTab = useTab;
}

void
IceUtil::OutputBase::nl()
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
IceUtil::OutputBase::sp()
{
    if (_separator)
    {
        _out << '\n';
    }
}

bool
IceUtil::OutputBase::operator!() const
{
    return !_out;
}

// ----------------------------------------------------------------------
// Output
// ----------------------------------------------------------------------

IceUtil::Output::Output() :
    OutputBase(),
    _blockStart("{"),
    _blockEnd("}")
{
}

IceUtil::Output::Output(ostream& os) :
    OutputBase(os),
    _blockStart("{"),
    _blockEnd("}")
{
}

IceUtil::Output::Output(const char* s) :
    OutputBase(s),
    _blockStart("{"),
    _blockEnd("}")
{
}

void 
IceUtil::Output::setBeginBlock(const char *bb)
{
    _blockStart = bb;
}

void 
IceUtil::Output::setEndBlock(const char *eb)
{
    _blockEnd = eb;
}

void
IceUtil::Output::sb()
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
IceUtil::Output::eb()
{
    dec();
    if (_blockEnd.length())
    {
        nl();
        _out << _blockEnd;
    }
    --_pos;
}

Output&
IceUtil::operator<<(Output& out, ios_base& (*val)(ios_base&))
{
    ostringstream s;
    s << val;
    out.print(s.str().c_str());
    return out;
}

// ----------------------------------------------------------------------
// XMLOutput
// ----------------------------------------------------------------------

IceUtil::XMLOutput::XMLOutput() :
    OutputBase(),
    _printed(true),
    _sgml(false)
{
}

IceUtil::XMLOutput::XMLOutput(ostream& os) :
    OutputBase(os),
    _printed(true),
    _sgml(false)
{
}

IceUtil::XMLOutput::XMLOutput(const char* s) :
    OutputBase(s),
    _printed(true),
    _sgml(false)
{
}

void
IceUtil::XMLOutput::setSGML(bool sgml)
{
    _sgml = true;
}

void
IceUtil::XMLOutput::print(const char* s)
{
    if (!_printed)
    {
	_out << '>';
	_printed = true;
    }
    OutputBase::print(s);
}

void
IceUtil::XMLOutput::nl()
{
    if (!_printed)
    {
	_printed = true;
	_out << '>';
    }
    OutputBase::nl();
}

void
IceUtil::XMLOutput::se(const std::string& element)
{
    nl();

    //
    // If we're not in SGML mode the output of the '>' character is
    // deferred until either the //end-element (in which case a /> is
    // emitted) or until something //is displayed.
    //
    _out << '<' << element;
    if (_sgml)
    {
	_out << '>';
    }
    else
    {
	_printed = false;
    }


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
}

void
IceUtil::XMLOutput::ee()
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

XMLOutput&
IceUtil::operator<<(XMLOutput& out, ios_base& (*val)(ios_base&))
{
    ostringstream s;
    s << val;
    out.print(s.str().c_str());
    return out;
}
