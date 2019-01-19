//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

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
