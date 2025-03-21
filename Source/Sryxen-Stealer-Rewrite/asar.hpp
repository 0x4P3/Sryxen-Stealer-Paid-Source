#ifndef ASAR_HPP
#define ASAR_HPP

#include <fstream>
#include <filesystem>
#include <vector>
#include <string>
#include <functional>
#include <algorithm>
#include "json.hpp"
#include "obfusheader.h"

// Praying that this shit works now

namespace AsarTools {
    using json = nlohmann::json;
    namespace fs = std::filesystem;

    struct FileEntry {
        fs::path path;
        uint64_t size;
        uint64_t offset;
        bool is_file;
    };

    class AsarUnpacker {
        fs::path asar_path_;
        uint64_t base_offset_;
        json header_;

        uint32_t read_le_uint32(std::istream& is) {
            char buffer[4];
            is.read(buffer, 4);
            return static_cast<uint32_t>(static_cast<unsigned char>(buffer[0])) |
                (static_cast<uint32_t>(static_cast<unsigned char>(buffer[1])) << 8) |
                (static_cast<uint32_t>(static_cast<unsigned char>(buffer[2])) << 16) |
                (static_cast<uint32_t>(static_cast<unsigned char>(buffer[3])) << 24);
        }

        void process_directory(const json& current_dir, const fs::path& current_path,
            std::vector<FileEntry>& entries) const {
            if (!current_dir.contains(OBF("files")))
                return;

            const json& files = current_dir[OBF("files")];
            for (auto& [name, entry] : files.items()) {
                fs::path entry_path = current_path / name;

                if (entry.contains(OBF("files"))) {
                    entries.push_back({ entry_path, 0, 0, false });
                    process_directory(entry, entry_path, entries);
                }
                else if (entry.contains(OBF("size")) && entry.contains(OBF("offset"))) {
                    uint64_t size = entry[OBF("size")].get<uint64_t>();
                    uint64_t offset = entry[OBF("offset")].is_string() ?
                        std::stoull(entry[OBF("offset")].get<std::string>()) :
                        entry[OBF("offset")].get<uint64_t>();

                    entries.push_back({ entry_path, size, offset, true });
                }
            }
        }

    public:
        AsarUnpacker(const fs::path& asar_path) : asar_path_(asar_path) {
            std::ifstream file(asar_path, std::ios::binary);
            if (!file) throw std::runtime_error(OBF("Failed to open ASAR file"));

            file.seekg(8);
            const uint32_t base_offset_adjustment = read_le_uint32(file);
            base_offset_ = base_offset_adjustment + 12;
            const uint32_t header_length = read_le_uint32(file);

            std::vector<char> header_data(header_length);
            file.read(header_data.data(), header_length);
            header_ = json::parse(header_data);
        }

        std::vector<FileEntry> list_files() const {
            std::vector<FileEntry> entries;
            process_directory(header_, OBF(""), entries);
            return entries;
        }

        void extract_all(const fs::path& out_dir) const {
            std::ifstream input(asar_path_, std::ios::binary);
            fs::create_directories(out_dir);

            for (const auto& entry : list_files()) {
                if (entry.is_file) {
                    fs::create_directories((out_dir / entry.path).parent_path());
                    input.seekg(base_offset_ + entry.offset);

                    std::ofstream output(out_dir / entry.path, std::ios::binary);
                    std::vector<char> buffer(16384);
                    size_t remaining = entry.size;

                    while (remaining > 0) {
                        const size_t read_size = (std::min)(remaining, buffer.size());
                        input.read(buffer.data(), read_size);
                        output.write(buffer.data(), input.gcount());
                        remaining -= input.gcount();
                    }
                }
            }
        }
    };

    class AsarPacker {
        fs::path in_dir_;
        fs::path out_file_;

        void write_le_uint32(std::ostream& os, uint32_t value) {
            char buffer[4];
            buffer[0] = static_cast<char>(value & 0xFF);
            buffer[1] = static_cast<char>((value >> 8) & 0xFF);
            buffer[2] = static_cast<char>((value >> 16) & 0xFF);
            buffer[3] = static_cast<char>((value >> 24) & 0xFF);
            os.write(buffer, 4);
        }

        json traverse_directory(const fs::path& dir, std::ofstream& temp_file) {
            json result;
            result[OBF("files")] = json::object();

            for (const auto& entry : fs::directory_iterator(dir)) {
                const auto& path = entry.path();
                std::string name = path.filename().string();

                if (entry.is_directory()) {
                    result[OBF("files")][name] = traverse_directory(path, temp_file);
                }
                else {
                    std::ifstream file(path, std::ios::binary | std::ios::ate);
                    if (!file) throw std::runtime_error(OBF("Failed to read input file"));

                    size_t size = static_cast<size_t>(file.tellg());
                    file.seekg(0);
                    size_t offset = static_cast<size_t>(temp_file.tellp());

                    const size_t chunk_size = 16384;
                    std::vector<char> buffer(chunk_size);
                    size_t remaining = size;

                    while (remaining > 0) {
                        size_t read_size = (std::min)(remaining, chunk_size);
                        file.read(buffer.data(), read_size);
                        temp_file.write(buffer.data(), file.gcount());
                        remaining -= file.gcount();
                    }

                    result[OBF("files")][name] = { {OBF("size"), size}, {OBF("offset"), std::to_string(offset)} };
                }
            }
            return result;
        }

    public:
        AsarPacker(const fs::path& in_dir, const fs::path& out_file)
            : in_dir_(in_dir), out_file_(out_file) {
            if (!fs::exists(in_dir) || !fs::is_directory(in_dir)) {
                throw std::runtime_error(OBF("Input directory does not exist"));
            }
        }

        void pack() {
            fs::path temp_path = out_file_.string() + OBF(".tmp");
            {
                std::ofstream temp_file(temp_path, std::ios::binary | std::ios::trunc);
                json header = traverse_directory(in_dir_, temp_file);

                std::string header_str = header.dump();

                std::ofstream out_file(out_file_, std::ios::binary);
                write_le_uint32(out_file, 4);
                write_le_uint32(out_file, header_str.size() + 8);
                write_le_uint32(out_file, header_str.size() + 4);
                write_le_uint32(out_file, header_str.size());
                out_file.write(header_str.data(), header_str.size());

                temp_file.close();
                std::ifstream temp_input(temp_path, std::ios::binary);
                out_file << temp_input.rdbuf();
            }
            fs::remove(temp_path);
        }
    };

    inline void OverWrite(const fs::path& extracted_dir, const fs::path& target_file,
        const std::string& new_content) {
        fs::path full_path = extracted_dir / target_file;
        fs::create_directories(full_path.parent_path());
        std::ofstream file(full_path, std::ios::binary | std::ios::trunc);
        file.write(new_content.data(), new_content.size());
    }
}
#endif