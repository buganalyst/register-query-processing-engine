#include "virtual_memory.hpp"

VirtualMemoryManager::VirtualMemoryManager(Storage& storage, std::size_t frameCount, std::size_t rowsPerPage)
    : storage_(storage), rowsPerPage_(rowsPerPage), bufferPool_(frameCount) {}
std::vector<Row> VirtualMemoryManager::fetchPage(const std::string& table, std::size_t page) {
    PageKey key{table, page};
    if (const auto* cached = bufferPool_.get(key)) return cached->rows;
    ++pageFaults_;
    ++diskReads_;
    auto rows = storage_.readPage(table, page, rowsPerPage_);
    auto frame = bufferPool_.put(key, rows);
    pageTable_[key] = frame;
    return rows;
}
std::size_t VirtualMemoryManager::pageFaults() const { return pageFaults_; }
std::size_t VirtualMemoryManager::diskReads() const { return diskReads_; }
std::size_t VirtualMemoryManager::frameCount() const { return bufferPool_.capacity(); }
std::size_t VirtualMemoryManager::rowsPerPage() const { return rowsPerPage_; }
const BufferPool& VirtualMemoryManager::bufferPool() const { return bufferPool_; }
