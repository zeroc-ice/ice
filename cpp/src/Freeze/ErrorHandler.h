#ifndef ERROR_HANDLER_H
#define ERROR_HANDLER_H

#include <sax/ErrorHandler.hpp>
#include <sax/SAXParseException.hpp>

#include <Freeze/Print.h>

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
	::std::cerr << "Warning at file \"" << DOMString(toCatch.getSystemId())
		    << "\", line " << toCatch.getLineNumber()
		    << ", column " << toCatch.getColumnNumber()
		    << "\n   Message: " << DOMString(toCatch.getMessage()) << ::std::endl;
    }
    
    void
    error(const SAXParseException& toCatch)
    {
	_sawErrors = true;
	::std::cerr << "Error at file \"" << DOMString(toCatch.getSystemId())
		    << "\", line " << toCatch.getLineNumber()
		    << ", column " << toCatch.getColumnNumber()
		    << "\n   Message: " << DOMString(toCatch.getMessage()) << ::std::endl;
	
    }

    void
    fatalError(const SAXParseException& toCatch)
    {
	_sawErrors = true;
	::std::cerr << "Fatal at file \"" << DOMString(toCatch.getSystemId())
		    << "\", line " << toCatch.getLineNumber()
		    << ", column " << toCatch.getColumnNumber()
		    << "\n   Message: " << DOMString(toCatch.getMessage()) << ::std::endl;
    }

    void resetErrors()
    {
    }

    bool getSawErrors() const { return _sawErrors; }

    bool    _sawErrors;
};

#endif
