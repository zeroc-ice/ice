// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#include <util/PlatformUtils.hpp>
#include <util/XMLString.hpp>
#include <util/XMLUniDefs.hpp>
#include <framework/XMLFormatter.hpp>
#include <util/TranscodingException.hpp>

#include <dom/DOM_DOMException.hpp>
#include <parsers/DOMParser.hpp>
//#include <dom/DOM.hpp>


#include <sax/ErrorHandler.hpp>
#include <sax/SAXParseException.hpp>
#include <dom/DOMString.hpp>

#include <iostream>

using namespace std;
// Global streaming operator for DOMString is defined in DOMPrint.cpp
//extern ostream& operator<<(ostream& target, const DOMString& s);
ostream& operator<< (ostream& target, const DOMString& s)
{
    char *p = s.transcode();
    target << p;
    delete [] p;
    return target;
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
	cerr << "Warning at file \"" << DOMString(toCatch.getSystemId())
	     << "\", line " << toCatch.getLineNumber()
	     << ", column " << toCatch.getColumnNumber()
	     << "\n   Message: " << DOMString(toCatch.getMessage()) << endl;
    }
    
    void
    error(const SAXParseException& toCatch)
    {
	_sawErrors = true;
	cerr << "Error at file \"" << DOMString(toCatch.getSystemId())
	     << "\", line " << toCatch.getLineNumber()
	     << ", column " << toCatch.getColumnNumber()
	     << "\n   Message: " << DOMString(toCatch.getMessage()) << endl;

    }

    void
    fatalError(const SAXParseException& toCatch)
    {
	_sawErrors = true;
	cerr << "Fatal at file \"" << DOMString(toCatch.getSystemId())
	     << "\", line " << toCatch.getLineNumber()
	     << ", column " << toCatch.getColumnNumber()
	     << "\n   Message: " << DOMString(toCatch.getMessage()) << endl;
    }

    void resetErrors()
    {
    }

    bool getSawErrors() const { return _sawErrors; }

    bool    _sawErrors;
};

void
usage(const char* progName)
{
}

int
main(int argc, char** argv)
{
        // Initialize the XML4C2 system
    try
    {
        XMLPlatformUtils::Initialize();
    }

    catch(const XMLException& toCatch)
    {
        cerr << "Error during Xerces-c Initialization.\n"
             << "  Exception message:"
             << DOMString(toCatch.getMessage()) << endl;
        return 1;
    }

    if (argc < 2)
    {
	usage(argv[0]);
	return EXIT_FAILURE;
    }

    //
    //  Create our parser, then attach an error handler to the parser.
    //  The parser will call back to methods of the ErrorHandler if it
    //  discovers errors during the course of parsing the XML document.
    //
    DOMParser* parser = new DOMParser;
    parser->setValidationScheme(DOMParser::Val_Auto);
    parser->setDoNamespaces(true);
    parser->setDoSchema(true);
    parser->setValidationSchemaFullChecking(true);

    DOMTreeErrorReporter *errReporter = new DOMTreeErrorReporter();
    parser->setErrorHandler(errReporter);
    parser->setCreateEntityReferenceNodes(false);
    parser->setToCreateXMLDeclTypeNode(true);

    //
    //  Parse the XML file, catching any XML exceptions that might propogate
    //  out of it.
    //
    bool errorsOccured = false;
    try
    {
        parser->parse(argv[1]);
        int errorCount = parser->getErrorCount();
        if (errorCount > 0)
            errorsOccured = true;
    }

    catch (const XMLException& e)
    {
        cerr << "An error occured during parsing\n   Message: "
             << DOMString(e.getMessage()) << endl;
        errorsOccured = true;
    }
    catch (const DOM_DOMException& e)
    {
       cerr << "A DOM error occured during parsing\n   DOMException code: "
             << e.code << endl;
        errorsOccured = true;
    }

    catch (...)
    {
        cerr << "An error occured during parsing\n " << endl;
        errorsOccured = true;
    }

    return (errorsOccured) ? EXIT_FAILURE : EXIT_SUCCESS;
}
