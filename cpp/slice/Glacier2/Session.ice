// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef GLACIER2_SESSION_ICE
#define GLACIER2_SESSION_ICE

#include <Ice/BuiltinSequences.ice>
#include <Ice/Identity.ice>
#include <Glacier2/SSLInfo.ice>

module Glacier2
{

/**
 *
 * This exception is raised if an attempt to create a new session
 * failed.
 *
 * @see Router::createSession
 * @see SessionManager::createSession
 *
 **/
exception CannotCreateSessionException
{
    /**
     *
     * The reason why session creation has failed.
     *
     **/
    string reason;
};

/**
 *
 * A client-visible session object, which is tied to the lifecycle of
 * a [Router].
 *
 * @see Router
 * @see SessionManager
 *
 **/
interface Session
{
    /**
     *
     * Destroy the session. This is called automatically when the
     * [Router] is destroyed.
     *
     **/
    void destroy();
};

exception DuplicateStringConstraintsException
{
    Ice::StringSeq duplicates;
};

exception StringConstraintsDoNotExistException
{
    Ice::StringSeq missing;
};

/**
 *
 * A filter object that can be applied to [Session]. Accessible through
 * the session's [SessionControl] object.
 *
 * @see SessionControl
 *
 **/
interface StringFilterManager
{
    /**
     *
     * Add allowable strings to the filter.
     *
     **/
    idempotent void addAccept(Ice::StringSeq additions)
    	throws DuplicateStringConstraintsException;

    /**
     *
     * Remove allowable strings the filter.
     *
     **/
    idempotent void removeAccept(Ice::StringSeq deletions)
	throws StringConstraintsDoNotExistException;

    /**
     *
     * Replace the set of allowable strings in the filter.
     *
     **/
    idempotent void setAccept(Ice::StringSeq acceptStrings)
    	throws DuplicateStringConstraintsException;
    
    /**
     *
     * Obtain the filter's acceptance criteria.
     *
     * @return A sequence of strings that the filter will permit.
     *
     **/
    nonmutating Ice::StringSeq getAccept();

    /**
     *
     * Add prohibited strings to the filter.
     *
     **/
    idempotent void addReject(Ice::StringSeq additions)
    	throws DuplicateStringConstraintsException;

    /**
     *
     * Remove prohibited strings from the filter.
     *
     **/
    idempotent void removeReject(Ice::StringSeq deletions)
	throws StringConstraintsDoNotExistException;

    /**
     *
     * Replace the set of prohibited srings in the filter.
     *
     **/
    idempotent void setReject(Ice::StringSeq rejectStrings)
    	throws DuplicateStringConstraintsException;

    /**
     *
     * Obtain a filter's rejection criteria.
     *
     * @return A sequence of strings that the filter will reject.
     *
     **/
    nonmutating Ice::StringSeq getReject()
	throws StringConstraintsDoNotExistException;

    /**
     *
     * Query the filter's accept override flag. If accept overrides is
     * enabled, the filter is by default allows anything in except those
     * items that match reject criteria. If an item does match the
     * rejection criteria, it may be overridden by a acceptance
     * criteria. If this flag is unset, then the filter rejects
     * everything by default except those items that match acceptance
     * criteria. If an item does match acceptance criteria it may be
     * overridden by rejection criteria.
     *
     * @return True if accept override is set, otherwise.
     *
     **/
    nonmutating bool getAcceptOverride();

    /**
     *
     * Sets the accept override flag. See [getAcceptOverride].
     *
     **/
    idempotent void setAcceptOverride(bool value);
     
};

exception DuplicateIdentityConstraintsException
{
    Ice::IdentitySeq duplicates;
};

exception IdentityConstraintsDoNotExistException
{
    Ice::IdentitySeq missing;
};

interface IdentityFilterManager
{
    /**
     *
     * Add allowable strings to the filter.
     *
     **/
    idempotent void addAccept(Ice::IdentitySeq additions)
	throws DuplicateIdentityConstraintsException;

    /**
     *
     * Remove allowable strings the filter.
     *
     **/
    idempotent void removeAccept(Ice::IdentitySeq deletions)
    	throws IdentityConstraintsDoNotExistException;

    /**
     *
     * Replace the set of allowable strings in the filter.
     *
     **/
    idempotent void setAccept(Ice::IdentitySeq acceptStrings)
	throws DuplicateIdentityConstraintsException;
    
    /**
     *
     * Obtain the filter's acceptance criteria.
     *
     * @return A sequence of strings that the filter will permit.
     *
     **/
    nonmutating Ice::IdentitySeq getAccept();

    /**
     *
     * Add prohibited strings to the filter.
     *
     **/
    idempotent void addReject(Ice::IdentitySeq additions)
	throws DuplicateIdentityConstraintsException;

    /**
     *
     * Remove prohibited strings from the filter.
     *
     **/
    idempotent void removeReject(Ice::IdentitySeq deletions)
    	throws IdentityConstraintsDoNotExistException;

    /**
     *
     * Replace the set of prohibited srings in the filter.
     *
     **/
    idempotent void setReject(Ice::IdentitySeq rejectStrings)
	throws DuplicateIdentityConstraintsException;

    /**
     *
     * Obtain a filter's rejection criteria.
     *
     * @return A sequence of strings that the filter will reject.
     *
     **/
    nonmutating Ice::IdentitySeq getReject();

    /**
     *
     * Query the filter's accept override flag. If accept overrides is
     * enabled, the filter is by default allows anything in except those
     * items that match reject criteria. If an item does match the
     * rejection criteria, it may be overridden by a acceptance
     * criteria. If this flag is unset, then the filter rejects
     * everything by default except those items that match acceptance
     * criteria. If an item does match acceptance criteria it may be
     * overridden by rejection criteria.
     *
     * @return True if accept override is set, otherwise.
     *
     **/
    nonmutating bool getAcceptOverride();

    /**
     *
     * Sets the accept override flag. See [getAcceptOverride].
     *
     **/
    idempotent void setAcceptOverride(bool value);
};

/**
 *
 * A administrative session control object, which is tied to the
 * lifecycle of a [Session].
 *
 * @see Session
 *
 **/
interface SessionControl
{
    /**
     *
     * Obtain a proxy for the category filter.
     *
     * @return A proxy for the category filter object.
     *
     **/
    StringFilterManager* categoryFilter();

    /**
     *
     * Obtain a proxy for the adapter id  filter.
     *
     * @return A proxy for the adapter id filter object.
     *
     **/
    StringFilterManager* adapterIdFilter();

    /**    
     *
     * Obtain a proxy for the identity filter.
     *
     * @return A proxy for the identity filter object.
     *
     **/
    IdentityFilterManager* identityFilter();
    
    /**
     *
     * Destroy the associated session.
     *
     **/
    void destroy();
};

/**
 *
 * The session manager for username/password authenticated users which
 * is responsible for managing [Session] objects. New session objects
 * are created by the [Router] object calling on an
 * application-provided session manager. If no session manager is
 * provided by the application, no client-visible sessions are passed
 * to the client.
 *
 * @see Router
 * @see Session
 *
 **/
interface SessionManager
{
    /**
     *
     * Create a new session.
     *
     * @param userId The user id for the session.
     *
     * @param control A proxy to the session control object.
     *
     * @return A proxy to the newly created session.
     *
     * @throws CannotCreateSessionException Raised if the session
     * cannot be created.
     *
     **/
    Session* create(string userId, SessionControl* control)
	throws CannotCreateSessionException;
};

/**
 *
 * The session manager for SSL authenticated users which is
 * responsible for managing [Session] objects. New session objects are
 * created by the [Router] object calling on an application-provided
 * session manager. If no session manager is provided by the
 * application, no client-visible sessions are passed to the client.
 *
 * @see Router
 * @see Session
 *
 **/
interface SSLSessionManager
{
    /**
     *
     * Create a new session.
     *
     * @param info The SSL info.
     *
     * @param control A proxy to the session control object.
     *
     * @return A proxy to the newly created session.
     *
     * @throws CannotCreateSessionException Raised if the session
     * cannot be created.
     *
     **/
    Session* create(SSLInfo info, SessionControl* control)
	throws CannotCreateSessionException;
};

};

#endif
