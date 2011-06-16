// Copyright (C) 2010 by Ivan Vashchaev
// Modified      2011 Oliver Schulz
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.


#include <memory.h>
#include <algorithm>
#include "block_allocator.h"


namespace froast {


block_allocator::block_allocator(size_t blocksize): m_head(0), m_blocksize(blocksize)
{
}

block_allocator::~block_allocator()
{
	while (m_head)
	{
		block *temp = m_head->next;
		::free(m_head);
		m_head = temp;
	}
}

void block_allocator::swap(block_allocator &rhs)
{
	std::swap(m_blocksize, rhs.m_blocksize);
	std::swap(m_head, rhs.m_head);
}

void *block_allocator::malloc(size_t size)
{
	if ((m_head && m_head->used + size > m_head->size) || !m_head)
	{
		// calc needed size for allocation
		size_t alloc_size = std::max(sizeof(block) + size, m_blocksize);

		// create new block
		char *buffer = (char *)::malloc(alloc_size);
		block *b = reinterpret_cast<block *>(buffer);
		b->size = alloc_size;
		b->used = sizeof(block);
		b->buffer = buffer;
		b->next = m_head;
		m_head = b;
	}

	void *ptr = m_head->buffer + m_head->used;
	m_head->used += size;
	return ptr;
}

void block_allocator::free()
{
	block_allocator(0).swap(*this);
}


} // namespace froast
