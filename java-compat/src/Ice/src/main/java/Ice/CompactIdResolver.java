// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

package Ice;

/**
 * Applications that make use of compact type IDs to conserve space
 * when marshaling class instances, and also use the streaming API to
 * extract such classes, can intercept the translation between compact
 * type IDs and their corresponding string type IDs by installing an
 * instance of <code>CompactIdResolver</code> in <code>InitializationData</code>.
 **/
public interface CompactIdResolver
{
    /**
     * Translates a compact (integer) ID into its string equivalent.
     *
     * @param id The compact ID.
     * @return A string type ID (such as <code>"::Module::Class"</code>),
     * or an empty string if the compact ID is unknown.
     **/
    String resolve(int id);
}
