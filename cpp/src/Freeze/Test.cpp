#include <util/PlatformUtils.hpp>
#include <util/XMLString.hpp>
#include <util/XMLUniDefs.hpp>
#include <framework/XMLFormatter.hpp>
#include <util/TranscodingException.hpp>

#include <dom/DOMString.hpp>

#include <dom/DOM.hpp>
#include <parsers/DOMParser.hpp>
#include <sax/SAXParseException.hpp>

#include <Freeze/Transform.h>
#include <Freeze/Print.h>

#include <iostream>
#include <string>

#include <Freeze/ErrorHandler.h>

using namespace std;

int
main(int argc, char** argv)
{
    if (argc < 3)
    {
	cerr << "Usage: " << argv[0] << " from to" << endl;
	return EXIT_FAILURE;
    }

    string from(argv[1]);
    string to(argv[2]);

    string fromschema = from + ".xsd";
    DOM_Document fromdoc;
    string frominstance = from + ".xml";
    DOM_Document fromschemadoc;

    string toschema = to + ".xsd";
    DOM_Document toschemadoc;
    string toinstance = to + ".xml";

    //DOM_Document iceschemadoc;

    //
    // Read each document.
    //

    try
    {
	XMLPlatformUtils::Initialize();
    }
    catch (const XMLException& toCatch)
    {
        cout << "Error during initialization! :\n"
             << DOMString(toCatch.getMessage()) << "\n";
        return EXIT_FAILURE;
    }
    printinit(); // TODO: garbage

    DOMParser parser;
    parser.setValidationScheme(DOMParser::Val_Auto);    // optional.
    parser.setDoNamespaces(true);    // optional

    DOMTreeErrorReporter *errReporter = new DOMTreeErrorReporter();
    parser.setErrorHandler(errReporter);

    try
    {
        parser.parse(fromschema.c_str());
	fromschemadoc = parser.getDocument();

	parser.parse(frominstance.c_str());
	fromdoc = parser.getDocument();

	parser.parse(toschema.c_str());
	toschemadoc = parser.getDocument();
    }
    catch (const XMLException& toCatch)
    {
        cout << "Exception message is: \n"
             << DOMString(toCatch.getMessage()) << "\n" ;
        return EXIT_FAILURE;
    }
    catch (const SAXParseException& toCatch)
    {
        cout << "Exception message is: \n"
             << DOMString(toCatch.getMessage()) << "\n" ;
        return EXIT_FAILURE;
    }
    catch (...)
    {
        cout << "Unexpected Exception \n" ;
        return EXIT_FAILURE;
    }


    try
    {
	Transformer transformer(fromschemadoc, toschemadoc);

	transformer.transform(cout, fromdoc);
	cout << endl;
    }
    catch(const DOM_DOMException& ex)
    {
	cerr << "DOM_DOMException: " << ex.code << endl;
    }
    catch(const IllegalTransform& ex)
    {
	cerr << ex << endl;
    }
    catch(const IncompatibleSchema& ex)
    {
	cerr << ex << endl;
    }
    catch(const InvalidSchema& ex)
    {
	cerr << ex << endl;
    }
    catch(const SchemaViolation& ex)
    {
	cerr << ex << endl;
    }


    return 0;
}
