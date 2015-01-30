// **********************************************************************
//
// Copyright (c) 2003-2015 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef PRICING_I_H
#define PRICING_I_H

#include <Pricing.h>

class PricingI : public Demo::PricingEngine
{
public:

    PricingI(const Ice::StringSeq&);

    virtual Ice::StringSeq getPreferredCurrencies(const Ice::Current&);

private:

	// Required to prevent compiler warnings with MSVC++
	PricingI& operator=(const PricingI&);

    const Ice::StringSeq _currencies;
};

#endif
