
#include <sstream>
#include <iostream>
#include <filesystem>
#include <fstream>
#include <unordered_map>
#include <vector>
#include <iomanip>
#include <openssl/sha.h>

namespace fs = std::filesystem;

std::string calculateFileHash(const fs::path& filePath) {
    std::ifstream file(filePath, std::ios::binary);
    if (!file.is_open()) {
        throw std::ios_base::failure("Failed to open file: " + filePath.string());
    }

    SHA256_CTX sha256;
    SHA256_Init(&sha256);

    char buffer[4096];
    while (file.read(buffer, sizeof(buffer))) {
        SHA256_Update(&sha256, buffer, file.gcount());
    }
    // Päivitetään loput tiedostosta
    SHA256_Update(&sha256, buffer, file.gcount());

    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256_Final(hash, &sha256);

    std::ostringstream hashString;
    for (int i = 0; i < SHA256_DIGEST_LENGTH; ++i) {
        hashString << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(hash[i]);
    }

    return hashString.str();
}

int main() {
    const std::string directory = "/directory"; // Hakemisto, joka tarkistetaan
    std::unordered_map<std::string, std::vector<std::string>> fileMap;

    try {
        // Käydään läpi tiedostot hakemistossa
        for (const auto& entry : fs::recursive_directory_iterator(directory)) {
            if (fs::is_regular_file(entry.path())) {
                try {
                    std::string hash = calculateFileHash(entry.path());
                    fileMap[hash].push_back(entry.path().string());
                } catch (const std::ios_base::failure& e) {
                    std::cerr << "Virhe tiedoston käsittelyssä: " << e.what() << '\n';
                }
            }
        }

        // Tulostetaan duplikaatit
        std::cout << "Duplikaatit:\n";
        for (const auto& [hash, files] : fileMap) {
            if (files.size() > 1) {
                std::cout << "Hash: " << hash << '\n';
                for (const auto& file : files) {
                    std::cout << "  " << file << '\n';
                }
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "Virhe: " << e.what() << '\n';
    }

    return 0;
}
