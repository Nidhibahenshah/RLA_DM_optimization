

#include "soundex.h"

/// @brief Function to calculate Soundex code for a name
/// @param size the size for truncating the soundex_code
void soundex(const std::string& name, std::string& soundex_code, int size) {
    std::string soundex_digits = "01230120022455012623010202";
                            //   "ABCDEFGHIJKLMNOPQRSTUVWXYZ"

    // Convert to uppercase
    std::string uppercase_name = name;
    std::transform(uppercase_name.begin(), uppercase_name.end(), uppercase_name.begin(), ::toupper);

    // Save the first letter
    soundex_code += uppercase_name[0];

    char prev_code = '\0';
    for (size_t i = 1; i < uppercase_name.length(); ++i) {
        if ( (uppercase_name[i] < 'A') || (uppercase_name[i] > 'Z') )
            continue;
        char code = soundex_digits[uppercase_name[i] - 'A'];
        if (code != prev_code) {
            soundex_code += code;
        }
        prev_code = code;
    }

    // Remove vowels and 'H', 'W', 'Y' from the Soundex code
    soundex_code.erase(std::remove(soundex_code.begin(), soundex_code.end(), '0'),
                       soundex_code.end());

    // Keep the first 4 characters
    soundex_code.resize(size, '0');
}
