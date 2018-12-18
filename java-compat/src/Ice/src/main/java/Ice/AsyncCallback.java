// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package Ice;

/**
 * An application can optionally supply an instance of this class in an
 * asynchronous invocation. The application must create a subclass and
 * implement the completed method.
 *
 * @deprecated This class is deprecated, use Ice.Callback instead.
 **/
@Deprecated
public abstract class AsyncCallback extends Callback
{
}
