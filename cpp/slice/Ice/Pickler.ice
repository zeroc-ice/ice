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

native InputStream;
native OutputStream;

/**
 *
 * A simple object serializer. Servants can be written to a native
 * ioutput stream type, and read from a native input stream type. For
 * C++, the output stream type is <literal>std::ostream</literal> and
 * the input stream type is <literal>std::istream</literal>.
 *
 * <note><para>For more sophisticated object persistence, you should
 * use the Freeze module.</para></note>
 *
 * @see Communicator::getPickler
 *
 **/
local class Pickler
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
     * @see ServantFactory
     *
     **/
    Object unpickle(string type, InputStream in);
};

};

#endif

