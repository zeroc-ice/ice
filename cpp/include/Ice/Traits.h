
#ifndef ICE_TRAITS_H
#define ICE_TRAITS_H

#ifdef ICE_CPP11_MAPPING

#include <type_traits>

namespace Ice
{

class ObjectPrx;
class Value;

template<class T> using IsProxy = ::std::is_base_of<::Ice::ObjectPrx, T>;
template<class T> using IsValue = ::std::is_base_of<::Ice::Value, T>;

}
#endif

#endif
