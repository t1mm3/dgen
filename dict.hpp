#ifndef H_GEN_DICT
#define H_GEN_DICT

#include <string>
#include <vector>
#include "buffer.hpp"

struct Dictionary {
protected:
	struct Entry {
		char* str;
		size_t len;
	};

	std::vector<Entry> m_index;

	const size_t kIndexAllocAhead = 1024*1024;

	static void lookup(char** __restrict__ ptr, size_t* __restrict__ len,
		size_t* __restrict__ indices, size_t num, int* __restrict__ sel,
		const Entry* __restrict__ entries, size_t max);
public:
	Dictionary();

	virtual ~Dictionary();

	size_t GetCount() const {
		return m_index.size();
	}

	void Insert(char* str, size_t len) {
		if (m_index.size() == m_index.capacity()) {
			m_index.reserve(m_index.size() + kIndexAllocAhead);
		}

		m_index.emplace_back(Entry { str, (size_t)len});
	}

	void Lookup(char** ptr, size_t* len, size_t* indices, size_t num, int* sel) const;
};

struct InlineDictionary : Dictionary {
private:
	const size_t kBufferSize = 16*1024;
	BufferFactory m_alloc;
	Buffer* m_head = nullptr;

public:
	void Put(const std::string& w);

	~InlineDictionary() override;
};

#include <boost/iostreams/device/mapped_file.hpp>

struct FileDictionary : Dictionary {
private:
	boost::iostreams::mapped_file_source m_file;
	bool m_file_open;

public:
	FileDictionary(const std::string& file);
	virtual ~FileDictionary();
};

#endif