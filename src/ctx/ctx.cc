/**
 * @file ctx.cc
 * @author Cristei Gabriel-Marian (cristei.g772@gmail.com)
 * @brief Modules Context
 * @version 0.1
 * @date 2021-09-26
 * 
 * @copyright Copyright (c) 2021
 * 
 */

// ===========================================
#include "ctx.hh"
#include <stdexcept>
#include <algorithm>
#include <utility>
#include <array>
// ===========================================

// ===========================================
namespace detail {
constexpr uint16_t byte_swap_16bit(uint16_t bytes) {
    return (uint16_t)(bytes << 8) | (uint16_t)(bytes >> 8);
}

constexpr auto to_array_32bit(uint32_t bytes) {
    return std::array<uint8_t, 4> {(uint8_t)((uint32_t)(bytes & 0xff000000) >> 24), (uint8_t)((uint32_t)(bytes & 0x00ff0000) >> 16), (uint8_t)((uint32_t)(bytes & 0x0000ff00) >> 8), (uint8_t)((uint32_t)(bytes & 0x000000ff))};
}

constexpr auto endianness_swap_32bit(uint32_t bytes) {
    return 0x10000 * ((uint32_t)(byte_swap_16bit((uint16_t)((bytes & 0x0000ffff))))) + (uint32_t)(byte_swap_16bit((uint16_t)((uint32_t)((bytes & 0xffff0000)) >> 16)));
}

template<typename Y, typename X, size_t N, template<typename, size_t> typename A, size_t... Is>
constexpr A<Y, N> cast_array_impl(const A<X, N>& a, std::index_sequence<Is...>) {
    return {std::get<Is>(a)...};
}

template<typename Y, typename X, size_t N, template<typename, size_t> typename A, typename Indices = std::make_index_sequence<N>>
constexpr A<Y, N> cast_array(const A<X, N>& a) {
    return cast_array_impl<Y>(a, Indices {});
}
}  // namespace detail
// Not particularly needed but I'd like the exception namings to be accurate, so they get syntactically checked
#define stringify(x) #x

using namespace modules;
context::context(const HMODULE& module) {
    initialize(module);
}

context::context(const std::string& path) {
    if (auto module = LoadLibraryExA(path.c_str(), nullptr, DONT_RESOLVE_DLL_REFERENCES); module) {
        initialize(module);
    } else {
        throw std::runtime_error("Failed loading " + path);
    }
}

void context::initialize(const HMODULE& module) {
    _bytes = (bytes)module;
    if (!_bytes) {
        throw std::runtime_error(stringify(_bytes) " is null.");
    }

    _dos_header = (PIMAGE_DOS_HEADER)_bytes;
    if (!_dos_header) {
        throw std::runtime_error(stringify(_dos_header) " is null.");
    }

    _nt_headers = (PIMAGE_NT_HEADERS)((uintptr_t)_bytes + (uintptr_t)_dos_header->e_lfanew);
    if (!_nt_headers) {
        throw std::runtime_error(stringify(_nt_headers) " is null.");
    }

    _size = _nt_headers->OptionalHeader.SizeOfImage;
    if (_size <= 0) {
        throw std::runtime_error(stringify(_size) " is <= 0. Perhaps wrong arch binary?");
    }

    auto section_count = _nt_headers->FileHeader.NumberOfSections;
    _sections.reserve(section_count);

    auto section_list = IMAGE_FIRST_SECTION(_nt_headers);
    for (auto i = 0; i < section_count; ++i) {
        const char* name              = (const char*)section_list->Name;
        _sections[std::string {name}] = section {name, section_list->PointerToRawData, section_list->SizeOfRawData};
        ++section_list;
    }

    if (_sections.empty()) {
        throw std::runtime_error(stringify(_sections) " is empty.");
    }
}

std::optional<ptr> context::find_signature(const int* bytes, size_t size, const std::string& section, size_t nth_match) const {
    size_t match = 0;

    uintptr_t start = 0;
    uintptr_t end   = _size;

    if (_sections.contains(section)) {
        const auto& value = get_section(section);
        start             = value.start;
        end               = value.size;
    }

    end -= size;

    for (auto i = start; i < (start + end); ++i) {
        auto found = true;
        for (auto j = 0; j < size; ++j) {
            if (get_byte(i + j) != bytes[j] && bytes[j] != -1) {
                found = false;
                break;
            }
        }

        if (found) {
            if (match != nth_match) {
                ++match;
                continue;
            }

            return ptr(&_bytes[i]);
        }
    }

    return std::nullopt;
}

std::optional<ptr> context::find_string(const char* bytes, size_t size, const std::string& section, size_t reference_instance) const {
    // hacky solution . . . . . .
    std::vector<int> casted = {};
    casted.reserve(size + 1);

    for (auto i = 0; i <= size; ++i) {
        casted.push_back(bytes[i]);
    }

    auto string_find = find_signature(casted.data(), casted.size(), ".rdata", 0);
    if (string_find.has_value()) {
        auto pattern = detail::cast_array<int>(detail::to_array_32bit(detail::endianness_swap_32bit(string_find.value().get())));
        return find_signature(pattern.data(), pattern.size(), section, reference_instance);
    } else {
        throw std::runtime_error("Failed finding string in .rdata.");
    }

    return std::nullopt;
}

std::optional<ptr> context::find_procedure(const std::string& name) const {
    auto procedure = ptr(GetProcAddress((HMODULE)get_bytes(), name.c_str()));
    if (procedure.valid()) {
        return procedure;
    }

    return std::nullopt;
}

std::optional<ptr> context::find_convar(const char* bytes, size_t size, bool server_bounded) const {
    size_t count         = 0;
    auto constructor_ref = find_string(bytes, size, ".text", count++);

    if (constructor_ref.has_value()) {
        int pad        = (server_bounded ? -6 : 4);
        uint8_t opcode = (server_bounded ? 0x68 : 0xE8);

        while (constructor_ref.value().get_byte(pad) != opcode) {
            constructor_ref = find_string(bytes, size, ".text", count++);
        }

        auto bounded_found = constructor_ref.value().followed_until(0xC7, server_bounded ? ptr::direction::forward : ptr::direction::back);
        auto final_found   = (server_bounded ? bounded_found : bounded_found.followed_until(0xB9, ptr::direction::forward));

        if (!final_found.valid()) {
            return std::nullopt;
        }

        return final_found.padded(1 + (int)server_bounded);
    } else {
        throw std::runtime_error("Failed finding .text.");
    }

    return std::nullopt;
}
// ===========================================
