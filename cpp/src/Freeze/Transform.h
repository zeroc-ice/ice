// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef TRANSFORM_H
#define TRANSFORM_H

#include <string>
#include <iosfwd>
#include <map>
#include <vector>

#include <dom/DOM.hpp>

class IllegalTransform
{
public:

    IllegalTransform(const char* file, int line) : _file(file), _line(line) { }
    ~IllegalTransform() { }

    const char* file() const { return _file; }
    int line() const { return _line; }

private:

    const char* _file;
    int _line;
};

::std::ostream& operator<<(::std::ostream&, const IllegalTransform&);

class IncompatibleSchema
{
public:

    IncompatibleSchema(const char* file, int line) : _file(file), _line(line) { }
    ~IncompatibleSchema() { }

    const char* file() const { return _file; }
    int line() const { return _line; }

private:

    const char* _file;
    int _line;
};

::std::ostream& operator<<(::std::ostream&, const IncompatibleSchema&);

class InvalidSchema
{
public:

    InvalidSchema(const char* file, int line) : _file(file), _line(line) { }
    ~InvalidSchema() { }

    const char* file() const { return _file; }
    int line() const { return _line; }

private:

    const char* _file;
    int _line;
};

::std::ostream& operator<<(::std::ostream&, const InvalidSchema&);

class SchemaViolation
{
public:

    SchemaViolation(const char* file, int line) : _file(file), _line(line) { }
    ~SchemaViolation() { }

    const char* file() const { return _file; }
    int line() const { return _line; }

private:

    const char* _file;
    int _line;
};

::std::ostream& operator<<(::std::ostream&, const SchemaViolation&);

struct DocumentInfo;

class Transform
{
public:

    Transform() { }
    virtual ~Transform() { }

    virtual void transform(::std::ostream&, const DocumentInfo*, const ::std::string&, DOM_Node) = 0;
    virtual ::std::ostream& print(::std::ostream&) = 0;
};

typedef ::std::map< ::std::string, Transform*> TransformMap;

class Transformer
{
public:

    Transformer(DOM_Document&, DOM_Document&);
    ~Transformer();

    void transform(::std::ostream&, DOM_Document&);

private:

    //
    // Map of local@uri element names to transforms. Needed for actual
    // transform.
    //
    TransformMap _elements;

    //
    // Map of local@uri class transforms (based on static type). This
    // information cached for creation of the transform.
    //
    TransformMap _staticClassTransforms;
};

#endif
