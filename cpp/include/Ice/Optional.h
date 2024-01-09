//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_OPTIONAL_H
#define ICE_OPTIONAL_H

# include <optional>
namespace Ice
{

/**
 * Ice::optional is an alias for std::optional, provided for compatibility with Ice 3.7; new code should use
 * std::optional directly.
 */
template<class T> using optional = std::optional<T>;

using std::operator==;
using std::operator!=;
using std::operator<;
using std::operator<=;
using std::operator>;
using std::operator>=;

/** Creates an optional object. */
using std::make_optional;
/** Exchanges the state of an optional object with another one. */
using std::swap;

/** This type indicates that no value is provided. */
using nullopt_t = std::nullopt_t;

/** An instance of nullopt_t used as a marker value to indicate that no value is provided. */
using std::nullopt;

/** Raised when accessing an optional that doesn't contain a value. */
using bad_optional_access = std::bad_optional_access;

/** This type indicates that an optional value should be constructed in place. */
using in_place_t = std::in_place_t;
/** An instance of in_place_t that indicates that an optional value should be constructed in place. */
using std::in_place;

}

namespace IceUtil
{

/**
 * IceUtil::Optional is an alias for std::optional, provided for compatibility with Ice 3.7 and earlier versions.
 */
template<class T> using Optional = std::optional<T>;

/** An alias for std::nullopt, provided for compatibility with Ice 3.7 and earlier versions. */
constexpr std::nullopt_t None = std::nullopt;

/** An alias for std::make_optional, provided for compatibility with Ice 3.7 and earlier versions. */
template<class T>
constexpr std::optional<std::decay_t<T>> makeOptional(T&& value)
{
    return std::make_optional(value);
}

}

#endif
