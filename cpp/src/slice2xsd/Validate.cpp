// **********************************************************************
//
// Copyright (c) 2001
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

#include <IceUtil/Config.h>

#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/util/XMLString.hpp>

#include <xercesc/dom/DOMException.hpp>
#include <xercesc/parsers/XercesDOMParser.hpp>

#include <xercesc/sax/ErrorHandler.hpp>
#include <xercesc/sax/SAXParseException.hpp>

#include <iostream>

using namespace std;

//
// Utility to make the usage of xerces easier.
//
static string
toString(const XMLCh* s)
{
    char* t = XMLString::transcode(s);
    string r(t);
    delete[] t;
    return r;
}

class DOMTreeErrorReporter : public ErrorHandler
{
public:
    DOMTreeErrorReporter() :
       _sawErrors(false)
    {
    }

    void
    warning(const SAXParseException& toCatch)
    {
	cerr << "Warning at file \"" << toString(toCatch.getSystemId())
	     << "\", line " << toCatch.getLineNumber()
	     << ", column " << toCatch.getColumnNumber()
	     << "\n   Message: " << toString(toCatch.getMessage()) << endl;
    }
    
    void
    error(const SAXParseException& toCatch)
    {
	_sawErrors = true;
	cerr << "Error at file \"" << toString(toCatch.getSystemId())
	     << "\", line " << toCatch.getLineNumber()
	     << ", column " << toCatch.getColumnNumber()
	     << "\n   Message: " << toString(toCatch.getMessage()) << endl;
    }

    void
    fatalError(const SAXParseException& toCatch)
    {
	_sawErrors = true;
	cerr << "Fatal at file \"" << toString(toCatch.getSystemId())
	     << "\", line " << toCatch.getLineNumber()
	     << ", column " << toCatch.getColumnNumber()
	     << "\n   Message: " << toString(toCatch.getMessage()) << endl;
    }

    void resetErrors()
    {
    }

    bool getSawErrors() const { return _sawErrors; }

    bool _sawErrors;
};

void
usage(const char* n)
{
    cerr << "Usage: " << n << " xml-files...\n";
    cerr <<
	"Options:\n"
	"-h, --help	      Show this message.\n"
	"-v, --version	      Display the Ice version.\n"
	;
}

int
main(int argc, char** argv)
{
    //
    // Initialize the XML4C2 system
    //
    try
    {
        XMLPlatformUtils::Initialize();
    }
    catch(const XMLException& toCatch)
    {
        cerr << "Error during Xerces-c Initialization.\n"
             << "  Exception message:"
             << toString(toCatch.getMessage()) << endl;
        return 1;
    }

    int idx = 1;
    while(idx < argc)
    {
	if(strcmp(argv[idx], "-h") == 0 || strcmp(argv[idx], "--help") == 0)
	{
	    usage(argv[0]);
	    return EXIT_SUCCESS;
	}
	else if(strcmp(argv[idx], "-v") == 0 || strcmp(argv[idx], "--version") == 0)
	{
	    cout << ICE_STRING_VERSION << endl;
	    return EXIT_SUCCESS;
	}
	else if(argv[idx][0] == '-')
	{
	    cerr << argv[0] << ": unknown option `" << argv[idx] << "'" << endl;
	    usage(argv[0]);
	    return EXIT_FAILURE;
	}
	else
	{
	    ++idx;
	}
    }

    if(argc < 2)
    {
	cerr << argv[0] << ": no input file" << endl;
	usage(argv[0]);
	return EXIT_FAILURE;
    }

    bool errorsOccured = false;
    for(idx = 1 ; idx < argc ; ++idx)
    {
        //
        //  Create our parser, then attach an error handler to the parser.
        //  The parser will call back to methods of the ErrorHandler if it
        //  discovers errors during the course of parsing the XML document.
        //
        XercesDOMParser* parser = new XercesDOMParser;
        parser->setValidationScheme(XercesDOMParser::Val_Auto);
        parser->setDoNamespaces(true);
        parser->setDoSchema(true);
        parser->setValidationSchemaFullChecking(true);

        DOMTreeErrorReporter errReporter;
        parser->setErrorHandler(&errReporter);
        parser->setCreateEntityReferenceNodes(false);

        //
        //  Parse the XML file, catching any XML exceptions that might propogate
        //  out of it.
        //
        try
        {
            parser->parse(argv[idx]);
            if(parser->getErrorCount() > 0)
            {
                errorsOccured = true;
            }
        }
        catch(const XMLException& e)
        {
            cerr << "An error occured during parsing\n   Message: " << toString(e.getMessage()) << endl;
            errorsOccured = true;
        }
        catch(const DOMException& e)
        {
            cerr << "A DOM error occured during parsing\n   DOMException code: " << e.code << endl;
            errorsOccured = true;
        }
        catch(...)
        {
            cerr << "An error occured during parsing\n " << endl;
            errorsOccured = true;
        }

        delete parser;
    }

    XMLPlatformUtils::Terminate();

    return (errorsOccured) ? EXIT_FAILURE : EXIT_SUCCESS;
}
