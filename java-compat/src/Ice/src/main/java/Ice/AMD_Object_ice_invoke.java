//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package Ice;

/**
 * Callback interface for Blobject AMD servants.
 * @see BlobjectAsync
 **/
public interface AMD_Object_ice_invoke
{
    /**
     * Indicates to the Ice run time that an operation
     * completed.
     *
     * @param ok <code>true</code> indicates that the operation
     * completed successfully; <code>false</code> indicates that the
     * operation raised a user exception.
     * @param outEncaps The encoded out-parameters for the operation or,
     * if <code>ok</code> is <code>false</code>, the encoded user exception.
     **/
    void ice_response(boolean ok, byte[] outEncaps);

    /**
     * Indicates to the Ice run time that an operation completed
     * with a run-time exception.
     *
     * @param ex The encoded Ice run-time exception. Note that, if <code>ex</code>
     * is a user exception, the caller receives {@link UnknownUserException}.
     * Use {@link #ice_response} to raise user exceptions.
     **/
    void ice_exception(java.lang.Exception ex);
}
