// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#include <Ice/Config.h>
#include <OutputUtil.h>

using namespace std;
using namespace Slice;

namespace Slice
{

NextLine nl;
StartBlock sb;
EndBlock eb;
Separator sp;

}

// ----------------------------------------------------------------------
// Output
// ----------------------------------------------------------------------

Slice::Output::Output()
    : _pos(0),
      _indent(0),
      _separator(true),
      _blockStart("{"),
      _blockEnd("}"),
      _useTab(true),
      _indentSize(4)
{
}

Slice::Output::Output(const char* s)
    : _pos(0),
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
Slice::Output::open(const char* s)
{
    _out.open(s);
}

void
Slice::Output::print(const char* s)
{
    for (unsigned int i = 0; i < strlen(s); ++i)
    {
    if (s[i] == '\n')
        _pos = 0;
    else
        ++_pos;
    }

    _out << s;
}

void
Slice::Output::inc()
{
    _indent += _indentSize;
    _separator = true;
}

void
Slice::Output::dec()
{
    assert(_indent >= _indentSize);
    _indent -= _indentSize;
    _separator = true;
}

void
Slice::Output::useCurrentPosAsIndent()
{
    _indentSave.push(_indent);
    _indent = _pos;
}

void
Slice::Output::zeroIndent()
{
    _indentSave.push(_indent);
    _indent = 0;
}

void
Slice::Output::restoreIndent()
{
    assert(!_indentSave.empty());
    _indent = _indentSave.top();
    _indentSave.pop();
}

void 
Slice::Output::setBeginBlock(const char *bb)
{
    _blockStart = bb;
}

void 
Slice::Output::setEndBlock(const char *eb)
{
    _blockEnd = eb;
}

void 
Slice::Output::setIndent(int indentSize)
{
    _indentSize = indentSize; 
}

void 
Slice::Output::setUseTab(bool useTab)
{
    _useTab = useTab;
}

void
Slice::Output::nl()
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
Slice::Output::sb()
{
    nl();
    _out << _blockStart;
    ++_pos;
    inc();
    _separator = false;
}

void
Slice::Output::eb()
{
    dec();
    nl();
    _out << _blockEnd;
    --_pos;
}

void
Slice::Output::sp()
{
    if (_separator)
        _out << '\n';
}

bool
Slice::Output::operator!() const
{
    return !_out;
}

Output&
Slice::operator<<(Output& o, const NextLine&)
{
    o.nl();
    return o;
}

Output&
Slice::operator<<(Output& o, const StartBlock&)
{
    o.sb();
    return o;
}

Output&
Slice::operator<<(Output& o, const EndBlock&)
{
    o.eb();
    return o;
}

Output&
Slice::operator<<(Output& o, const Separator&)
{
    o.sp();
    return o;
}
