// **********************************************************************
//
// Copyright (c) 2003
// ZeroC, Inc.
// Billerica, MA, USA
//
// All Rights Reserved.
//
// Ice is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License version 2 as published by
// the Free Software Foundation.
//
// **********************************************************************

#ifndef XML_TRANSFORM_H
#define XML_TRANSFORM_H

#include <Freeze/DB.h>
#include <IceUtil/OutputUtil.h>

#include <xercesc/dom/DOM.hpp>

#include <map>

#ifdef WIN32
#   ifdef XML_TRANSFORM_API_EXPORTS
#       define XML_TRANSFORM_API __declspec(dllexport)
#   else
#       define XML_TRANSFORM_API __declspec(dllimport)
#   endif
#else
#   define XML_TRANSFORM_API /**/
#endif

namespace XMLTransform
{

//
// Raised in the case that the transform is illegal.
//
class XML_TRANSFORM_API IllegalTransform : public ::IceUtil::Exception
{
public:

    IllegalTransform(const char*, int);

    virtual ::std::string ice_name() const;
    virtual void ice_print(::std::ostream&) const;
    virtual ::IceUtil::Exception* ice_clone() const;
    virtual void ice_throw() const;

    ::std::string reason;
};

//
// Raised in the case that the given schema is incompatible with the old schema.
//
class XML_TRANSFORM_API IncompatibleSchema : public ::IceUtil::Exception
{
public:

    IncompatibleSchema(const char*, int);

    virtual ::std::string ice_name() const;
    virtual void ice_print(::std::ostream&) const;
    virtual ::IceUtil::Exception* ice_clone() const;
    virtual void ice_throw() const;

    ::std::string reason;
};

//
// Raised in the case that the given schema is not an XML schema.
//
class XML_TRANSFORM_API InvalidSchema : public ::IceUtil::Exception
{
public:

    InvalidSchema(const char*, int);

    virtual ::std::string ice_name() const;
    virtual void ice_print(::std::ostream&) const;
    virtual ::IceUtil::Exception* ice_clone() const;
    virtual void ice_throw() const;

    ::std::string reason;
};

//
// Raised in the case that the given XML instance document violates the associated schema.
//
class XML_TRANSFORM_API SchemaViolation : public ::IceUtil::Exception
{
public:

    SchemaViolation(const char*, int);

    virtual ::std::string ice_name() const;
    virtual void ice_print(::std::ostream&) const;
    virtual ::IceUtil::Exception* ice_clone() const;
    virtual void ice_throw() const;

    ::std::string reason;
};

//
// Raised if a type was not found in the new schema.
//
class XML_TRANSFORM_API MissingTypeException : public ::IceUtil::Exception
{
public:

    MissingTypeException(const char*, int);

    virtual ::std::string ice_name() const;
    virtual void ice_print(::std::ostream&) const;
    virtual ::IceUtil::Exception* ice_clone() const;
    virtual void ice_throw() const;

    ::std::string reason;
};

//
// Raised in the case of a general transformation failure.
//
class XML_TRANSFORM_API TransformException : public ::IceUtil::Exception
{
public:

    TransformException(const char*, int);

    virtual ::std::string ice_name() const;
    virtual void ice_print(::std::ostream&) const;
    virtual ::IceUtil::Exception* ice_clone() const;
    virtual void ice_throw() const;

    ::std::string reason;
};

//
// Map from namespace prefix to namespace URI.
//
typedef ::std::map< ::std::string, ::std::string> PrefixURIMap;

//
// This represents the information associated with an XML document. Note that the namespace information is
// only retrieved from the top-level node (which matches the layout of XML schema documents).
//
class XML_TRANSFORM_API DocumentInfo : public ::IceUtil::Shared
{
public:

    DocumentInfo(DOMDocument*, bool, DOMNode*, const ::std::string& = "");
    ~DocumentInfo();

    DOMDocument* getDocument() const;
    ::std::string findURI(const ::std::string& prefix) const;
    ::std::string getTargetNamespace() const;

private:

    DOMDocument* _document;
    bool _releaseDocument;
    PrefixURIMap _nsMap;
    ::std::string _targetNamespace;
};

typedef ::IceUtil::Handle<DocumentInfo> DocumentInfoPtr;

//
// Transform interface.
//
class XML_TRANSFORM_API Transform : public ::IceUtil::Shared
{
public:

    Transform();
    virtual ~Transform();

    typedef ::std::map< ::std::string, ::std::string > MissingTypeMap;

    virtual void transform(::IceUtil::XMLOutput&, const DocumentInfoPtr&, const ::std::string&, DOMNode*,
                           const MissingTypeMap&) = 0;
    virtual void checkMissingTypes(const DocumentInfoPtr&, DOMNode*, const MissingTypeMap&) { }
    virtual void collectMissingTypes(const DocumentInfoPtr&, DOMNode*, MissingTypeMap&) { }
    virtual ::std::ostream& print(::std::ostream&) = 0;
};

typedef ::IceUtil::Handle<Transform> TransformPtr;

//
// Map of string to transform type.
//
typedef ::std::map< ::std::string, TransformPtr> TransformMap;

//
// Given an old and new XML schema, this class will transform an instance document that corresponds to the old
// schema formation into a document that matches the new schema.
//
class XML_TRANSFORM_API Transformer
{
public:

    Transformer(const Ice::StringSeq&, const Ice::StringSeq&, const Ice::StringSeq&, const Ice::StringSeq&,
                DOMDocument*, DOMDocument*);
    ~Transformer();

    void transform(::IceUtil::XMLOutput&, DOMDocument*, const std::string&, bool, bool = true);

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

class XML_TRANSFORM_API DBTransformer
{
public:

    DBTransformer(const Freeze::DBEnvironmentPtr&, const Freeze::DBPtr&, const Ice::StringSeq&, const Ice::StringSeq&,
                  const Ice::StringSeq&, const Ice::StringSeq&, bool);
    ~DBTransformer();

    //
    // This version allows the caller to specify the filenames for the
    // old and new schemas.
    //
    void transform(const std::string&, const std::string&);

    //
    // This version allows the caller to specify a single schema for
    // transformation. This is commonly used for schemas whose formal
    // types don't change, but whose actual (instance) types do change.
    //
    void transform(const std::string&);

private:

    void transform(DOMDocument*, DOMDocument*);

    Freeze::DBEnvironmentPtr _dbEnv;
    Freeze::DBPtr _db;
    Ice::StringSeq _loadOld;
    Ice::StringSeq _loadNew;
    Ice::StringSeq _pathOld;
    Ice::StringSeq _pathNew;
    bool _force;
};

} // End namespace XMLTransform

#endif
