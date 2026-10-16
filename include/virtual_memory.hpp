#pragma once
#include "buffer_pool.hpp"
#include "storage.hpp"
#include <unordered_map>

class VirtualMemoryManager {
public:
    VirtualMemoryManager(Storage& storage, std::size_t frameCount, std::size_t rowsPerPage);
    std::vector<Row> fetchPage(const std::string& table, std::size_t page);
    std::size_t pageFaults() const;
    std::size_t diskReads() const;
    std::size_t frameCount() const;
    std::size_t rowsPerPage() const;
    const BufferPool& bufferPool() const;
private:
    Storage& storage_;
    std::size_t rowsPerPage_;
    BufferPool bufferPool_;
    std::unordered_map<PageKey, std::size_t, PageKeyHash> pageTable_;
    std::size_t pageFaults_ = 0;
    std::size_t diskReads_ = 0;
};
