#ifndef INCLUDED_RTTI_HPP_
#define INCLUDED_RTTI_HPP_

#include <cstdint>

namespace rtti {

struct TypeDescriptor {
    uint64_t rtti_vtable;  // Address of type_info vtable.
    uint64_t reserved;     // Internal runtime reference.
    char type_name[];      // Start of type name string, should be ".?AV".
};

struct BaseClassDescriptor {
    int32_t type_descriptor_rva;         // Rva to type descriptor of the class complete class.
    uint32_t num_contained_bases;        // Number of nested classes in base_class_rva_array_rva.
    int32_t member_displacement;         // Member displacement.
    int32_t vtable_displacement;         // VTable displacement.
    int32_t displacement_within_vtable;  // Displacement within vtable.
    uint32_t attributes;
};

struct ClassHierarchyDescriptor {
    uint32_t signature;                // Seems like its just 0 on 64bit.
    uint32_t attributes;               // Bit 0 set = multiple inheritance, bit 1 set = virtual inheritance.
    uint32_t num_base_classes;         // Number of base class RVAs in base_class_rva_array_rva.
    int32_t base_class_rva_array_rva;  // RVA to array of BaseClassDescriptor RVAs.
};

struct CompleteObjectLocator {
    uint32_t signature;                      // 1 if 64bit, 0 if 32bit.
    uint32_t vtable_offset;                  // Offset of this vtable in the complete class.
    uint32_t constructor_offset;             // Constructor displacement offset.
    int32_t type_descriptor_rva;             // RVA to type descriptor of the complete class.
    int32_t class_hierarchy_descriptor_rva;  // RVA to inheritance hierarchy description.
    int32_t complete_object_locator_rva;     // RVA to the objects base (used to get the base address of the module).
};

}  // namespace rtti

#endif  // INCLUDED_RTTI_HPP_