#ifndef PRINT_H
#define PRINT_H

#include <ostream>

#include <dom/DOM.hpp>

::std::ostream& operator<<(::std::ostream&, DOM_Node&);
::std::ostream& operator<< (::std::ostream&, const DOMString&);

void printinit();


#endif
