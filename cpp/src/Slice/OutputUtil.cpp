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
    : pos_(0),
      indent_(0),
      separator_(true),
      _blockStart("{"),
      _blockEnd("}"),
      _useTab(true),
      _indentSize(4)
{
}

Slice::Output::Output(const char* s)
    : pos_(0),
      indent_(0),
      separator_(true),
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
    out_.open(s);
}

void
Slice::Output::print(const char* s)
{
    for(unsigned int i = 0; i < strlen(s); ++i)
    {
    if(s[i] == '\n')
        pos_ = 0;
    else
        ++pos_;
    }

    out_ << s;
}

void
Slice::Output::inc()
{
    indent_ += _indentSize;
    separator_ = true;
}

void
Slice::Output::dec()
{
    assert(indent_ >= _indentSize);
    indent_ -= _indentSize;
    separator_ = true;
}

void
Slice::Output::useCurrentPosAsIndent()
{
    indentSave_.push(indent_);
    indent_ = pos_;
}

void
Slice::Output::zeroIndent()
{
    indentSave_.push(indent_);
    indent_ = 0;
}

void
Slice::Output::restoreIndent()
{
    assert(!indentSave_.empty());
    indent_ = indentSave_.top();
    indentSave_.pop();
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
    out_ << '\n';
    pos_ = 0;
    separator_ = true;

    int indent = indent_;

    if (_useTab)
    {
        while(indent >= 8)
        {
            indent -= 8;
            out_ << '\t';
            pos_ += 8;
        }
    }
    else
    {
        while(indent >= _indentSize)
        {
            indent -= _indentSize;
            out_ << "    ";
            pos_ += _indentSize;
        }
    }

    while(indent > 0)
    {
        --indent;
        out_ << ' ';
        ++pos_;
    }

    out_.flush();
}

void
Slice::Output::sb()
{
    nl();
    out_ << _blockStart;
    ++pos_;
    inc();
    separator_ = false;
}

void
Slice::Output::eb()
{
    dec();
    nl();
    out_ << _blockEnd;
    --pos_;
}

void
Slice::Output::sp()
{
    if(separator_)
        out_ << '\n';
}

bool
Slice::Output::operator!() const
{
    return !out_;
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
