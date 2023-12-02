#include <iostream>
#include <iomanip>
#include <fstream>
#include <vector>
#include <string>
#include <filesystem>
#include <algorithm>
#include <limits>

namespace fs = std::filesystem;
std::string selectedDirectory;

bool selectDirectory();
void listDirectoryContent();
void displayFileHex();
void deleteFile();
void mirrorReflectFile();

bool isValidInput() {
    if (std::cin.fail()) {
        // Clear the error state
        std::cin.clear();
        // Ignore remaining input
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        return false;
    }
    return true;
}

std::string getValidatedStringInput() {
    std::string input;
    getline(std::cin, input);
    while (input.empty() || input.length() > 255) {
        std::cout << "Invalid input. Please try again: ";
        getline(std::cin, input);
    }
    return input;
}

int main() {
    int choice;
    bool directorySelected = false;

    do {
        std::cout << "0 - Exit\n"
                  << "1 - Select directory\n"
                  << "2 - List directory content\n"
                  << "3 - Display file (hexadecimal view)\n"
                  << "4 - Delete file\n"
                  << "5 - Mirror reflect file (byte level)\n"
                  << "Select option: ";

        std::cin >> choice;

        // Clear newline character left in the buffer
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

        // Immediately handle exit option
        if (choice == 0) {
            std::cout << "Exiting...\n";
            break;
        }

        if (!isValidInput() || choice < 0 || choice > 5) {
            std::cout << "Invalid option. Please try again.\n";
            continue;
        }

        if (choice != 1 && !directorySelected) {
            std::cout << "Please select a directory first.\n";
            continue;
        }

        switch (choice) {
            case 1:
                directorySelected = selectDirectory();
                break;
            case 2:
                listDirectoryContent();
                break;
            case 3:
                displayFileHex();
                break;
            case 4:
                deleteFile();
                break;
            case 5:
                mirrorReflectFile();
                break;
            case 0:
                std::cout << "Exiting...\n";
                break;
            default:
                std::cout << "Invalid option. Please try again.\n";
        }
    } while (choice != 0);

    return 0;
}

bool selectDirectory() {
    std::cout << "Enter the absolute path of the directory: ";
    std::string path = getValidatedStringInput();

    if (fs::exists(path) && fs::is_directory(path)) {
        selectedDirectory = path;
        std::cout << "Directory selected: " << selectedDirectory << std::endl;
        return true;
    } else {
        std::cout << "Invalid directory. Please try again." << std::endl;
        return false;
    }
}

void listDirectoryContent() {
    if (selectedDirectory.empty()) {
        std::cout << "No directory selected. Please select a directory first.\n";
        return;
    }

    std::cout << "Contents of directory: " << selectedDirectory << "\n";

    std::vector<std::string> directories;
    std::vector<std::pair<std::string, uintmax_t>> files;

    for (const auto& entry : fs::directory_iterator(selectedDirectory)) {
        if (entry.is_directory()) {
            directories.push_back(entry.path().filename().string());
        } else if (entry.is_regular_file()) {
            files.push_back({entry.path().filename().string(), entry.file_size()});
        }
    }

    std::cout << "\nDirectories:\n";
    for (const auto& dir : directories) {
        std::cout << dir << "\n";
    }

    std::cout << "\nFiles:\n";
    for (const auto& file : files) {
        std::cout << file.first << " (" << file.second << " bytes)\n";
    }
}

void displayFileHex() {
    if (selectedDirectory.empty()) {
        std::cout << "No directory selected. Please select a directory first.\n";
        return;
    }

    std::cout << "Enter the filename: ";
    std::string filename = getValidatedStringInput();

    fs::path filePath = fs::path(selectedDirectory) / filename;

    if (!fs::exists(filePath) || !fs::is_regular_file(filePath)) {
        std::cout << "File does not exist in the selected directory.\n";
        return;
    }

    std::ifstream file(filePath, std::ios::binary);
    if (!file.is_open()) {
        std::cout << "Failed to open the file.\n";
        return;
    }

    unsigned char buffer[16];
    while (!file.eof()) {
        file.read(reinterpret_cast<char*>(buffer), sizeof(buffer));
        std::streamsize bytesRead = file.gcount();

        for (int i = 0; i < bytesRead; ++i) {
            std::cout << std::uppercase << std::hex << std::setw(2) << std::setfill('0')
                      << static_cast<int>(buffer[i]) << ' ';
        }
        std::cout << "\n";
    }

    file.close();
}

void deleteFile() {
    if (selectedDirectory.empty()) {
        std::cout << "No directory selected. Please select a directory first.\n";
        return;
    }

    std::cout << "Enter the filename to delete: ";
    std::string filename = getValidatedStringInput();

    fs::path filePath = fs::path(selectedDirectory) / filename;

    if (!fs::exists(filePath) || !fs::is_regular_file(filePath)) {
        std::cout << "File does not exist in the selected directory.\n";
        return;
    }

    try {
        if (fs::remove(filePath)) {
            std::cout << "File successfully deleted.\n";
        } else {
            std::cout << "Failed to delete the file.\n";
        }
    } catch (const fs::filesystem_error& e) {
        std::cout << "Error: " << e.what() << "\n";
    }
}

// Basic logic harvested and modified from https://stackoverflow.com/a/4245986
unsigned char mirrorByte(unsigned char b) {
    b = (b & 0xF0) >> 4 | (b & 0x0F) << 4;
    b = (b & 0xCC) >> 2 | (b & 0x33) << 2;
    b = (b & 0xAA) >> 1 | (b & 0x55) << 1;
    return b;
}

void mirrorReflectFile() {
    if (selectedDirectory.empty()) {
        std::cout << "No directory selected. Please select a directory first.\n";
        return;
    }

    std::cout << "Enter the filename to mirror reflect: ";
    std::string filename = getValidatedStringInput();

    fs::path filePath = fs::path(selectedDirectory) / filename;

    if (!fs::exists(filePath) || !fs::is_regular_file(filePath)) {
        std::cout << "File does not exist in the selected directory.\n";
        return;
    }

    std::ifstream inputFile(filePath, std::ios::binary);
    if (!inputFile.is_open()) {
        std::cout << "Failed to open the file for reading.\n";
        return;
    }

    std::vector<unsigned char> buffer(std::istreambuf_iterator<char>(inputFile), {});
    inputFile.close();

    std::transform(buffer.begin(), buffer.end(), buffer.begin(), mirrorByte);

    std::ofstream outputFile(filePath, std::ios::binary | std::ios::trunc);
    if (!outputFile.is_open()) {
        std::cout << "Failed to open the file for writing.\n";
        return;
    }

    outputFile.write(reinterpret_cast<const char*>(buffer.data()), buffer.size());
    outputFile.close();

    std::cout << "File has been mirror reflected.\n";
}
