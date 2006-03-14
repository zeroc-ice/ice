// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceStorm/WeightedGraph.h>
#include <IceXML/Parser.h>

#include <map>
#include <list>
#include <iostream>

using namespace std;
using namespace IceStorm;

namespace IceStorm
{

#ifdef _MSC_VER
#    pragma warning(disable:4786)
#endif

class GraphHandler : public IceXML::Handler
{
public:

    GraphHandler(WeightedGraph& graph) :
	_graph(graph)
    {
    }

    ~GraphHandler()
    {
    }

    virtual void startElement(const string&, const IceXML::Attributes&, int, int); 
    virtual void endElement(const string&, int, int) { }
    virtual void characters(const string&, int, int) { }

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
GraphHandler::startElement(const string& name, const IceXML::Attributes& attrs, int line, int)
{
    if(name == "vertex")
    {
        IceXML::Attributes::const_iterator p = attrs.find("name");
        if(p == attrs.end())
        {
            WeightedGraphParseException ex;
            ostringstream ostr;
            ostr << "line " << line << ": <vertex> name attribute missing";
            ex.reason = ostr.str();
            throw ex;
        }

        _graph.addVertex(p->second);
    }
    else if(name == "edge")
    {
        IceXML::Attributes::const_iterator p;

        p = attrs.find("source");
        if(p == attrs.end())
        {
            WeightedGraphParseException ex;
            ostringstream ostr;
            ostr << "line " << line << ": <edge> source attribute missing";
            ex.reason = ostr.str();
            throw ex;
        }

        string source = p->second;

        p = attrs.find("target");
        if(p == attrs.end())
        {
            WeightedGraphParseException ex;
            ostringstream ostr;
            ostr << "line " << line << ": <edge> target attribute missing";
            ex.reason = ostr.str();
            throw ex;
        }

        string target = p->second;

        int cost = 0;
        p = attrs.find("cost");
        if(p != attrs.end())
        {
            cost = atoi(p->second.c_str());
        }

        _graph.addEdge(source, target, cost);
    }
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

    try
    {
	GraphHandler handler(*this);
        IceXML::Parser::parse(xmlFile, handler); 
    }
    catch(const WeightedGraphParseException& e)
    {
	cerr << e.reason << endl;
	error();
    }
    catch(const IceXML::ParserException& e)
    {
	cerr << e << endl;
        error();
    }
    catch(...)
    {
	error();
    }

    return _error == 0;
}

void
WeightedGraph::dump(ostream& os)
{
    for(unsigned int i = 0 ; i < _vertices.size(); ++i)
    {
	for(unsigned int j = 0; j < _vertices.size(); ++j)
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
    if(p != _vlookup.end())
    {
	size_t row = p->second * _vertices.size();
	for(size_t i = row; i < row + _vertices.size(); ++i)
	{
	    if(_edges[i] != -1)
	    {
		edges.push_back(make_pair(_vertices[i - row], _edges[i]));
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
    for(unsigned int i = 0; i < _vertices.size(); ++i)
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
	for(list<pair<unsigned int, int> >::iterator p = visited.begin(); p != visited.end(); ++p)
	{
	    //
	    // Ignore loops.
	    //
	    if((*p).first != i)
	    {
		newEdges[i*_vertices.size() + (*p).first] = (*p).second;
		if(_reflective)
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
    for( list<pair<unsigned int, int> >::iterator p = visited.begin(); p != visited.end(); ++p)
    {
	if(p->first == vertex)
	{
	    if(p->second > cost)
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
    size_t row = vertex * _vertices.size();
    for(size_t i = row ; i < row + _vertices.size() ; ++i)
    {
	if(_edges[i] != -1)
	{
	    if(cost + _edges[i] <= max)
	    {
		visit(static_cast<unsigned int>(i - row), cost + _edges[i], visited, max);
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
    if(from == to)
    {
	WeightedGraphParseException ex;
	ex.reason = "loops are not permitted";
	throw ex;
    }

    if(_edges.size() == 0)
    {
	//
	// Prepare the edge map
	//
	_edges.resize(_vertices.size()*_vertices.size(), -1);
	for(unsigned int i = 0; i < _vertices.size(); ++i)
	{
	    _vlookup.insert(pair<const string, int>(_vertices[i], i));
	}
    }

    //
    // Location of from and to.
    //
    map<string, int>::iterator p = _vlookup.find(from);
    if(p == _vlookup.end())
    {
	WeightedGraphParseException ex;
	ex.reason = "<edge> vertex " + from + " not found";
	throw ex;
    }

    int fidx = p->second;
    p = _vlookup.find(to);
    if(p == _vlookup.end())
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
    if(_reflective)
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

