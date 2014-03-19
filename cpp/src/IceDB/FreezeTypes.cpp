// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceDB/FreezeTypes.h>

using namespace FreezeDB;
using namespace std;

DatabaseException::DatabaseException(const char* file, int line, const Freeze::DatabaseException& ex) :
    IceDB::DatabaseException(file, line),
    message(ex.message)
{
}

DatabaseException::~DatabaseException() throw()
{
}

void
DatabaseException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\n" << message;
}

DatabaseException*
DatabaseException::ice_clone() const
{
    return new DatabaseException(*this);
}

void
DatabaseException::ice_throw() const
{
    throw *this;
}

DeadlockException::DeadlockException(const char* file, int line, const Freeze::DatabaseException& ex) :
    IceDB::DeadlockException(file, line),
    message(ex.message)
{
}

DeadlockException::~DeadlockException() throw()
{
}

void
DeadlockException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\n" << message;
}

DeadlockException*
DeadlockException::ice_clone() const
{
    return new DeadlockException(*this);
}

void
DeadlockException::ice_throw() const
{
    throw *this;
}

NotFoundException::NotFoundException(const char* file, int line) :
    IceDB::NotFoundException(file, line)
{
}

NotFoundException::~NotFoundException() throw()
{
}

NotFoundException*
NotFoundException::ice_clone() const
{
    return new NotFoundException(*this);
}

void
NotFoundException::ice_throw() const
{
    throw *this;
}

void
FreezeDB::throwDatabaseException(const char* file, int line, const Freeze::DatabaseException& ex)
{
    if(dynamic_cast<const Freeze::DeadlockException*>(&ex))
    {
        throw DeadlockException(file, line, ex);
    }
    else if(dynamic_cast<const Freeze::NotFoundException*>(&ex))
    {
        throw NotFoundException(file, line);
    }
    else
    {
        throw DatabaseException(file, line, ex);
    }
}

DatabaseConnection::DatabaseConnection(const Freeze::ConnectionPtr& connection) : _connection(connection)
{
}

Ice::EncodingVersion
DatabaseConnection::getEncoding() const
{
    return _connection->getEncoding();
}

void 
DatabaseConnection::beginTransaction()
{
    assert(!_connection->currentTransaction());
    _connection->beginTransaction();
}

void 
DatabaseConnection::commitTransaction()
{
    assert(_connection->currentTransaction());
    _connection->currentTransaction()->commit();
}

void 
DatabaseConnection::rollbackTransaction()
{
    assert(_connection->currentTransaction());
    _connection->currentTransaction()->rollback();
}

ConnectionPool::ConnectionPool(const Ice::CommunicatorPtr& communicator, const string& envName) :
    _communicator(communicator),
    _envName(envName),
    _connection(newConnection())
{
}

IceDB::DatabaseConnectionPtr
ConnectionPool::getConnection()
{
    return _connection;
}

IceDB::DatabaseConnectionPtr
ConnectionPool::newConnection()
{
    return new DatabaseConnection(Freeze::createConnection(_communicator, _envName));
}
