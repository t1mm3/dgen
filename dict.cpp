#include "dict.hpp"
#include "utils.hpp"
#include <fstream>
#include <iostream>
#include <cassert>
#include <stdexcept>

Buffer::Buffer()
 : size(0), next(nullptr)
{
}

char*
Buffer::Alloc(size_t bytes)
{
	assert(data);
	if (size + bytes > capacity) {
		return nullptr;
	}
	char* r = data;
	data += bytes;
	return r;
}


Buffer*
BufferFactory::NewBuffer(size_t capacity)
{
	char* b = new char[capacity + sizeof(Buffer) + alignof(Buffer)];
	if (!b) {
		return nullptr;
	}

	Buffer* buf = new (b) Buffer();

	buf->capacity = capacity;
	buf->data = (char*)buf + sizeof(Buffer);

	return buf;
}

void
BufferFactory::FreeBuffer(Buffer* buf)
{
	char* b = (char*)buf;
	delete[] b;
}


Dictionary::Dictionary()
{
}

Dictionary::~Dictionary()
{
}


void
Dictionary::lookup(char** __restrict__ ptr, size_t* __restrict__ len,
	size_t* __restrict__ indices, size_t num, int* __restrict__ sel,
	const Entry* __restrict__ entries, size_t max)
{
	VectorExec(sel, num, [&] (auto i) {
		auto& e = entries[indices[i] % max];
		ptr[i] = e.str;
		len[i] = e.len;
	});
}

void
InlineDictionary::Put(const std::string& w)
{
	strings.emplace_back(w);

	auto& s = strings[strings.size() - 1];
	Insert((char*)s.c_str(), s.size());
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