// Copyright (c) ZeroC, Inc.

#include "FileCache.h"
#include "../Ice/FileUtil.h"
#include "Ice/Communicator.h"
#include "Ice/Properties.h"
#include "IceGrid/Admin.h"

#include <deque>
#include <fstream>

using namespace std;
using namespace IceGrid;

FileCache::FileCache(const shared_ptr<Ice::Communicator>& com)
    : _messageSizeMax(com->getProperties()->getIcePropertyAsInt("Ice.MessageSizeMax") * 1024 - 256)
{
}

int64_t
FileCache::getOffsetFromEnd(const string& file, int originalCount)
{
    ifstream is(IceInternal::streamFilename(file).c_str()); // file is a UTF-8 string
    if (is.fail())
    {
        throw FileNotAvailableException("failed to open file '" + file + "'");
    }

    if (originalCount < 0)
    {
        return 0;
    }

    is.seekg(0, ios::end);
    streampos endOfFile = is.tellg();
    if (originalCount == 0)
    {
        return endOfFile;
    }

    streamoff blockSize = 16 * 1024; // Start reading a block of 16K from the end of the file.
    streampos lastBlockOffset = endOfFile;
    int totalCount = 0;
    string line;
    deque<pair<streampos, string>> lines;
    do
    {
        lines.clear();

        //
        // Move the current position of the stream to the new block to
        // read.
        //
        is.clear();
        if (lastBlockOffset - blockSize > streamoff(0))
        {
            is.seekg(lastBlockOffset - blockSize);
            getline(is, line); // Ignore the first line as it's most likely not complete.
        }
        else
        {
            is.seekg(0, ios::beg); // We've reach the beginning of the file.
        }

        //
        // Read the block and count the number of lines in the block
        // If we found the "first last N lines", we start throwing out
        // the lines read at the beginning of the file.
        //
        int count = originalCount - totalCount; // Number of lines left to find.
        while (is.good() && is.tellg() <= streamoff(lastBlockOffset))
        {
            streampos beg = is.tellg();
            getline(is, line);
            if (is.eof() && line.empty()) // Don't count the last line if it's empty.
            {
                continue;
            }

            lines.emplace_back(beg, line);
            ++totalCount;
            if (lines.size() == static_cast<unsigned int>(count + 1))
            {
                --totalCount;
                lines.pop_front();
            }
        }

        if (lastBlockOffset - blockSize < streamoff(0))
        {
            break; // We're done if the block started at the beginning of the file.
        }
        else if (totalCount < originalCount)
        {
            //
            // Otherwise, it we still didn't find the required number of lines,
            // read another block of text before this block.
            //
            lastBlockOffset -= blockSize; // Position of the block we just read.
            blockSize *= 2;               // Read a bigger block.
        }
    } while (totalCount < originalCount && !is.bad());

    if (is.bad())
    {
        throw FileNotAvailableException("unrecoverable error occurred while reading file '" + file + "'");
    }

    if (lines.empty())
    {
        return 0;
    }
    else
    {
        return lines[0].first;
    }
}

bool
FileCache::read(const string& file, int64_t offset, int size, int64_t& newOffset, Ice::StringSeq& lines)
{
    assert(size > 0);

    if (size > _messageSizeMax)
    {
        size = _messageSizeMax;
    }

    if (size <= 5)
    {
        throw FileNotAvailableException("maximum bytes per read request is too low");
    }

    ifstream is(IceInternal::streamFilename(file).c_str()); // file is a UTF-8 string
    if (is.fail())
    {
        throw FileNotAvailableException("failed to open file '" + file + "'");
    }

    //
    // Check if the requested offset is past the end of the file, if
    // that's the case return an empty sequence of lines and indicate
    // the EOF.
    //
    is.seekg(0, ios::end);
    if (offset >= is.tellg())
    {
        newOffset = is.tellg();
        lines = Ice::StringSeq();
        return true;
    }

    //
    // Read lines from the file until we read enough or reached EOF.
    //
    newOffset = offset;
    lines = Ice::StringSeq();
    is.seekg(static_cast<streamoff>(offset), ios::beg);
    int totalSize = 0;
    string line;

    while (is.good())
    {
        getline(is, line);

        int lineSize = static_cast<int>(line.size()) + 5; // 5 bytes for the encoding of the string size (worst case)
        if (lineSize + totalSize > size)
        {
            if (totalSize + 5 < size)
            {
                // There's some room left for a part of the string, return a partial string
                line = line.substr(0, static_cast<size_t>(size - totalSize - 5));
                lines.push_back(line);
                newOffset += line.size();
            }
            else
            {
                lines.emplace_back("");
            }
            return false; // We didn't reach the end of file, we've just reached the size limit!
        }

        totalSize += lineSize;
        lines.push_back(line);

        //
        // If there was a partial read update the offset using the current line size,
        // otherwise we have read a new complete line and we can use tellg to update
        // the offset.
        //
        if (!is.good())
        {
            newOffset += line.size();
        }
        else
        {
            newOffset = is.tellg();
        }
    }

    if (is.bad())
    {
        throw FileNotAvailableException("unrecoverable error occurred while reading file '" + file + "'");
    }

    return is.eof();
}
