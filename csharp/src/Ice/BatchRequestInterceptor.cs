// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

namespace Ice
{
    public interface BatchRequest
    {
        /// <summary>
        /// Confirms the queuing of the batch request.
        /// </summary>
        void enqueue();

        /// <summary>
        /// Get the marshalled size of the request.
        /// </summary>
        /// <returns>The request size.</returns>
        int getSize();

        /// <summary>
        /// Get the name of the operation
        /// </summary>
        /// <returns>The request operation.</returns>
        string getOperation();

        /// <summary>
        /// The proxy used to invoke the batch request.
        /// </summary>
        /// <returns>The request proxy.</returns>
        ObjectPrx getProxy();
    }
}
