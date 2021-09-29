#pragma once

// ===========================================
#include <string>
#include <fstream>
#include <utility>
#include <vector>
// ===========================================

// ===========================================
// TODO: more languages support !!!!!!!!!!
// - make value characteristics a detail
//   held in a language representative
//   so the language gets set as a detail
// ===========================================

// ===========================================
/**
 * @brief Contains all code generation structs
 * restrained to context, and detail methods
 * used for the creation of the aforementioned
 * 
 */
namespace code_gen {
namespace detail {
    enum indent_style {
        TABS,
        SPACES
    };
    template<indent_style style>
    struct basic_indentation;

    template<>
    struct basic_indentation<indent_style::TABS> {
        constexpr static auto value = '\t';
    };

    template<>
    struct basic_indentation<indent_style::SPACES> {
        constexpr static auto value = "    ";
    };

    template<class Lambda, int = (Lambda {}(), 0)>
    constexpr bool is_constexpr(Lambda) {
        return true;
    }

    constexpr bool is_constexpr(...) {
        return false;
    }

    template<typename T>
    constexpr static bool is_constexpr_initable() {
        if constexpr (is_constexpr([]() { return T {}; })) {
            return true;
        }

        return false;
    }
}  // namespace detail
using indentation = detail::basic_indentation<detail::indent_style::TABS>;

/**
* @brief Represents a file
* 
*/
struct context {
    //
    // CONSTRUCTORS
    //

    context() = default;

    /**
     * @brief Construct a new context object
     * 
     * @param path Where the dump will happen
     */
    [[nodiscard]] context(const std::string& path);

    /**
     * @brief Destroy the context object
     * 
     * Closes file
     * 
     */
    ~context();

  private:
    //
    // DATA
    //

    std::string _path   = {};
    std::ofstream _file = {};
    size_t _scopes      = 0;

  public:
    //
    // UTILITY
    //

    inline const auto& get_path() const {
        return _path;
    }

    inline const auto& get_file() const {
        return _file;
    }

    constexpr auto indent(size_t n) {
        for (auto i = 0; i < n; ++i) {
            _file << indentation::value;
        }
    }

    constexpr auto indent() {
        indent(_scopes);
    }

    constexpr auto break_line(size_t n = 1) {
        for (auto i = 0; i < n; ++i) {
            _file << '\n';
        }
    }

    inline auto comment(const std::string& entry) {
        indent();
        _file << "// " << entry << '\n';
    }

    /**
     * @brief List a value to the stream with pretty-fication given
     * conditions and respect to indentation
     * 
     * @tparam T Input and output type of value
     * @param entry_name Value variable name
     * @param value Variable value
     * @return
     */
    template<class T>
    constexpr auto push_value(const std::string& entry_name, T&& value) {
        using raw_type                            = std::remove_cvref_t<T>;
        constexpr auto is_constexpr_constructible = detail::is_constexpr_initable<raw_type>();
        constexpr auto is_integral                = std::is_integral_v<raw_type>;

        indent();

        if (is_integral) {
            _file << std::hex;
        }

        _file << (is_constexpr_constructible ? "constexpr static " : "static ") << typeid(T).name() << ' ' << entry_name << " = " << (is_integral ? "0x" : "") << value << ';' << std::endl;
    }

    /**
     * @brief Open namespace style scope
     * 
     * @param name Namespace name
     */
    void push_namespace(const std::string& name) {
        indent();
        _file << "namespace " << name << " {\n";

        ++_scopes;
    }

    /**
     * @brief Pop last scope
     * 
     */
    void pop_scope() {
        --_scopes;

        indent();

        // ';' isn't necessary, anywho, it isn't detrementary to
        // the build process, so, I guess it'll stay to save some
        // annoyance or extra, not particularly necessary, complexity
        _file << "};\n";
    }
};
}  // namespace code_gen