// **********************************************************************
//
// Copyright (c) 2003-2015 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <IceGrid/IceGrid.h>
#include <Pricing.h>

using namespace std;
using namespace Demo;

class PricingClient : public Ice::Application
{
public:

    virtual int run(int, char*[]);
};

int
main(int argc, char* argv[])
{
    PricingClient app;
    return app.main(argc, argv, "config.client");
}

int
PricingClient::run(int argc, char* argv[])
{
    if(argc > 1)
    {
        cerr << appName() << ": too many arguments" << endl;
        return EXIT_FAILURE;
    }

    //
    // Create a proxy to the well-known object with the `pricing'
    // identity.
    //
    PricingEnginePrx pricing = PricingEnginePrx::uncheckedCast(communicator()->stringToProxy("pricing"));
    if(!pricing)
    {
        cerr << argv[0] << ": couldn't find a `::Demo::PricingEngine' object." << endl;
        return EXIT_FAILURE;
    }

    //
    // If no context is set on the default locator (with the
    // Ice.Default.Locator.Context property, see the comments from the
    // `config.client' file in this directory), ask for the preferred
    // currency.
    //
    Ice::Context ctx = communicator()->getDefaultLocator()->ice_getContext();
    if(ctx["currency"].empty())
    {
        cout << "enter your preferred currency (USD, EUR, GBP, INR, AUD, JPY): ";
        string currency;
        cin >> currency;
        
        //
        // Setup a locator proxy with a currency context.
        //
        Ice::LocatorPrx locator = communicator()->getDefaultLocator();
        ctx["currency"] = currency;
        pricing = pricing->ice_locator(locator->ice_context(ctx));
    }
    else
    {
        cout << "Preferred currency configured for the client: " << ctx["currency"] << endl;
    }

    //
    // Get the preferred currencies of the server
    //
    Ice::StringSeq currencies = pricing->getPreferredCurrencies();
    cout << "Preferred currencies of the server: ";
    for(Ice::StringSeq::const_iterator p = currencies.begin(); p != currencies.end(); ++p)
    {
        cout << *p;
        if(p + 1 != currencies.end())
        {
            cout << ", ";
        }
    }
    cout << endl;
    return EXIT_SUCCESS;
}
