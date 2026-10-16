#pragma once
#include "types.hpp"
#include <list>
#include <unordered_map>
#include <vector>

struct CachedPage {
    PageKey key;
    std::vector<Row> rows;
    std::size_t frame = 0;
};

class BufferPool {
public:
    explicit BufferPool(std::size_t capacity);
    bool contains(const PageKey& key) const;
    const CachedPage* get(const PageKey& key);
    std::size_t put(const PageKey& key, const std::vector<Row>& rows);
    std::size_t capacity() const;
    std::size_t size() const;
    std::size_t hits() const;
    std::size_t misses() const;
    std::size_t evictions() const;
private:
    std::size_t capacity_;
    std::size_t nextFrame_ = 0;
    std::size_t hits_ = 0;
    std::size_t misses_ = 0;
    std::size_t evictions_ = 0;
    std::list<PageKey> lru_;
    struct Entry { CachedPage page; std::list<PageKey>::iterator position; };
    std::unordered_map<PageKey, Entry, PageKeyHash> cache_;
};
