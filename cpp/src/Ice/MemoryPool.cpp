// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/MemoryPool.h>

// #define MEMPOOL_DEBUG

//
// TODO: Will a rover to a free block help speed things up here? It's
// probably not a very significant optimization as long as the pool is
// primarily for BasicStream usage. If it becomes more general purpose, it
// will need more consideration.
//

namespace IceInternal
{

struct BlockInfo;

struct PageInfo
{
    size_t nBlocks;
    size_t pageSize;

    BlockInfo* blocks;
    PageInfo* nextPage;
};

struct BlockInfo
{
    //
    // We could get rid of the next link since the next blockinfo should b
    // b+size. However, it is pretty useful as a convenience and it allows
    // for a some block validation.
    //
    BlockInfo* next; 
    BlockInfo* prev;

    //
    // We keep track of the parent page so we can quickly update
    // information on the parent page. 
    //
    PageInfo*  page;  

    //
    // The amount of user memory for this block. The actual size of the
    // block in the pool is size + sizeof BlockInfo
    //
    size_t size;

    //
    // The amount of memory in the block that is actually used.
    size_t used;

    //
    // Flag indicating whether this block is allocated or not.
    //
    bool free;

    Ice::Byte* user;
};

const size_t blocksPerOversize = 4;

} // End of namespace IceInternal

IceInternal::MemoryPool::MemoryPool(size_t highWaterMark):
    _pageSize(4 * 1024 * 1024),
    _maxPageSize(8 * _pageSize),
    _highWaterMark(highWaterMark),
    _currentSize(0),
    _pages(0)
{
}

IceInternal::MemoryPool::~MemoryPool()
{
    while(_pages != 0)
    {
	PageInfo* current = _pages;
	_pages = _pages->nextPage;
	::free(current);
    }
}

//
// The Memory pool's public interface. There should be no reason to
// call directly on the memory pool's instance.
// 
Ice::Byte* 
IceInternal::MemoryPool::alloc(size_t n)
{
    IceUtil::Mutex::Lock lock(_mutex);
    return allocBlock(n);
}

void
IceInternal::MemoryPool::free(Ice::Byte* b)
{
    if(b == 0)
    {
	return;
    }
    IceUtil::Mutex::Lock lock(_mutex);
    freeBlock(b);
}

Ice::Byte* 
IceInternal::MemoryPool::realloc(Ice::Byte* b, size_t n)
{
    //
    // TODO: Is this safe? Can we assume that nobody else is going to try and
    // delete this block? If so this is super speedy! In one throughput
    // test with 1000 iterations, we call realloc 4200 times!
    //
    BlockInfo* block = reinterpret_cast<BlockInfo*>(b - sizeof(BlockInfo));
    if(block->size >= n)
    {
	block->used = n;
	return b;
    }

    IceUtil::Mutex::Lock lock(_mutex);
    return reallocBlock(b, n);
}

IceInternal::BlockInfo* 
IceInternal::MemoryPool::initBlock(Ice::Byte* p, PageInfo* page, size_t n, bool allocated)
{
    BlockInfo* block = reinterpret_cast<BlockInfo*>(p);
    block->size = n;
    block->prev = 0;
    block->next = 0;
    block->page = page;
    block->free = !allocated;
    block->user = p + sizeof(BlockInfo);

#ifdef MEMPOOL_DEBUG
    memset(block->user, 'I', block->size);
#endif
    return block;
}

//
// Page layout:
//
// +----------+-----------+-------------+-----------+--------------......-+
// | PageInfo | BlockInfo | user memory | BlockInfo | user memory         |
// +----------+-----------+-------------+-----------+--------------......-+
//
IceInternal::PageInfo* 
IceInternal::MemoryPool::initPage(size_t n)
{
    Ice::Byte* rawData = reinterpret_cast<Ice::Byte*>(malloc(n));
#ifdef MEMPOOL_DEBUG
    memset(rawData, 'P', n);
#endif
    if(rawData == 0)
    {
	return 0;
    }

    PageInfo* p = reinterpret_cast<PageInfo*>(rawData);

    p->nBlocks = 0;

    //
    // We keep track of the page size because it'll help make decisions
    // about culling free pages.
    //
    p->pageSize = n;

    //
    // Initialize the first free block.
    //
    p->blocks = initBlock(rawData + sizeof(PageInfo), p, n - (sizeof(PageInfo) + sizeof(BlockInfo)), false);
    p->nextPage = 0;
    return p; 
}

IceInternal::PageInfo* 
IceInternal::MemoryPool::createNewPage(size_t n)
{
    const size_t overhead = sizeof(PageInfo) + sizeof(BlockInfo);
    const size_t defaultMaxBlockSize = _pageSize - overhead;

    size_t newPageSize = 0;
    if(n > defaultMaxBlockSize)
    {
	if((n + overhead) < _maxPageSize)
	{
	    newPageSize = _maxPageSize;
	}
	else 
	{
	    newPageSize = n + overhead;
	}
    }
    else
    {
	newPageSize = _pageSize;
    }
    _currentSize += newPageSize;
    return initPage(newPageSize);
}

//
// Remove unused pages (pages with no allocated blocks). If the force
// arg is false, only remove pages if the total memory allocated for the
// pool is over the high watermark. If force is true, remove unused
// pages unconditionally. Generally speaking, this is useful for
// shrinking memory to within a certain constraint when possible, but
// that doesn't mean that it's always possible. There may not be any
// free pages. However, since this pool is primarily intended for the
// Ice::BasicStream class, usage of pool memory is probably fairly
// transient so opportunities for cleanup will occur fairly often.
//
void
IceInternal::MemoryPool::purgePages(bool force)
{
    if((force || _currentSize > _highWaterMark))
    {
	PageInfo* newList = 0;
	PageInfo* p = _pages;
	while(p != 0)
	{
	    PageInfo* next = p->nextPage;
	    if(p->nBlocks == 0)
	    {
		_currentSize -= p->pageSize;
		::free(p);
	    }
	    else
	    {
		p->nextPage = newList;
		newList = p;
	    }
	    p = next;
	}
	_pages = newList;
    }
}

//
// Get some memory from the pool.
//
Ice::Byte* 
IceInternal::MemoryPool::allocBlock(size_t n)
{
    if(n < 16)
    {
	n = 16;
    }
    //
    // All n should be an exact multiple of 16. This makes address math
    // a little easier and it ensures that blocks aren't insanely
    // small. This should not be an issue when servicing
    // Ice::BasicStream. 
    //
    n = n + (n % 16);

    //
    // Try each page until we get a successful allocation.
    //
    for(PageInfo* p = _pages; p != 0; p = p->nextPage)
    {
	Ice::Byte* block = getBlock(p, n);
	if(block)
	{
	    return block;
	}
    }

    //
    // None of the currently allocated pages has sufficient free space
    // to allocate a block of the required size, so we'll need to
    // create a new page and allocate from there. 
    //
    PageInfo* newPage = createNewPage(n);
    if(newPage == 0)
    {
	//
	// Trouble! Our attempt to create a page has failed, so we need
	// to look at purging pages and try again. 
	//
	purgePages(true);
	
	newPage = createNewPage(n);
	assert(newPage != 0);

	//
	// If newPage is 0, there will be trouble. Since we are
	// malloc() based, returning 0 is the most reasonable thing to
	// do and matches earlier behavior.
	//
	if(newPage == 0)
	{
	    return 0;
	}
    }
    newPage->nextPage = _pages;
    _pages = newPage;
    return getBlock(newPage, n);
}

#ifdef MEMPOOL_DEBUG
void 
validateBlock(BlockInfo* p)
{
    assert(!p->prev || p->prev->next == p);
    assert(!p->next || p->next->prev == p);
    if(p->next)
    {
	assert(reinterpret_cast<size_t>(p) + sizeof(BlockInfo) + p->size == reinterpret_cast<size_t>(p->next));
    }
    if(p->prev)
    {
	assert(reinterpret_cast<size_t>(p->prev) + sizeof(BlockInfo) + p->prev->size == 
	       reinterpret_cast<size_t>(p));
    }
}
#else
#   define validateBlock(x) (void)x
#endif

//
// Iterate through this page's blocks, trying to find one that is big
// enough for 'n'. Return an address to a block's user memory on a
// successful find, otherwise return 0.
//
Ice::Byte* 
IceInternal::MemoryPool::getBlock(PageInfo* page, size_t n)
{
    BlockInfo* p = page->blocks;

    const size_t requiredMem = n + sizeof(BlockInfo);

    while(p != 0)
    {
	if((n <= p->size) && p->free)
	{
	    validateBlock(p);
	    Ice::Byte* base = reinterpret_cast<Ice::Byte*>(p);
	    BlockInfo* newBlock = 0;

	    //
	    // TODO: It might be nice to leave some extra space for
	    // reallocations. How big of a space to reserve? Since
	    // Ice::BasicStream already does a 'predictive' reserve and
	    // we coalesce adjacent free blocks, it might be overkill
	    // at this point.
	    //
	    size_t offset = 0;
	    if ((requiredMem + 16) <= p->size)
	    {
		//
		// p will be the block for the allocated memory.
		// newBlock will be the remaining free memory and will
		// be to the 'right' of p.
		//
		offset = requiredMem;
	    }

	    if(offset != 0)
	    {
		newBlock = initBlock(base + offset, p->page, p->size - requiredMem, false);
		newBlock->next = p->next;
		newBlock->prev = p;
		if(newBlock->next)
		{
		    newBlock->next->prev = newBlock;
		}

		//
		// Adjust p's members.
		//
		p->next = newBlock;
		p->size = n;
	    }

	    if(newBlock)
	    {
		validateBlock(newBlock);
	    }

	    p->free = false;
	    p->page->nBlocks++; 
#ifdef MEMPOOL_DEBUG
	    memset(p->user, 'G', p->size);
#endif
	    validateBlock(p);
	    p->used = n;
	    return p->user;
	}
	p = p->next;
    }

    return 0;
}

void 
IceInternal::MemoryPool::freeBlock(Ice::Byte* p)
{
    BlockInfo* block = reinterpret_cast<BlockInfo*>(p - sizeof(BlockInfo));


    validateBlock(block);
    block->free = true;
    block->used = 0;
    block->page->nBlocks--;

    //
    // Combine with next block if it is free. This means that the next
    // block is obliterated.
    //
    BlockInfo* nextBlock = block->next;
    if(nextBlock && nextBlock->free)
    {
	block->size += nextBlock->size + sizeof(BlockInfo);
	block->next = nextBlock->next;
	if(nextBlock->next)
	{
	    nextBlock->next->prev = block;
	}
    }

    //
    // Combine with the previous block if it is free. This means that
    // this block is obliterated.
    //
    BlockInfo* previousBlock = block->prev;
    if(previousBlock && previousBlock->free)
    {
	previousBlock->size += block->size + sizeof(BlockInfo);
	previousBlock->next = block->next;
	if(block->next)
	{
	    block->next->prev = previousBlock;
	}
	block = previousBlock;
    }

#ifdef MEMPOOL_DEBUG
    memset(block->user, 'E', block->size);
    if(block->prev)
    {
	validateBlock(block->prev);
    }
    if(block->next)
    {
	validateBlock(block->next);
    }
    validateBlock(block);
#endif

    if(block->page->nBlocks == 0)
    {
	purgePages(false);
    }
}

Ice::Byte* 
IceInternal::MemoryPool::reallocBlock(Ice::Byte* p, size_t n)
{
    //
    // Note: The way we allocate and free blocks *could* mean that a
    // free block is available immediately after the current block.
    //
    BlockInfo* block = reinterpret_cast<BlockInfo*>(p - sizeof(BlockInfo));
    assert(!block->free);
    validateBlock(block);

    //
    // The way we allocate blocks, its very possible that the
    // current block is already big enough!
    //
    if(n > block->size)
    {
	//
	// If the next block is free, try combining it with the
	// current block to satisfy the allocation requirement.
	//
	if(block->next && block->next->free && (block->size + block->next->size) >= n)
	{
	    block->size += block->next->size;
	    block->next = block->next->next;
	    if(block->next)
	    {
		block->next->prev = block;
	    }
	    block->used = n;
	    validateBlock(block);
	}
	else
	{
	    //
	    // Realloc with current block has failed. Allocate a new
	    // block that is big enough and copy the contents of the
	    // old block into the new.
	    //
	    Ice::Byte* t = allocBlock(n);
	    memcpy(t, p, block->used);
	    freeBlock(p);
	    return t;
	}
    }

    assert(n <= block->size);
    return p;
}
