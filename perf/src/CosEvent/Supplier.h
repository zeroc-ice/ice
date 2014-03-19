// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICEPERF_SUPPLIER_H
#define ICEPERF_SUPPLIER_H

#include <orbsvcs/CosEventCommS.h>

class Supplier : public POA_CosEventComm::PushSupplier
{
public:
  int run(int argc, char* argv[]);

  virtual void disconnect_push_supplier(ACE_ENV_SINGLE_ARG_DECL_NOT_USED)
    ACE_THROW_SPEC ((CORBA::SystemException));

private:
};

#endif /* SUPPLIER_H */
