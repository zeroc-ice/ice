// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#include <map>
#include <list>
#include <iostream>

#include <assert.h>

#include <parsers/SAXParser.hpp>
#include <sax/HandlerBase.hpp>

#include <IceStorm/WeightedGraph.h>

using namespace std;
using namespace IceStorm;

namespace IceStorm
{

class SAXErrorHandler : public ErrorHandler
{
public:

    void
    warning(const SAXParseException& exception)
    {
	char* s = XMLString::transcode(exception.getMessage());
	cerr << "warning: " << s << endl;
	delete[] s;
    }

    void
    error(const SAXParseException& exception)
    {
	char* s = XMLString::transcode(exception.getMessage());
	cerr << "error: " << s << endl;
	delete[] s;
    }

    void
    fatalError(const SAXParseException& exception)
    {
	char* s = XMLString::transcode(exception.getMessage());
	cerr << "fatal:" << s << endl;
	delete[] s;
    }

    void
    resetErrors()
    {
    }
};

#ifdef WIN32
#    pragma warning(disable:4786)
#endif

class SAXGraphHandler : public DocumentHandler
{
public:

    SAXGraphHandler(WeightedGraph& graph) :
	_graph(graph)
    {
    }

    ~SAXGraphHandler()
    {
    }

    virtual void characters(const XMLCh *const chars, const unsigned int length) { }
    virtual void endDocument() { }
    virtual void endElement(const XMLCh *const name) { }
    virtual void ignorableWhitespace(const XMLCh *const chars, const unsigned int length) { }
    virtual void processingInstruction(const XMLCh *const target, const XMLCh *const data) { }
    virtual void resetDocument() { }
    virtual void setDocumentLocator(const Locator *const locator) { }
    virtual void startDocument() { }
    virtual void startElement(const XMLCh *const name, AttributeList &attrs); 

private:

    WeightedGraph& _graph;
};

//
// Exception during parsing.
//
struct WeightedGraphParseException
{
    std::string reason;
};

} // End namespace IceStorm

void
SAXGraphHandler::startElement(const XMLCh *const name, AttributeList &attrs) 
{
    char* str = XMLString::transcode(name);
    
    try
    {
	if (strcmp(str, "vertex") == 0)
	{
	    XMLCh* n = XMLString::transcode("name");
	    const XMLCh* value = attrs.getValue(n);
	    delete[] n;
	    if (value == 0)
	    {
		WeightedGraphParseException ex;
		ex.reason = "<vertex> name attribute missing";
		throw ex;
	    }

	    char* vstr = XMLString::transcode(value);

	    _graph.addVertex(vstr);

	    delete[] vstr;
	}
	else if (strcmp(str, "edge") == 0)
	{
	    XMLCh* n = XMLString::transcode("source");
	    const XMLCh* value = attrs.getValue(n);
	    delete[] n;
	    if (value == 0)
	    {
		WeightedGraphParseException ex;
		ex.reason = "<edge> source attribute missing";
		throw ex;
	    }

	    char* source = XMLString::transcode(value);
	    n = XMLString::transcode("target");
	    value = attrs.getValue(n);
	    delete[] n;
	    if (value == 0)
	    {
		delete[] source;
		WeightedGraphParseException ex;
		ex.reason = "<edge> target attribute missing";
		throw ex;
	    }
	    char* target = XMLString::transcode(value);
	    n = XMLString::transcode("cost");
	    value = attrs.getValue(n);
	    delete[] n;

	    int cost = 0;
	    if (value != 0)
	    {
		char* cstr = XMLString::transcode(value);
		cost = atoi(cstr);
		delete[] cstr;
	    }

	    try
	    {
		_graph.addEdge(source, target, cost);
	    }
	    catch(...)
	    {
		delete[] source;
		delete[] target;
		throw;
	    }
	}
    }
    catch(const WeightedGraphParseException& ex)
    {
	//
	// Xerces eats all exceptions (apparently by design). Not sure
	// how the error handling is supposed to work.
	//
	cerr << ex.reason << endl;
	_graph.error();
	delete[] str;
    }
    catch(...)
    {
	_graph.error();
	delete[] str;
    }
    delete[] str;
}

WeightedGraph::WeightedGraph(bool reflective) :
    _reflective(reflective)
{
}

WeightedGraph::~WeightedGraph()
{
}

bool
WeightedGraph::parse(const string& xmlFile)
{
    _error = 0;
    SAXParser* parser = new SAXParser;
    parser->setValidationScheme(SAXParser::Val_Never);

    try
    {
	SAXErrorHandler err;
	SAXGraphHandler handler(*this);
	parser->setDocumentHandler(&handler);
	parser->setErrorHandler(&err);
	parser->parse(xmlFile.c_str());
    }
    catch(const XMLException& e)
    {
	cerr << e.getMessage() << endl;
    }
    int rc = parser->getErrorCount();
    delete parser;

    return _error == 0 && rc == 0;
}

void
WeightedGraph::dump(ostream& os)
{
    for (unsigned int i = 0 ; i < _vertices.size(); ++i)
    {
	for (unsigned int j = 0; j < _vertices.size(); ++j)
	{
	    os << _edges[i*_vertices.size() + j] << " ";
	}
	os << endl;
    }
}

void
WeightedGraph::swap(const std::vector<int>& edges)
{
    assert(edges.size() == _edges.size());
    _edges = edges;
}

vector<string>
WeightedGraph::getVertices() const
{
    return _vertices;
}

vector<pair<string, int> >
WeightedGraph::getEdgesFor(const std::string& vertex) const
{
    vector<pair<string, int> > edges;

    map<std::string, int>::const_iterator p = _vlookup.find(vertex);
    if (p != _vlookup.end())
    {
	int row = p->second * _vertices.size();
	for (unsigned int i = row; i < row + _vertices.size(); ++i)
	{
	    if (_edges[i] != -1)
	    {
		edges.push_back(make_pair(_vertices[i-row], _edges[i]));
	    }
	}
    }

    return edges;
}

void
WeightedGraph::compute(vector<int>& newEdges, int max)
{
    //
    // Reinitialize the adjacency matrix.
    //
    newEdges.clear();
    newEdges.resize(_vertices.size()*_vertices.size(), -1);

    //
    // For each vertex calculate the cost for all reachable vertexes
    // within the given max cost.
    //
    for (unsigned int i = 0; i < _vertices.size(); ++i)
    {
	//
	// List of <vertex-index,cost> pairs.
	//
	list<pair<unsigned int, int> > visited;

	visit(i, 0, visited, max);

	//
	// Add each vertex-index/cost pair to the new adjacency
	// matrix.
	//
	for (list<pair<unsigned int, int> >::iterator p = visited.begin(); p != visited.end(); ++p)
	{
	    //
	    // Ignore loops.
	    //
	    if ((*p).first != i)
	    {
		newEdges[i*_vertices.size() + (*p).first] = (*p).second;
		if (_reflective)
		{
		    newEdges[(*p).first*_vertices.size() + i] = (*p).second;
		}
	    }
	}
    }
}

void
WeightedGraph::visit(unsigned int vertex, int cost, list<pair<unsigned int, int> >& visited, int max)
{
    //
    // Is the given vertex already in the visited list? If so, then
    // check that is the cost is minimal cost and replace if not.
    //
    for ( list<pair<unsigned int, int> >::iterator p = visited.begin(); p != visited.end(); ++p)
    {
	if (p->first == vertex)
	{
	    if (p->second > cost)
	    {
		p->second = cost;
	    }
	    return;
	}
    }

    //
    // Add the vertex to the visited set (with the cost to visit).
    //
    visited.push_back(make_pair(vertex, cost));

    //
    // Run through each edges for this vertex (that's a row in the
    // adjacency matrix).
    //
    int row = vertex * _vertices.size();
    for (unsigned int i = row ; i < row + _vertices.size() ; ++i)
    {
	if (_edges[i] != -1)
	{
	    if (cost + _edges[i] <= max)
	    {
		visit(i - row, cost + _edges[i], visited, max);
	    }
	}
    }
}

void
WeightedGraph::addVertex(const string& name)
{
    _vertices.push_back(name);
}

void
WeightedGraph::addEdge(const string& from, const string& to, int cost)
{
    //
    // Loops are not permitted.
    //
    if (from == to)
    {
	WeightedGraphParseException ex;
	ex.reason = "loops are not permitted";
	throw ex;
    }

    if (_edges.size() == 0)
    {
	//
	// Prepare the edge map
	//
	_edges.resize(_vertices.size()*_vertices.size(), -1);
	for (unsigned int i = 0; i < _vertices.size(); ++i)
	{
	    _vlookup.insert(make_pair(_vertices[i], i));
	}
    }

    //
    // Location of from and to.
    //
    map<string, int>::iterator p = _vlookup.find(from);
    if (p == _vlookup.end())
    {
	WeightedGraphParseException ex;
	ex.reason = "<edge> vertex " + from + " not found";
	throw ex;
    }

    int fidx = p->second;
    p = _vlookup.find(to);
    if (p == _vlookup.end())
    {
	WeightedGraphParseException ex;
	ex.reason = "<edge> vertex " + to + " not found";
	throw ex;
    }
    int tidx = p->second;

    //
    // Fill in from, to - to,from
    //
    _edges[fidx*_vertices.size() + tidx] = cost;
    if (_reflective)
    {
	_edges[tidx*_vertices.size() + fidx] = cost;
    }
}

void
WeightedGraph::error()
{
    //
    // An error occurred during the semantic handling of the document.
    //
    ++_error;
}

