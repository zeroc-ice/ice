// **********************************************************************
//
// Copyright (c) 2004
// ZeroC, Inc.
// Billerica, MA, USA
//
// All Rights Reserved.
//
// Ice is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License version 2 as published by
// the Free Software Foundation.
//
// **********************************************************************

#ifndef FREEZE_SCRIPT_TRANSFORMER_H
#define FREEZE_SCRIPT_TRANSFORMER_H

#include <Ice/Ice.h>
#include <Slice/Parser.h>
#include <FreezeScript/Exception.h>

class Db;
class DbTxn;

namespace FreezeScript
{

class Transformer
{
public:

    Transformer(const Ice::CommunicatorPtr&, const Slice::UnitPtr&, const Slice::UnitPtr&, bool, bool);

    void analyze(const std::string&, const std::string&, const std::string&, const std::string&, std::ostream&,
                 Ice::StringSeq&, Ice::StringSeq&);

    void analyze(std::ostream&, Ice::StringSeq&, Ice::StringSeq&);

    void transform(std::istream&, Db*, DbTxn*, Db*, DbTxn*, std::ostream&, bool);

private:

    static Slice::TypePtr findType(const Slice::UnitPtr&, const std::string&, Ice::StringSeq&);

    Ice::CommunicatorPtr _communicator;
    Slice::UnitPtr _old;
    Slice::UnitPtr _new;
    bool _ignoreTypeChanges;
    bool _purgeObjects;
};

} // End of namespace FreezeScript

#endif
