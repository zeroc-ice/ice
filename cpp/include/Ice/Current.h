// Copyright (c) ZeroC, Inc.

#ifndef ICE_CURRENT_H
#define ICE_CURRENT_H

#include "ConnectionF.h"
#include "Ice/Context.h"
#include "Ice/Identity.h"
#include "Ice/OperationMode.h"
#include "Ice/Version.h"
#include "ObjectAdapterF.h"

namespace Ice
{
    /// Information about an incoming request being dispatched.
    /// \headerfile Ice/Ice.h
    struct Current
    {
        /// The object adapter.
        ObjectAdapterPtr adapter;

        /// Information about the connection over which the current method invocation was received. If the invocation is
        /// direct due to collocation optimization, this value is set to null.
        ConnectionPtr con;

        /// The Ice object identity.
        Identity id;

        /// The facet.
        std::string facet;

        /// The operation name.
        std::string operation;

        /// The mode of the operation.
        OperationMode mode;

        /// The request context, as received from the client.
        Context ctx;

        /// The request id unless oneway (0).
        int requestId;

        /// The encoding version used to encode the input and output parameters.
        EncodingVersion encoding;
    };

    /// A default-initialized Current instance.
    ICE_API extern const Current emptyCurrent;
}

#endif
