//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Diagnostics;
using System.Runtime.Serialization;

namespace IceInternal
{
    internal class Ex
    {
        // UOE = UnknownObjectException as in unknown class. TODO: fix/remove.
        internal static void ThrowUOE(Type expectedType, Ice.AnyClass v)
        {
            // If the object is an unknown sliced object, we didn't find an
            // class factory, in this case raise a NoClassFactoryException
            // instead.
            if (v is Ice.UnknownSlicedClass usv)
            {
                throw new Ice.NoClassFactoryException("", usv.TypeId ?? "");
            }

            string typeId = Ice.TypeExtensions.GetIceTypeId(v.GetType())!;
            string expectedTypeId = Ice.TypeExtensions.GetIceTypeId(expectedType)!;
            throw new Ice.UnexpectedObjectException(
                $"expected element of type `{expectedTypeId}' but received `{typeId}'", typeId, expectedTypeId);
        }

        internal static void ThrowMemoryLimitException(int requested, int maximum)
        {
            throw new Ice.MemoryLimitException(
                $"requested {requested} bytes, maximum allowed is {maximum} bytes (see Ice.MessageSizeMax)");
        }
    }

    public class RetryException : Exception
    {
        internal RetryException(System.Exception ex) => _ex = ex;

        internal System.Exception Get() => _ex;

        private readonly System.Exception _ex;
    }
}
