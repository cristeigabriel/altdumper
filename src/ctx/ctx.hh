#pragma once

// ===========================================
#include <vector>
#include <string>
#include <optional>
#include <unordered_map>
#include <Windows.h>
#include "../ptr/ptr.hh"
// ===========================================

// ===========================================
/**
 * @brief Contains all module related structs
 * restrained to context
 * 
 */
namespace modules {
namespace have {
    struct section {
        //
        // DATA
        //

        const char* name = nullptr;
        uintptr_t start  = 0;
        uintptr_t size   = 0;
    };
}  // namespace have

/**
 * @brief Module context structure
 * 
 */
using namespace have;
struct context {
    //
    // CONSTRUCTORS
    //

    context() = default;

    /**
     * @brief Construct a new context object from module handle
     * 
     * @param module Module object
     */
    [[nodiscard]] context(const HMODULE& module);

    /**
     * @brief Construct a new context object from module handle retrieved
     * 
     * @param path Path to load (!) module from, without TLS callbacks, to local process
     */
    [[nodiscard]] context(const std::string& path);

  private:
    //
    // LOCAL
    //

    /**
     * @brief Initialize local data. Exceptions are handled by the user
     * 
     * @param module Module object
     */
    void initialize(const HMODULE& module);

    //
    // DATA
    //

    using bytes  = uint8_t*;
    bytes _bytes = nullptr;
    size_t _size = 0;

    PIMAGE_DOS_HEADER _dos_header = nullptr;
    PIMAGE_NT_HEADERS _nt_headers = nullptr;

    using sections     = std::unordered_map<std::string, section>;
    sections _sections = {};

  public:
    //
    // UTILITY
    //

    inline auto get_bytes() const {
        return _bytes;
    }

    inline auto get_at(uintptr_t n) const {
        return (uintptr_t)_bytes + n;
    }

    inline auto get_byte(uintptr_t n) const {
        return *(uint8_t*)(get_at(n));
    }

    inline auto get_size() const {
        return _size;
    }

    inline auto get_dos_header() const {
        return _dos_header;
    }

    inline auto get_nt_headers() const {
        return _nt_headers;
    }

    [[nodiscard]] inline const auto& get_sections() const {
        return _sections;
    }

    [[nodiscard]] inline const auto& get_section(const std::string& name) const {
        return _sections.at(name);
    }

    /**
     * @brief Find byte array pattern in bytes
     * 
     * @param bytes Dynamic address to said bytes
     * @param size Size of byte array pointed to
     * @param section Module section to scan through
     * @param nth_match N-th selection of a repeating pattern
     * @return std::optional<ptr> Contained pointer
     */
    [[nodiscard]] std::optional<ptr> find_signature(const int* bytes, size_t size, const std::string& section, size_t nth_match) const;

    /**
     * @brief Find null terminated string in .rdata then scan for address in .text
     * 
     * @param bytes The string itself
     * @param size String size
     * @param section Section to scan for references
     * @param reference_instance N-th reference
     * @return std::optional<ptr> Contained pointer
     */
    [[nodiscard]] std::optional<ptr> find_string(const char* bytes, size_t size, const std::string& section, size_t reference_instance) const;

    /**
     * @brief CS:GO/Source-Engine specific - Find ConVar with string by constructor, return pointer
     * 
     * @param bytes The string/convar name itself
     * @param size String size
     * @param server_bounded Constructor type, non-server-bounded example (CS:GO):
     * r_aspectratio, server-bounded example: cl_cmdrate
     * @return std::optional<ptr> Contained pointer
     */
    [[nodiscard]] std::optional<ptr> find_convar(const char* bytes, size_t size, bool server_bounded) const;
};
}  // namespace modules
// ===========================================
