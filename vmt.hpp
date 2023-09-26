#ifndef INCLUDED_VMT_HPP_
#define INCLUDED_VMT_HPP_

#include <string>
#include <tuple>
#include <unordered_set>

namespace vmt {

std::tuple<uint64_t*, size_t> Find(const std::string_view& module_name, const std::string_view& type_name,
                                   size_t occurrence = 0);

class Hook {
public:
    size_t table_size_;
    uint64_t* table_array_;
    uint64_t* table_array_copy_;
    std::unordered_set<void*> hooked_instances_;

    explicit Hook(uint64_t* table_array, size_t table_size);

    ~Hook();

    bool SwapIndex(size_t index, void* function);

    bool HookInstance(void* p_instance);

    bool UnhookInstance(void* p_instance);

    void UnhookAll();

private:
};

}  // namespace vmt

#endif  // INCLUDED_VMT_HPP_