// **********************************************************************
//
// Copyright (c) 2003
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

#ifndef TRANSFORM_TRANSFORMER_H
#define TRANSFORM_TRANSFORMER_H

#include <Ice/Ice.h>
#include <Slice/Parser.h>
#include <Transform/Exception.h>

#ifndef TRANSFORM_API
#   ifdef TRANSFORM_API_EXPORTS
#       define TRANSFORM_API ICE_DECLSPEC_EXPORT
#    else
#       define TRANSFORM_API ICE_DECLSPEC_IMPORT
#    endif
#endif

class Db;
class DbTxn;

namespace Transform
{

class TRANSFORM_API Transformer
{
public:

    Transformer(const Ice::CommunicatorPtr&, const Slice::UnitPtr&, const Slice::UnitPtr&, bool, bool);

    void analyze(const std::string&, const std::string&, const std::string&, const std::string&, std::ostream&,
                 Ice::StringSeq&, Ice::StringSeq&);

    void analyze(std::ostream&, Ice::StringSeq&, Ice::StringSeq&);

    void transform(std::istream&, Db*, DbTxn*, Db*, DbTxn*, std::ostream&);

private:

    static void createCoreSliceTypes(const Slice::UnitPtr&);
    static void createEvictorSliceTypes(const Slice::UnitPtr&);
    static Slice::TypePtr findType(const Slice::UnitPtr&, const std::string&, Ice::StringSeq&);

    Ice::CommunicatorPtr _communicator;
    Slice::UnitPtr _old;
    Slice::UnitPtr _new;
    bool _ignoreTypeChanges;
    bool _purgeObjects;
};

} // End of namespace Transform

#endif
