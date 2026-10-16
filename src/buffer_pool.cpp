#include "buffer_pool.hpp"
#include <stdexcept>

BufferPool::BufferPool(std::size_t capacity) : capacity_(capacity) {
    if (capacity_ == 0) throw std::runtime_error("Buffer pool must have at least one frame");
}
bool BufferPool::contains(const PageKey& key) const { return cache_.find(key) != cache_.end(); }
const CachedPage* BufferPool::get(const PageKey& key) {
    auto found = cache_.find(key);
    if (found == cache_.end()) { ++misses_; return nullptr; }
    ++hits_;
    lru_.erase(found->second.position);
    lru_.push_front(key);
    found->second.position = lru_.begin();
    return &found->second.page;
}
std::size_t BufferPool::put(const PageKey& key, const std::vector<Row>& rows) {
    auto existing = cache_.find(key);
    if (existing != cache_.end()) {
        existing->second.page.rows = rows;
        lru_.erase(existing->second.position);
        lru_.push_front(key);
        existing->second.position = lru_.begin();
        return existing->second.page.frame;
    }
    std::size_t frame;
    if (cache_.size() < capacity_) frame = nextFrame_++;
    else {
        PageKey victim = lru_.back();
        auto victimEntry = cache_.find(victim);
        frame = victimEntry->second.page.frame;
        cache_.erase(victimEntry);
        lru_.pop_back();
        ++evictions_;
    }
    lru_.push_front(key);
    CachedPage page{key, rows, frame};
    cache_.emplace(key, Entry{page, lru_.begin()});
    return frame;
}
std::size_t BufferPool::capacity() const { return capacity_; }
std::size_t BufferPool::size() const { return cache_.size(); }
std::size_t BufferPool::hits() const { return hits_; }
std::size_t BufferPool::misses() const { return misses_; }
std::size_t BufferPool::evictions() const { return evictions_; }
