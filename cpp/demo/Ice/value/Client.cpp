// **********************************************************************
//
// Copyright (c) 2001
// Mutable Realms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#include <Ice/Ice.h>
#include <Value.h>
#include <ObjectFactory.h>

using namespace std;

int
run(int argc, char* argv[], const Ice::CommunicatorPtr& communicator)
{
    Ice::PropertiesPtr properties = communicator->getProperties();
    const char* refProperty = "Value.Initial";
    std::string ref = properties->getProperty(refProperty);
    if(ref.empty())
    {
	cerr << argv[0] << ": property `" << refProperty << "' not set" << endl;
	return EXIT_FAILURE;
    }

    Ice::ObjectPrx base = communicator->stringToProxy(ref);
    InitialPrx initial = InitialPrx::checkedCast(base);
    if(!initial)
    {
	cerr << argv[0] << ": invalid object reference" << endl;
	return EXIT_FAILURE;
    }

    char c;

    cout << '\n'
	 << "Let's first transfer a simple object, for a class without\n"
	 << "operations, and print its contents. No factory is required\n"
	 << "for this.\n"
	 << "[press enter]\n";
    cin.getline(&c, 1);

    SimplePtr simple = initial->getSimple();
    cout << "==> " << simple->_message << endl;

    cout << '\n'
	 << "Ok, this worked. Now let's try to transfer an object for a class\n"
	 << "with operations, without installing a factory first. This should\n"
	 << "give us a `no factory' exception.\n"
	 << "[press enter]\n";
    cin.getline(&c, 1);

    PrinterPtr printer;
    PrinterPrx printerProxy;
    try
    {
	initial->getPrinter(printer, printerProxy);
    }
    catch(const Ice::NoObjectFactoryException& ex)
    {
	cout << "==> " << ex << endl;
    }

    cout << '\n'
	 << "Yep, that's what we expected. Now let's try again, but with\n"
	 << "installing an appropriate factory first. If successful, we print\n"
	 << "the object's content.\n"
	 << "[press enter]\n";
    cin.getline(&c, 1);

    Ice::ObjectFactoryPtr factory = new ObjectFactory;
    communicator->addObjectFactory(factory, "::Printer");

    initial->getPrinter(printer, printerProxy);
    cout << "==> " << printer->_message << endl;

    cout << '\n'
	 << "Cool, it worked! Let's try calling the printBackwards() method\n"
	 << "on the object we just received locally.\n"
	 << "[press enter]\n";
    cin.getline(&c, 1);

    cout << "==> ";
    printer->printBackwards();

    cout << '\n'
	 << "Now we call the same method, but on the remote object. Watch the\n"
	 << "server's output.\n"
	 << "[press enter]\n";
    cin.getline(&c, 1);

    printerProxy->printBackwards();

    cout << '\n'
	 << "Next, we transfer a derived object from the server as base\n"
	 << "object. Since we didn't install a factory for the derived\n"
	 << "class yet, we will get another `no factory' exception.\n"
	 << "[press enter]\n";
    cin.getline(&c, 1);

    PrinterPtr derivedAsBase;
    try
    {
	derivedAsBase = initial->getDerivedPrinter();
	assert(false);
    }
    catch(const Ice::NoObjectFactoryException& ex)
    {
	cout << "==> " << ex << endl;
    }
    
    cout << '\n'
	 << "Now we install a factory for the derived class, and try again.\n"
	 << "We won't get a `no factory' exception anymore, but since we\n"
	 << "receive the derived object as base object, we need to do a\n"
	 << "dynamic_cast<> to get from the base to the derived object.\n"
	 << "[press enter]\n";
    cin.getline(&c, 1);
    
    communicator->addObjectFactory(factory, "::DerivedPrinter");
    
    derivedAsBase = initial->getDerivedPrinter();
    DerivedPrinterPtr derived = DerivedPrinterPtr::dynamicCast(derivedAsBase);
    assert(derived);

    cout << "==> dynamic_cast<> to derived object succeded" << endl;

    cout << '\n'
	 << "Let's print the message contained in the derived object, and\n"
	 << "call the operation printUppercase() on the derived object\n"
	 << "locally.\n"
	 << "[press enter]\n";
    cin.getline(&c, 1);

    cout << "==> " << derived->_derivedMessage << endl;
    cout << "==> ";
    derived->printUppercase();

    cout << '\n'
	 << "Finally, we try the same again, but instead of returning the\n"
	 << "derived object, we throw an exception containing the derived\n"
	 << "object.\n"
	 << "[press enter]\n";
    cin.getline(&c, 1);

    try
    {
	initial->throwDerivedPrinter();
    }
    catch(const DerivedPrinterException& ex)
    {
	derived = ex.derived;
	assert(derived);
    }

    cout << "==> " << derived->_derivedMessage << endl;
    cout << "==> ";
    derived->printUppercase();

    cout << '\n'
	 << "That's it for this demo. Have fun with Ice!\n";

    return EXIT_SUCCESS;
}

int
main(int argc, char* argv[])
{
    int status;
    Ice::CommunicatorPtr communicator;

    try
    {
	Ice::PropertiesPtr properties = Ice::createProperties(argc, argv);
        properties->load("config");
	communicator = Ice::initializeWithProperties(argc, argv, properties);
	status = run(argc, argv, communicator);
    }
    catch(const Ice::Exception& ex)
    {
	cerr << ex << endl;
	status = EXIT_FAILURE;
    }

    if(communicator)
    {
	try
	{
	    communicator->destroy();
	}
	catch(const Ice::Exception& ex)
	{
	    cerr << ex << endl;
	    status = EXIT_FAILURE;
	}
    }

    return status;
}
