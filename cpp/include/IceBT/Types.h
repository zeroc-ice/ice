//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef __IceBT_Types_h__
#define __IceBT_Types_h__

#include <IceUtil/PushDisableWarnings.h>
#include <Ice/ProxyF.h>
#include <Ice/ObjectF.h>
#include <Ice/ValueF.h>
#include <Ice/Exception.h>
#include <Ice/StreamHelpers.h>
#include <Ice/Comparable.h>
#include <optional>
#include <Ice/ExceptionHelpers.h>
#include <IceUtil/UndefSysMacros.h>

#ifndef ICEBT_API
#    if defined(ICE_STATIC_LIBS)
#        define ICEBT_API /**/
#    elif defined(ICEBT_API_EXPORTS)
#        define ICEBT_API ICE_DECLSPEC_EXPORT
#    else
#        define ICEBT_API ICE_DECLSPEC_IMPORT
#    endif
#endif

namespace IceBT
{
    /**
     * Indicates a failure in the Bluetooth plug-in.
     * \headerfile IceBT/IceBT.h
     */
    class ICE_CLASS(ICEBT_API) BluetoothException
        : public ::Ice::LocalExceptionHelper<BluetoothException, ::Ice::LocalException>
    {
    public:
        ICE_MEMBER(ICEBT_API) virtual ~BluetoothException();

        BluetoothException(const BluetoothException&) = default;

        /**
         * The file and line number are required for all local exceptions.
         * @param file The file name in which the exception was raised, typically __FILE__.
         * @param line The line number at which the exception was raised, typically __LINE__.
         */
        BluetoothException(const char* file, int line)
            : ::Ice::LocalExceptionHelper<BluetoothException, ::Ice::LocalException>(file, line)
        {
        }

        /**
         * One-shot constructor to initialize all data members.
         * The file and line number are required for all local exceptions.
         * @param file The file name in which the exception was raised, typically __FILE__.
         * @param line The line number at which the exception was raised, typically __LINE__.
         * @param reason Provides more information about the failure.
         */
        BluetoothException(const char* file, int line, const ::std::string& reason)
            : ::Ice::LocalExceptionHelper<BluetoothException, ::Ice::LocalException>(file, line),
              reason(reason)
        {
        }

        /**
         * Obtains a tuple containing all of the exception's data members.
         * @return The data members in a tuple.
         */
        std::tuple<const ::std::string&> ice_tuple() const { return std::tie(reason); }

        /**
         * Obtains the Slice type ID of this exception.
         * @return The fully-scoped type ID.
         */
        ICE_MEMBER(ICEBT_API) static ::std::string_view ice_staticId();
        /**
         * Prints this exception to the given stream.
         * @param stream The target stream.
         */
        ICE_MEMBER(ICEBT_API) virtual void ice_print(::std::ostream& stream) const override;

        /**
         * Provides more information about the failure.
         */
        ::std::string reason;
    };
}

#include <IceUtil/PopDisableWarnings.h>
#endif
