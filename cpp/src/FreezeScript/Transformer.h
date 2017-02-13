// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef FREEZE_SCRIPT_TRANSFORMER_H
#define FREEZE_SCRIPT_TRANSFORMER_H

#include <Ice/Ice.h>
#include <Slice/Parser.h>
#include <Freeze/Connection.h>
#include <FreezeScript/Data.h>

class Db;
class DbTxn;

namespace FreezeScript
{

void
transformDatabase(const Ice::CommunicatorPtr&,
                  const FreezeScript::ObjectFactoryPtr& objectFactory,
                  const Slice::UnitPtr&, const Slice::UnitPtr&,
                  Db*, Db*, DbTxn*, const Freeze::ConnectionPtr&, const std::string&, const std::string&, bool,
                  std::ostream&, bool, std::istream&);

} // End of namespace FreezeScript

#endif
