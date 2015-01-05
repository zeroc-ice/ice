// **********************************************************************
//
// Copyright (c) 2003-2015 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <PricingI.h>

using namespace std;

PricingI::PricingI(const Ice::StringSeq& currencies) : _currencies(currencies)
{
}

Ice::StringSeq
PricingI::getPreferredCurrencies(const Ice::Current&)
{
    return _currencies;
}
