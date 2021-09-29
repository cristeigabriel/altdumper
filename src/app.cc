/**
 * @file app.cc
 * @author Cristei Gabriel-Marian (cristei.g772@gmail.com)
 * @brief Smart, Fast, JSON-configuration based External Memory Dumper
 * @version 0.1
 * @date 2021-09-26
 * 
 * @copyright Copyright (c) 2021
 * 
 */

// ===========================================
#include <iostream>
#include <vector>
#include <thread>
#include <algorithm>
#include <variant>
#include <array>
#include <fstream>
#include <filesystem>
#include <map>
#include <limits>
#include <Windows.h>
#include <ShlObj.h>
// ===========================================
#include "ctx/ctx.hh"
#include "code_gen/code_gen.hh"
#include "vendor/json/json.hh"
// ===========================================

// ===========================================
namespace indices {
enum {
    exit,
    write,
    make
};
}
// ===========================================

// ===========================================
namespace utility {
namespace json {
    struct signature {
        //
        // CONSTRUCTORS
        //

        signature() = default;

        /**
         * @brief Construct a new signature object from data
         * 
         * @param signature 
         * @param nth_match
         * @param padding 
         * @param dereferences 
         */
        signature(std::string&& signature, size_t nth_match, int padding, int dereferences) {
            _signature    = std::move(signature);
            _nth_match    = nth_match;
            _padding      = padding;
            _dereferences = dereferences;
        }

        /**
         * @brief Construct a new signature object from JSON
         * 
         * @param json Entry
         */
        signature(const nlohmann::json& json) {
            _signature    = std::move(json["signature"].get<std::string>());
            _nth_match    = json["nth-match"].get<size_t>();
            _padding      = json["padding"].get<int>();
            _dereferences = json["dereferences"].get<int>();
        }

      private:
        //
        // DATA
        //

        std::string _signature = {};
        size_t _nth_match      = 0;
        int _padding           = 0;
        int _dereferences      = 0;

      public:
        //
        // UTILITY
        //

        inline auto get_signature() const {
            return _signature;
        }

        inline auto get_nth_match() const {
            return _nth_match;
        }

        inline auto get_padding() const {
            return _padding;
        }

        inline auto get_dereferences() const {
            return _dereferences;
        }

        //
        // EXPORT
        //

        static nlohmann::json to_json(signature&& object) {
            nlohmann::json json;

            json["signature"]    = object.get_signature();
            json["nth-match"]    = object.get_nth_match();
            json["padding"]      = object.get_padding();
            json["dereferences"] = object.get_dereferences();

            return json;
        }
    };

    // TODO: recursive follow tables
    struct string_search {
        //
        // CONSTRUCTORS
        //

        string_search() = default;

        /**
         * @brief Construct a new string search object from data
         * 
         * @param string Null terminated
         * @param reference_instance 
         * @param padding 
         * @param dereferences 
         */
        string_search(std::string&& string, size_t reference_instance, int padding, int dereferences) {
            _string             = std::move(string);
            _reference_instance = reference_instance;
            _padding            = padding;
            _dereferences       = dereferences;
        }

        /**
         * @brief Construct a new string search object from JSON
         * 
         * @param json Entry
         */
        string_search(const nlohmann::json& json) {
            _string             = std::move(json["string"].get<std::string>());
            _reference_instance = json["reference-instance"].get<size_t>();
            _padding            = json["padding"].get<int>();
            _dereferences       = json["dereferences"].get<int>();
        }

      private:
        //
        // DATA
        //

        std::string _string        = {};
        size_t _reference_instance = 0;
        int _padding               = 0;
        int _dereferences          = 0;

      public:
        //
        // UTILITY
        //

        inline auto get_string() const {
            return _string;
        }

        inline auto get_reference_instance() const {
            return _reference_instance;
        }

        inline auto get_padding() const {
            return _padding;
        }

        inline auto get_dereferences() const {
            return _dereferences;
        }

        //
        // EXPORT
        //

        static nlohmann::json to_json(string_search&& object) {
            nlohmann::json json;

            json["string"]             = object.get_string();
            json["reference-instance"] = object.get_reference_instance();
            json["padding"]            = object.get_padding();
            json["dereferences"]       = object.get_dereferences();

            return json;
        }
    };
}  // namespace json
namespace winapi {
    [[nodiscard]] auto get_file_from_prompt() {
        char file[MAX_PATH] = {};

        OPENFILENAMEA info = {};
        ZeroMemory(&info, sizeof(info));

        info.lStructSize     = sizeof(info);
        info.hwndOwner       = NULL;
        info.lpstrFile       = file;
        info.lpstrFile[0]    = '\0';
        info.nMaxFile        = sizeof(file);
        info.lpstrFilter     = "All\0*.*\0JSON\0*.JSON\0DLLs\0*.DLL\0";
        info.nFilterIndex    = 1;
        info.lpstrFileTitle  = NULL;
        info.nMaxFileTitle   = 0;
        info.lpstrInitialDir = NULL;
        info.Flags           = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

        GetOpenFileNameA(&info);
        return std::string {file};
    }

    [[nodiscard]] auto get_folder_from_prompt() {
        char folder[MAX_PATH] = {};

        BROWSEINFO info = {};
        ZeroMemory(&info, sizeof(info));

        info.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;

        const auto pidl = SHBrowseForFolder(&info);
        if (pidl != 0) {
            SHGetPathFromIDListA(pidl, folder);
            CoTaskMemFree(pidl);

            return std::string {folder};
        }

        return std::string {};
    }
}  // namespace winapi
}  // namespace utility
// ===========================================

// ===========================================

// There may be "alternation" between
// '\n' and std::endl. It is meant
// to be as such. We're meaning to
// flush the buffer when we do that.

namespace functions {
namespace handlers {
    auto add_signature(nlohmann::json& section) {
        std::cout << "Entry:\n";
        std::string entry = {};
        std::getline(std::cin >> std::ws, entry);

        std::cout << "Signature:\n";
        std::string signature = {};
        std::getline(std::cin >> std::ws, signature);

    nth_match_label:
        std::cout << "N-th match (starts from 0):\n";
        size_t nth_match = 0;
        std::cin >> nth_match;

        if (std::cin.fail()) {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<int>::max(), '\n');
            goto nth_match_label;
        }

    padding_label:
        std::cout << "Padding:\n";
        int padding = 0;
        std::cin >> padding;

        if (std::cin.fail()) {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<int>::max(), '\n');
            goto padding_label;
        }

    dereferences_label:
        std::cout << "Dereferences:\n";
        int dereferences = 0;
        std::cin >> dereferences;

        if (std::cin.fail()) {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<int>::max(), '\n');
            goto dereferences_label;
        }

        section[entry] = utility::json::signature::to_json({std::move(signature), nth_match, padding, dereferences});
    }

    auto add_string_search(nlohmann::json& section) {
        std::cout << "Entry:\n";
        std::string entry = {};
        std::getline(std::cin >> std::ws, entry);

        std::cout << "String (null terminated):\n";
        std::string string = {};
        std::getline(std::cin >> std::ws, string);

    reference_instance_label:
        std::cout << "Reference instance:\n";
        size_t reference_instance = 0;
        std::cin >> reference_instance;

        if (std::cin.fail()) {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<int>::max(), '\n');
            goto reference_instance_label;
        }

    padding_label:
        std::cout << "Padding:\n";
        int padding = 0;
        std::cin >> padding;

        if (std::cin.fail()) {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<int>::max(), '\n');
            goto padding_label;
        }

    dereferences_label:
        std::cout << "Dereferences:\n";
        int dereferences = 0;
        std::cin >> dereferences;

        if (std::cin.fail()) {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<int>::max(), '\n');
            goto dereferences_label;
        }

        section[entry] = utility::json::string_search::to_json({std::move(string), reference_instance, padding, dereferences});
    }
}  // namespace handlers
[[nodiscard]] int exit() {
    system("pause");
    std::exit(EXIT_SUCCESS);
    return EXIT_SUCCESS;
}

[[nodiscard]] int write() {
    // entry
    std::cout << "You're going to be walked through the process of creating a JSON entry of your choice. You can always manually create one, edit an existing one, or come back here to create one, step by step.\n";

    // data
    int level                       = 0;
    std::vector<std::string> pushed = {};
    nlohmann::json json             = {};

    // levels
    enum levels {
        prompt,
        push
    };

here:
    if (level == levels::prompt) {
        // separated print call for logic
        std::cout << "To stop this process, press \"Cancel\" on the dialogue. Generation will then commence.\nSelect a module:\nCurrent ones:\n";

        // perhaps i should iterators. . . . . perhaps
        for (auto i = 0; i < pushed.size(); ++i) {
            std::cout << pushed[i] << ((i == (pushed.size() - 1)) ? ".\n" : ", ");
        }

        auto&& entry = utility::winapi::get_file_from_prompt();
        if (entry.empty()) {
            std::cout << "Finished. You will now be prompted to select a folder to where your config will be saved:\n";

            auto&& folder = utility::winapi::get_folder_from_prompt();

            std::cout << "Name (no extension):\n";
            std::string name = {};
            std::cin >> name;

            auto&& path = folder + (name + ".json");
            std::cout << path << '\n';

            // save json
            std::ofstream config(path);
            config << std::setw(4) << json << '\n';
            config.close();

            return EXIT_SUCCESS;
        } else if (std::ranges::find(pushed, entry) != pushed.cend()) {
            std::cout << "Entry already present." << std::endl;
            goto here;
        } else {
            pushed.push_back(entry);
            level = levels::push;
            goto here;
        }
    } else if (level == levels::push) {
        auto&& name = pushed.back();

        // initialize node
        auto& node = json[name];

        // initialize sections
        auto& signatures    = node["signatures"];
        auto& string_search = node["string-search"];

    restart:
        // TODO: misc push-levels for ConVars, NetVars (CS:GO/...)
        enum push_levels {
            go_back = 1,
            add_signature,
            add_string_search
        };

        std::cout << "Pushing to \"" << name << "\".\nTo go back to module prompting, input \"" << push_levels::go_back << "\". To import a signature, input \"" << push_levels::add_signature << "\". To import a string search, input \"" << push_levels::add_string_search << "\".\n";

        int indice = -1;
        std::cin >> indice;

        switch (indice) {
            case push_levels::go_back: {
                level = levels::prompt;
                goto here;
            } break;
            case push_levels::add_signature: {
                handlers::add_signature(signatures);
            } break;
            case push_levels::add_string_search: {
                handlers::add_string_search(string_search);
            } break;
        }

        goto restart;
    }

    return EXIT_SUCCESS;
}

[[nodiscard]] int make() {
    std::cout << "Provide config file:\n";
    auto&& config_name = utility::winapi::get_file_from_prompt();

    // open stream
    std::ifstream file(config_name);

    // parse stream as json
    const nlohmann::json& config = nlohmann::json::parse(file);

    // make file editable/free again, as contents've been copied
    file.close();

    // final data container
    std::map<std::string, std::map<std::string, uintptr_t>> addresses = {};

    std::vector<std::thread> thread_pool = {};

    // multi-threaded process
    for (const auto& [key, value] : config.items()) {
        // this is done here to order DLL entries by JSON order,
        // not by which thread runs first
        auto& map_entry_key = addresses[key];

        // couldn't uniformly initialize a std::function, for some reason, didn't give it
        // much thought either. shouldn't be too bad, anyhow/anywho
        const std::function<void()>& work = [&]() {
            modules::context dll(key);

            const auto& signatures    = value["signatures"];
            const auto& string_search = value["string-search"];

            for (const auto& [key, value] : signatures.items()) {
                const auto& data = utility::json::signature(value);

                // credits for this runtime solution: https://github.com/spirthack/CSGOSimple
                // TODO: look into making this better
                // offtopic: for an alternative, compile-time solution, refer to:
                // https://github.com/cristeigabriel/STB
                static auto pattern_to_bytes = [](std::string&& pattern) {
                    auto bytes = std::vector<int> {};
                    auto start = pattern.data();
                    auto end   = pattern.data() + pattern.size();

                    for (auto current = start; current < end; ++current) {
                        if (*current == '?') {
                            ++current;
                            if (*current == '?')
                                ++current;
                            bytes.push_back(-1);
                        } else {
                            bytes.push_back(strtoul(current, &current, 16));
                        }
                    }

                    return bytes;
                };

                uintptr_t address = 0;

                auto&& vec      = pattern_to_bytes(data.get_signature());
                const auto& sig = dll.find_signature(vec.data(), vec.size(), ".text", data.get_nth_match());
                if (sig.has_value()) {
                    address = (sig.value().padded(data.get_padding()).dereferenced(data.get_dereferences()).get() - (uintptr_t)dll.get_bytes());
                } else {
                    // well, we can still continue. but, this is decided by
                    // the one who handles the errors. rawly, upon catches we
                    // just
                    throw std::runtime_error("Failed finding pattern.");
                }

                map_entry_key[key] = address;
            }

            for (const auto& [key, value] : string_search.items()) {
                const auto& data = utility::json::string_search(value);

                uintptr_t address = 0;

                auto&& str      = data.get_string();
                const auto& ptr = dll.find_string(str.c_str(), str.size(), ".text", data.get_reference_instance());
                if (ptr.has_value()) {
                    address = (ptr.value().padded(data.get_padding()).dereferenced(data.get_dereferences()).get() - (uintptr_t)dll.get_bytes());
                } else {
                    throw std::runtime_error("Failed finding string.");
                }

                map_entry_key[key] = address;
            }
        };

        thread_pool.push_back(std::thread {work});
    }

    // run thread pool
    for (auto& thread : thread_pool) {
        if (thread.joinable()) {
            thread.join();
        }
    }

    // get saved output folder
    std::cout << "You'll be prompted to provide a folder where your code generation result will be saved:\n";
    auto&& path = utility::winapi::get_folder_from_prompt();

    // store file name
    std::cout << "Input a file name (with extension):\n";
    std::string file_name = {};
    std::getline(std::cin >> std::ws, file_name);

    // initialize code generation context with filesystem input
    code_gen::context output(path + file_name);

    // intro`
    output.break_line(2);
    output.comment("altdumper - " __TIMESTAMP__);
    output.break_line(2);

    // namespace/scope for the whole context
    output.comment(config_name);
    output.push_namespace("altdumper");

    // values will all be addresses, and we want them to be printed
    // in hexadecimal, for ease
    // TODO: consider making the spew JSON? so it's inherently more
    // universal than the code-gen allows for
    std::cout << std::hex;
    for (const auto& [dll, entries] : addresses) {
        // serialize name
        auto begin             = dll.rfind("\\") + 1;
        auto&& serialized_name = dll.substr(begin);
        auto end               = serialized_name.rfind(".");
        serialized_name        = serialized_name.substr(0, end);

        // start namespace/scope with dll name with no extensions, comment
        // full path right before
        output.comment(dll);
        output.push_namespace(serialized_name);

        std::cout << "[+] " << dll << " (" << serialized_name << ")\n";

        for (const auto& [entry, value] : entries) {
            output.push_value(entry, value);
            std::cout << "[-]\t" << entry << '=' << value << '\n';
        }

        // pop dll namespace/scope
        output.pop_scope();
    }

    output.pop_scope();

    // pop whole context namespace/scope

    return EXIT_SUCCESS;
}
}  // namespace functions
// ===========================================

// ===========================================
/**
 * @brief Process which option to take on
 * 
 * @param indice User input
 * @return int Status
 */
[[nodiscard]] int function(int indice) {
    switch (indice) {
        case indices::exit: {
            return functions::exit();
        } break;
        case indices::write: {
            return functions::write();
        } break;
        case indices::make: {
            return functions::make();
        } break;
    }

    return EXIT_FAILURE;
}
// ===========================================

/**
 * @brief Dispatcher
 * 
 * @return int Result
 */
int main() {
    try {
    here:
        // entry dialogue
        std::cout
            << "Hello. Here are the following options:\n- exit: "
            << indices::exit
            << " (Exits the program)"
               "\n- write: "
            << indices::write
            << " (Generate JSON entry)"
               "\n- make: "
            << indices::make
            << " (Spew addresses, prompt code generation)\n";

        // process
        int indice = EXIT_FAILURE;
        std::cin >> indice;

        const auto result = function(indice);
        if (result == EXIT_FAILURE) {
            // flushes buffer
            std::cout << "(!) This option is not supported. Please retry!"
                      << std::endl;
            goto here;
        }

        system("pause");
        return result;
    } catch (const std::exception& err) {
        _RPT0(_CRT_ERROR, err.what());
        std::cout << err.what() << std::endl;
    }

    // what the sus
    // yeah, this isn't needed, just here to avoid some (possible) warning.
    return EXIT_FAILURE;
}