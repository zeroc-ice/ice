// Copyright (c) ZeroC, Inc.

#include "OutputUtil.h"
#include "FileUtil.h"
#include <cassert>
#include <cstring>

using namespace std;
using namespace IceInternal;

namespace IceInternal
{
    NextLine nl;
    StartBlock sb;
    EndBlock eb;
    StartPar spar;
    EndPar epar;
    Separator sp;
}

// ----------------------------------------------------------------------
// OutputBase
// ----------------------------------------------------------------------

IceInternal::OutputBase::OutputBase()
    : _out(_fout),
      _pos(0),
      _indent(0),
      _indentSize(4),
      _useTab(false),
      _separator(true)
{
}

IceInternal::OutputBase::OutputBase(ostream& os)
    : _out(os),
      _pos(0),
      _indent(0),
      _indentSize(4),
      _useTab(false),
      _separator(true)
{
}

IceInternal::OutputBase::OutputBase(const string& s)
    : _out(_fout),
      _pos(0),
      _indent(0),
      _indentSize(4),
      _useTab(false),
      _separator(true)
{
    open(s);
}

IceInternal::OutputBase::~OutputBase() = default;

void
IceInternal::OutputBase::open(const string& s)
{
    //
    // Remove any existing file first. This prevents file name
    // mismatches on case-insensitive OSs.
    //
    IceInternal::unlink(s);
    _fout.open(IceInternal::streamFilename(s).c_str());
}

void
IceInternal::OutputBase::close()
{
    if (_fout.is_open())
    {
        _fout.close();
    }
}

bool
IceInternal::OutputBase::isOpen()
{
    return _fout.is_open();
}

void
IceInternal::OutputBase::print(const string& s)
{
    size_t len = s.size();
    for (unsigned int i = 0; i < len; ++i)
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
IceInternal::OutputBase::inc()
{
    _indent += _indentSize;
}

void
IceInternal::OutputBase::dec()
{
    assert(_indent >= _indentSize);
    _indent -= _indentSize;
}

void
IceInternal::OutputBase::useCurrentPosAsIndent()
{
    _indentSave.push(_indent);
    _indent = _pos;
}

void
IceInternal::OutputBase::zeroIndent()
{
    _indentSave.push(_indent);
    _indent = 0;
}

void
IceInternal::OutputBase::restoreIndent()
{
    assert(!_indentSave.empty());
    _indent = _indentSave.top();
    _indentSave.pop();
}

int
IceInternal::OutputBase::currIndent()
{
    return _indent;
}

void
IceInternal::OutputBase::setIndent(int indentSize)
{
    _indentSize = indentSize;
}

void
IceInternal::OutputBase::setUseTab(bool useTab)
{
    _useTab = useTab;
}

void
IceInternal::OutputBase::newline()
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
IceInternal::OutputBase::separator()
{
    if (_separator)
    {
        _out << '\n';
    }
}

bool
IceInternal::OutputBase::operator!() const
{
    return !_out;
}

// ----------------------------------------------------------------------
// Output
// ----------------------------------------------------------------------

IceInternal::Output::Output(bool breakBeforeBlock, bool shortEmptyBlock)
    : OutputBase(),
      _blockStart("{"),
      _blockEnd("}"),
      _par(-1),
      _breakBeforeBlock(breakBeforeBlock),
      _shortEmptyBlock(shortEmptyBlock),
      _emptyBlock(false)
{
}

IceInternal::Output::Output(ostream& os, bool breakBeforeBlock, bool shortEmptyBlock)
    : OutputBase(os),
      _blockStart("{"),
      _blockEnd("}"),
      _par(-1),
      _breakBeforeBlock(breakBeforeBlock),
      _shortEmptyBlock(shortEmptyBlock),
      _emptyBlock(false)
{
}

IceInternal::Output::Output(const char* s, bool breakBeforeBlock, bool shortEmptyBlock)
    : OutputBase(s),
      _blockStart("{"),
      _blockEnd("}"),
      _par(-1),
      _breakBeforeBlock(breakBeforeBlock),
      _shortEmptyBlock(shortEmptyBlock),
      _emptyBlock(false)
{
}

void
IceInternal::Output::print(const string& s)
{
    _emptyBlock = false;
    if (_par >= 0)
    {
        if (++_par > 1) // No comma for the first parameter.
        {
            _out << ", ";
        }
    }
    OutputBase::print(s);
}

void
IceInternal::Output::sb()
{
    if (_blockStart.length())
    {
        if (_breakBeforeBlock)
        {
            newline();
        }
        else
        {
            _out << ' ';
        }
        _out << _blockStart;
    }
    ++_pos;
    inc();
    _separator = false;
    _emptyBlock = true;
}

void
IceInternal::Output::eb()
{
    dec();
    if (_emptyBlock && _shortEmptyBlock)
    {
        if (_blockEnd.length())
        {
            _separator = true;
            _out << _blockEnd;
        }
    }
    else
    {
        if (_blockEnd.length())
        {
            newline();
            _out << _blockEnd;
        }
    }
    --_pos;
}

void
IceInternal::Output::spar(string_view s)
{
    _emptyBlock = false;
    _out << s;
    _par = 0;
}

void
IceInternal::Output::epar(string_view s)
{
    _par = -1;
    _out << s;
}

Output&
IceInternal::operator<<(Output& out, ios_base& (*val)(ios_base&))
{
    ostringstream s;
    s << val;
    out.print(s.str());
    return out;
}
