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
using namespace IceLang;

namespace IceLang
{

NextLine nl;
StartBlock sb;
EndBlock eb;
Separator sp;

}

// ----------------------------------------------------------------------
// Output
// ----------------------------------------------------------------------

IceLang::Output::Output()
    : pos_(0),
      indent_(0),
      indentSave_(-1),
      separator_(true)
{
}

void
IceLang::Output::open(const char* s)
{
    out_.open(s);
}

void
IceLang::Output::print(const char* s)
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
IceLang::Output::inc()
{
    indent_ += 4;
    separator_ = true;
}

void
IceLang::Output::dec()
{
    assert(indent_ >= 4);
    indent_ -= 4;
    separator_ = true;
}

void
IceLang::Output::useCurrentPosAsIndent()
{
    assert(indentSave_ == -1);
    indentSave_ = indent_;
    indent_ = pos_;
}

void
IceLang::Output::zeroIndent()
{
    assert(indentSave_ == -1);
    indentSave_ = indent_;
    indent_ = 0;
}

void
IceLang::Output::restoreIndent()
{
    assert(indentSave_ != -1);
    indent_ = indentSave_;
    indentSave_ = -1;
}

void
IceLang::Output::nl()
{
    out_ << '\n';
    pos_ = 0;
    separator_ = true;

    int indent = indent_;

    while(indent >= 8)
    {
	indent -= 8;
	out_ << "\t";
	pos_ += 8;
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
IceLang::Output::sb()
{
    nl();
    out_ << '{';
    ++pos_;
    inc();
    separator_ = false;
}

void
IceLang::Output::eb()
{
    dec();
    nl();
    out_ << '}';
    --pos_;
}

void
IceLang::Output::sp()
{
    if(separator_)
	out_ << '\n';
}

bool
IceLang::Output::operator!() const
{
    return !out_;
}

Output&
IceLang::operator<<(Output& o, const NextLine&)
{
    o.nl();
    return o;
}

Output&
IceLang::operator<<(Output& o, const StartBlock&)
{
    o.sb();
    return o;
}

Output&
IceLang::operator<<(Output& o, const EndBlock&)
{
    o.eb();
    return o;
}

Output&
IceLang::operator<<(Output& o, const Separator&)
{
    o.sp();
    return o;
}
