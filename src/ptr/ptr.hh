#pragma once

// ===========================================
#include <type_traits>
#include <numeric>
#include <limits>
#include <stdexcept>
// ===========================================

// ===========================================
/**
 * @brief Contains all altdumper pointer
 * permutations which are used in itself
 * and can convert types assignable to a
 * pointer to itself
 * 
 */
struct ptr {
    //
    // CONSTRUCTORS
    //

    ptr() = default;

    /**
     * @brief Construct a new ptr object from an integral/pointer
     * 
     * @tparam T Passed argument type
     * @param address 
     */
    template<typename T>
    requires std::is_integral_v<T> || std::is_pointer_v<T>
    [[nodiscard]] inline ptr(T address)
        : _address((uintptr_t)address) {}

  private:
    //
    // DATA
    //

    uintptr_t _address;

  public:
    //
    // ENUMS
    //

    enum direction : bool {
        back,
        forward
    };

  public:
    //
    // UTILITY
    //

    static inline bool valid(uintptr_t address) {
        return ((address > 0) && (address <= std::numeric_limits<uintptr_t>::max()));
    }

    inline auto valid() const {
        return valid(_address);
    }

    inline auto pad(int padding) {
        _address += padding;
    }

    [[nodiscard]] inline auto padded(int padding) const {
        auto _this = *this;
        _this.pad(padding);
        return _this;
    }

    inline auto dereference(size_t n) {
        auto out = _address;
        for (auto i = 0; i < n; ++i) {
            if (!valid(out)) {
                throw std::runtime_error("Failed dereferencing.");
                break;
            }

            out = *(uintptr_t*)out;
        }

        _address = out;
    }

    [[nodiscard]] inline auto dereferenced(size_t n) const {
        auto _this = *this;
        _this.dereference(n);
        return _this;
    }

    template<typename T = decltype(_address)>
    constexpr auto get() const {
        return (T)_address;
    }

    /**
     * @brief Get the byte at current address
     * 
     * @param n Indice from address
     * @return 
     */
    inline auto get_byte(int n = 0) const {
        return *(uint8_t*)((uintptr_t)_address + n);
    }

    /**
     * @brief Move address backwards/forwards until a specified byte is met
     * 
     * @param byte Byte to match
     * @param where Back/Forward
     * @return
     */
    inline auto follow_until(uint8_t byte, direction where) {
        do {
            pad(where == direction::back ? -1 : 1);
        } while (get_byte() != byte && valid());
    }

    /**
     * @brief Move address backwards/forwards in a copy of the structure until a specified byte is met
     * 
     * @param byte Byte to match
     * @param where Back/Forward
     * @return ptr Copy of current structure with aforementioned modifications
     */
    [[nodiscard]] inline auto followed_until(uint8_t byte, direction where) const {
        auto _this = *this;
        _this.follow_until(byte, where);
        return _this;
    }
};
// ===========================================
