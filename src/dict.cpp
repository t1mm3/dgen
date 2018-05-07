#include "dict.hpp"
#include "utils.hpp"
#include "build.hpp"
#include <fstream>
#include <iostream>
#include <cassert>
#include <stdexcept>

Dictionary::Dictionary()
{
}

Dictionary::~Dictionary()
{
}

#include "dict_impl.hpp"

void
Dictionary::lookup(char** __restrict__ ptr, size_t* __restrict__ len,
	size_t* __restrict__ indices, size_t num, int* __restrict__ sel,
	const Entry* __restrict__ entries, size_t max, BaseType t)
{
	switch (t) {
#define A(C, B) case B: dictlookup_##C(ptr, len, (C*)indices, num, sel, entries, max); break;
	A(int8_t, I8);
	A(uint8_t, U8);

	A(int16_t, I16);
	A(uint16_t, U16);

	A(int32_t, I32);
	A(uint32_t, U32);

	A(int64_t, I64);

#undef A
	default:
		assert(false);
	}
}

void
InlineDictionary::Put(const std::string& w)
{
	const size_t len = w.size();
	const size_t alloc_len = len + kBufferSize;

	char* sptr = nullptr;
	if (m_head) {
		sptr = m_head->Alloc(len);
	}

	// out of memory?
	if (!sptr) {
		auto nhead = m_alloc.NewBuffer(std::max(kBufferSize, alloc_len));
		nhead->next = m_head;
		m_head = nhead;

		sptr = m_head->Alloc(alloc_len);
		if (!sptr) {
			throw std::bad_alloc();
		}
	}

	// copy string
	memcpy(sptr, w.c_str(), len);

	// insert
	Insert(sptr, len);
}

InlineDictionary::~InlineDictionary()
{
	Buffer* curr = m_head;
	while (curr) {
		auto next = curr->next;
		m_alloc.FreeBuffer(curr);
		curr = next;
	}
}

FileDictionary::FileDictionary(const std::string& file)
 : Dictionary()
{
	m_file_open = false;
	m_file.open(file);
	if(m_file.is_open()) {
		try {
			size_t size = m_file.size();
			char *const data = (char*)m_file.data();
			char *const end = data + size;

			auto push_word = [&] (char* start, char* end) {
				ssize_t len = end - start;
				assert(len > 0);

				Insert(start, len);
			};

			char* curr = data;
			char* begin = data;
			while (*curr && curr != end) {
				if (curr[0] == '\n') {
					push_word(begin, curr);
					curr++; // skip newline
					begin = curr;
				}
				curr++;
			}

			push_word(begin, curr);

			m_file_open = true;
		} catch (...) {
			m_file.close();
			throw;
		}		
	} else {
		throw std::invalid_argument("could not map the file '" + file + "'");
	}
}

FileDictionary::~FileDictionary()
{
	if (m_file_open) {
		m_file.close();
	}
}