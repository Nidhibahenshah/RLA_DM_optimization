#include "../src/soundex.h"
#include "../src/util.h"

// Function to calculate Soundex codes for dataset
void calculate_soundex(std::vector<std::unordered_map<std::string, std::string>>& dataset) {
    for (auto& row : dataset) {
        std::string soundex_lastname;
        std::string soundex_firstname;
        soundex(row["Lastname"], soundex_lastname);
        soundex(row["Firstname"], soundex_firstname);
        row["Soundex"] = soundex_lastname + soundex_firstname;
    }
}

// Function to save Soundex codes to a file
void save_soundex_to_file(const std::vector<std::unordered_map<std::string, std::string>>& dataset, const std::string& filename) {
    std::ofstream file(filename);
    if (file.is_open()) {
        file << "ID,Soundex\n";
        for (const auto& row : dataset) {
            file << row.at("ID") << "," << row.at("Soundex") << "\n";
        }
        file.close();
    }
}

void usage() {
    std::cout << "Usage example:" << std::endl;
    std::cout << " ./test_soundex input_file.csv output_file.csv" << std::endl;
}


int main(int argc, char** argv) {

    if (argc < 3) {
        std::cout << "Insufficient program options.";
        usage();
        exit(1);
    }

    std::string input_filename = argv[1];
    std::string output_filename = argv[2];

    std::vector<std::unordered_map<std::string, std::string>> dataset1 = read_csv(input_filename);

    // Calculate Soundex codes
    calculate_soundex(dataset1);

    // Save Soundex codes to files
    save_soundex_to_file(dataset1, output_filename);

    return 0;
}
