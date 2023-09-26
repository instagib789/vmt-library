#include "Vmt.hpp"

#include "module.hpp"
#include "rtti.hpp"

std::tuple<uint64_t*, size_t> vmt::Find(const std::string_view& module_name, const std::string_view& type_name,
                                        size_t occurrence) {
    // Get module and section information.
    auto [module_address, module_size] = module::GetModuleAddress(module_name);
    if (module_address == 0) {
        return std::make_tuple(nullptr, 0);
    }
    auto [text, text_size] = module::GetSectionAddress(module_address, ".text");
    auto [rdata, rdata_size] = module::GetSectionAddress(module_address, ".rdata");
    auto [data, data_size] = module::GetSectionAddress(module_address, ".data");
    if (text == 0 || rdata == 0 || data == 0) {
        return std::make_tuple(nullptr, 0);
    }

    // Lambda to check if an address is a virtual method table.
    auto is_vmt = [&](uint64_t scan_address, std::string_view& found_type_name) {
        // VTables start with an address to its CompleteObjectLocator and are followed by all its function addresses.
        uint64_t col_address = *reinterpret_cast<uint64_t*>(scan_address - sizeof(uint64_t));
        uint64_t table_address = *reinterpret_cast<uint64_t*>(scan_address);

        // Checks to determine if the address is the start of a vmt.
        if (col_address < rdata || col_address >= rdata + rdata_size) {
            return false;
        }
        if (table_address < text || table_address >= text + text_size) {
            return false;
        }
        auto* col = reinterpret_cast<rtti::CompleteObjectLocator*>(col_address);
        if (col->signature != 1) {
            return false;
        }
        uint64_t td_address = module_address + col->type_descriptor_rva;
        if (td_address < data || td_address >= data + data_size) {
            return false;
        }
        auto* td = reinterpret_cast<rtti::TypeDescriptor*>(td_address);
        if (td->type_name[0] != '.' || td->type_name[1] != '?' || td->type_name[2] != 'A' || td->type_name[3] != 'V') {
            return false;
        }

        found_type_name = std::string_view(td->type_name);
        return true;
    };

    // Search all .rdata section for vtable with type_name.
    size_t found_occurrence = 0;
    for (uint64_t scan_address = rdata; scan_address < rdata + rdata_size; scan_address += 8) {
        std::string_view found_type_name;
        if (is_vmt(scan_address, found_type_name)) {
            // Compare the found type_name.
            if (type_name.compare(found_type_name) == 0) {
                // It's a match! Now check occurrence and find the table size.
                if (occurrence == found_occurrence) {
                    uint64_t table_address = scan_address;
                    size_t table_size = 0;
                    while (*reinterpret_cast<uint64_t*>(scan_address) >= text &&
                           *reinterpret_cast<uint64_t*>(scan_address) < text + text_size) {
                        ++table_size;
                        scan_address += 8;
                    }
                    return std::make_tuple(reinterpret_cast<uint64_t*>(table_address), table_size);
                }
                ++found_occurrence;
            }
        }
    }
    return std::make_tuple(nullptr, 0);
}

vmt::Hook::Hook(uint64_t* table_array, size_t table_size) : table_array_(table_array), table_size_(table_size) {
    table_array_copy_ = new uint64_t[table_size_];
    memcpy(table_array_copy_, table_array_, table_size_ * sizeof(uint64_t));
};

vmt::Hook::~Hook() {
    UnhookAll();
    delete[] table_array_copy_;
};

bool vmt::Hook::SwapIndex(size_t index, void* function) {
    if (index >= table_size_) {
        return false;
    }
    table_array_copy_[index] = reinterpret_cast<uint64_t>(function);
    return true;
};

bool vmt::Hook::HookInstance(void* p_instance) {
    if (*reinterpret_cast<uint64_t**>(p_instance) != table_array_) {
        return false;
    }
    hooked_instances_.emplace(p_instance);
    *reinterpret_cast<uint64_t**>(p_instance) = table_array_copy_;
    return true;
};

bool vmt::Hook::UnhookInstance(void* p_instance) {
    if (*reinterpret_cast<uint64_t**>(p_instance) != table_array_copy_) {
        return false;
    }
    hooked_instances_.erase(p_instance);
    *reinterpret_cast<uint64_t**>(p_instance) = table_array_;
    return true;
}

void vmt::Hook::UnhookAll() {
    for (void* p_instance : hooked_instances_) {
        if (*reinterpret_cast<uint64_t**>(p_instance) == table_array_copy_) {
            *reinterpret_cast<uint64_t**>(p_instance) = table_array_;
        }
    }
    hooked_instances_.clear();
}