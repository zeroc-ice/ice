// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef SLICE_XSD_VISITOR_H
#define SLICE_XSD_VISITOR_H

#include <Slice/Parser.h>
#include <IceUtil/OutputUtil.h>
#include <stack>

namespace Slice
{

class XsdVisitor : public ::IceUtil::noncopyable, public ParserVisitor
{
public:

    XsdVisitor();
    XsdVisitor(::std::ostream&);

    void emitElements(bool);

    virtual bool visitClassDefStart(const ClassDefPtr&);
    virtual bool visitExceptionStart(const ExceptionPtr&);
    virtual bool visitStructStart(const StructPtr&);
    virtual void visitOperation(const OperationPtr&);
    virtual void visitEnum(const EnumPtr&);
    virtual void visitSequence(const SequencePtr&);
    virtual void visitDictionary(const DictionaryPtr&);

private:

    void annotate(const ::std::string&);
    void emitElement(const DataMemberPtr&);
    void emitElement(const TypeString&);

    std::string containedToId(const ContainedPtr&);

protected:

    std::string toString(const SyntaxTreeBasePtr&);

    ::IceUtil::XMLOutput O;
    bool _emitElements;
};

}

#endif
