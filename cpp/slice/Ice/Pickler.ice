// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_PICKLER_ICE
#define ICE_PICKLER_ICE

module Ice
{

/**
 *
 * A language-specific input stream type. For C++, a Slice
 * [InputStream] is mapped to a C++ [std::istream].
 *
 **/
native InputStream;

/**
 *
 * A language-specific output stream type. For C++, a Slice
 * [OutputStream] is mapped to a C++ [std::ostream].
 *
 **/
native OutputStream;

/**
 *
 * A simple object serializer. Servants can be written to an
 * [OutputStream] and read from an [InputStream].
 *
 * <note><para>For more sophisticated object persistence, you should
 * use the Freeze module.</para></note>
 *
 * @see InputStream
 * @see OutputStream
 *
 **/
local interface Pickler
{
    /**
     *
     * Write ("pickle") a Servant to an output stream.
     *
     * @param servant The Servant to write.
     *
     * @param out The output stream.
     *
     * @see unpickle
     * @see OutputStream
     *
     **/
    void pickle(Object servant, OutputStream out);

    /**
     *
     * Read ("unpickle") a Servant from an input stream. A suitable
     * Servant Factory for the Servant's type must be installed.
     *
     * @param type The servant's type.
     *
     * @param in The input stream.
     *
     * @return The Servant that has been read from the input stream.
     *
     * @see pickle
     * @see InputStream
     * @see ServantFactory
     *
     **/
    Object unpickle(string type, InputStream in);
};

};

#endif

