#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <assert.h>
#include "double_metaphone.h"
#include <cstring> // For memset
#include <string>
#include <array>

const unsigned int max_length = 32;
const unsigned int pad = 5;  // Define padding at the front
static bool SlavoDone = false;
static bool SlavoValue = false;

const bool* InitializeVowelArray() {
    static bool vowel[256];
    memset(vowel, false, sizeof(vowel)); // Set all elements to false

    // Mark vowels as true
    vowel['A'] = true;
    vowel['E'] = true;
    vowel['I'] = true;
    vowel['O'] = true;
    vowel['U'] = true;
    vowel['Y'] = true;

    return vowel;
}

// Sentinel values updated to use 127 instead of 255
const char MaxChar = 127;               // Maximum positive value for signed char
const std::string EndOfList(5, MaxChar); 

// Modified sorted arrays using MaxChar and EndOfList
const std::string ACH_LIST[2] = {"CH", EndOfList};
const std::string BACHER_MACHER_LIST[3] = {"ACHER", "MACHER", EndOfList};
const std::string CAESAR_LIST[2] = {"AESAR", EndOfList};
const std::string CHIA_LIST[2] = {"HIA", EndOfList};
const char CH_LIST[2] = {'H', MaxChar};
const std::string CHAE_LIST[2] = {"HAE", EndOfList};
const std::string HARAC_HARIS_LIST[3] = {"ARAC", "ARIS", EndOfList};
const std::string HOR_HYM_HIA_HEM_LIST[5] = {"EM", "IA", "OR", "YM", EndOfList};
const std::string CHORE_LIST[2] = {"HORE", EndOfList};
const std::string VAN_VON_LIST[3] = {"VAN ", "VON ", EndOfList};
const std::string SCH_LIST[2] = {"SCH", EndOfList};
const std::string ORCHES_ARCHIT_ORCHID_LIST[4] = {"ARCHIT", "ORCHID","ORCHES",  EndOfList};
const char T_S_LIST[3] = {'S', 'T', MaxChar};
const char A_O_U_E_LIST[5] = {'A', 'E', 'O', 'U', MaxChar};
const char L_R_N_M_B_H_F_V_W_SPACE_LIST[11] = {' ', 'B', 'F', 'H', 'L', 'M', 'N', 'R', 'V', 'W', MaxChar};
const char MC_LIST[2] = {'C', MaxChar};
const char CZ_LIST[2] = {'Z', MaxChar};
const std::string WICZ_LIST[2] = {"ICZ", EndOfList};
const std::string CIA_LIST[2] = {"IA", EndOfList};
const char CC_LIST[2] = {'C', MaxChar};
const char I_E_H_LIST[4] = {'E', 'H', 'I', MaxChar};
const char HU_LIST[2] = {'U', MaxChar};
const std::string UCCEE_UCCES_LIST[3] = {"UCCEE", "UCCES", EndOfList};
const char CK_CG_CQ_LIST[4] = {'G', 'K', 'Q', MaxChar};
const char CI_CE_CY_LIST[4] = {'E', 'I', 'Y', MaxChar};
const std::string CIO_CIE_CIA_LIST[4] = {"IA", "IE", "IO", EndOfList};
const char C_Q_G_LIST[4] = {'C', 'G', 'Q', MaxChar};
const char C_K_Q_LIST[4] = {'C', 'K', 'Q', MaxChar};
const char CE_CI_LIST[3] = {'E', 'I', MaxChar};
const char DG_LIST[2] = {'G', MaxChar};
const char I_E_Y_LIST[4] = {'E', 'I', 'Y', MaxChar};
const char DT_DD_LIST[3] = {'D', 'D', MaxChar};
const char B_H_D_LIST[4] = {'B', 'D', 'H', MaxChar};
const char B_H_LIST[3] = {'B', 'H', MaxChar};
const char C_G_L_R_T_LIST[6] = {'C', 'G', 'L', 'R', 'T', MaxChar};
const char EY_LIST[2] = {'Y', MaxChar};
const char LI_LIST[2] = {'I', MaxChar};
const char ES_EP_EB_EL_EY_IB_IL_IN_IE_EI_ER_LIST[13] = {'B', 'E', 'E', 'E', 'I', 'I', 'I', 'L', 'N', 'P', 'R', 'S', MaxChar};
const char ER_LIST[2] = {'R', MaxChar};
const std::string RGY_OGY_LIST[3] = {"GY", "GY", EndOfList};
const std::string DANGER_RANGER_MANGER_LIST[4] = {"ANGER", "ANGER", "ANGER", EndOfList};
const char EI_E_LIST[3] = {'E', 'I', MaxChar};
const char VOWELS[4] = {'E', 'I', 'Y', MaxChar};
const std::string AGGI_OGGI[3] = {"GGI", "GGI", EndOfList};
//const std::string VAN_VON[3] = {"AN", "ON", EndOfList};
const char ET[2] = {'T', MaxChar};
const std::string FRENCH_ENDINGS[2] = {"ER", EndOfList};
const std::string SAN_LIST[2] = {"AN ", EndOfList};
const std::string JOSE_LIST[2] = {"OSE", EndOfList};
const char LTKSNMBZ_LIST[9] = {'B', 'K', 'L', 'M', 'N', 'S', 'T', 'Z', MaxChar};
const char SKL_LIST[4] = {'K', 'L', 'S', MaxChar};
const std::string ILLO_ILLA_ALLE_LIST[4] = {"LLE", "LLA", "LLO", EndOfList};
const char AS_OS_LIST[3] = {'S', 'S', MaxChar};
const char A_O_LIST[3] = {'A', 'O', MaxChar};
const std::string ALLE_LIST[2] = {"LLE", EndOfList};
const std::string UMB_LIST[2] = {"MB", EndOfList};
const char P_B_LIST[3] = {'B', 'P', MaxChar};
const char IE_LIST[2] = {'E', MaxChar};
const char ME_LIST[3] = {'A', 'E', MaxChar};
const std::string ISL_YSL_LIST[3] = {"SL", "SL", EndOfList};
const std::string SUGAR_LIST[2] = {"UGAR", EndOfList};
const char SH_LIST[2] = {'H', MaxChar};
const std::string HEIM_HOEK_HOLM_HOLZ_LIST[5] = {"EIM", "OEK", "OLM", "OLZ", EndOfList};
const std::string SIO_SIA_LIST[3] = {"IA", "IO", EndOfList};
const std::string SIAN_LIST[2] = {"IAN", EndOfList};
const char SC_H_LIST[3] = {'H', 'C', MaxChar};
const char SC_I_E_Y_LIST[5] = {'E', 'I', 'C', 'Y', MaxChar};
const char SC_LIST[2] = {'C', MaxChar};
const char TH_LIST[2] = {'H', MaxChar};
const std::string TTH_LIST[2] = {"TH", EndOfList};
const char T_D_LIST[3] = {'D', 'T', MaxChar};
const char WR_LIST[2] = {'R', MaxChar};
const std::string WICZ_WITZ_LIST[3] = {"ICZ", "ITZ", EndOfList};
const char C_X_LIST[3] = {'C', 'X', MaxChar};
static const char* ZO_ZI_ZA_LIST[] = {"ZA", "ZI", "ZO", nullptr};
const char M_N_L_W_LIST[5] = {'L', 'M', 'N', 'W', MaxChar};
const char Z_LIST[2] = {'Z', MaxChar};
const char OO_ER_EN_UY_ED_EM[10] = {'D', 'E', 'E', 'M', 'N', 'O', 'R', 'U', 'Y', MaxChar};
const char ER_EN[4] = {'E', 'N', 'R', MaxChar};
const char AI_OI[3] = {'I', 'I', MaxChar};
const std::string TIA_TCH[3] = {"CH", "IA", EndOfList};
const char OM_AM[3] = {'M', 'M', MaxChar};
const char WH[2] = {'H', MaxChar};
const char S_Z[3] = {'S', 'Z', MaxChar};
const std::string TION[2] = {"ION", EndOfList};
const std::string EWSKI[5] = {"WSKI", "WSKY", "WSKI", "WSKY", EndOfList};
const std::string IAU_EAU[3] = {"IAU", "AU", EndOfList};
const char AU_OU[3] = {'A', 'O', MaxChar};
const char C_X[3] = {'C', 'X', MaxChar};
//const char ZO_ZI_ZA[4] = {'A', 'I', 'O', MaxChar};
const std::string GN_KN_PN_WR_PS[6] = {"GN", "KN", "PN", "WR", "PS", EndOfList};

const bool* vowel = InitializeVowelArray();

void MakeUpper(std::string &s) {
    for (unsigned int i = 0; i < s.length(); i++) {
        s[i] = toupper(s[i]);
    }
}

bool SlavoGermanic(const std::string& original, int last, unsigned int pad) {
    if (SlavoDone) return SlavoValue;
    SlavoDone = true;

    for (int i = pad; i < last; i++) {
        if (original[i] == 'W' || original[i] == 'K') {
            return true;
        }
        if (original[i] == 'C' && original[i + 1] == 'Z') {
            return true;
        }
        if (original[i + 1] == 'I' && original[i + 2] == 'T' && original[i + 3] == 'Z') {
            return true;
        }
    }
    SlavoValue = false;
    return false;
}

// Templated search method with size parameter
template<class T, size_t N>
bool search(const T (&list)[N], const T& x) {
    int i = 0;
    while (list[i] < x) i++;
    return (i < (N - 1)) && (list[i] == x);
}

void HandleVowel(std::string& primary, const std::string& original, int& current, int /*last*/) { 
    primary += original[current];
//    secondary += original[current];
    current += 1;
}

void HandleB(std::string& primary, const std::string& original, int& current, int /*last*/) {   
    primary += "P";
  //  secondary += "P";
    if (original[current + 1] == 'B')
        current += 2;
    else
        current += 1;
}

void HandleC(std::string& primary, const std::string& original, int& current, int last) {       
    if (current >= pad + 2 && !vowel[static_cast<unsigned char>(original[current - 2])] && 
        original.substr(current - 1, 3) == "ACH" &&
        ((current + 2 <= last && original[current + 2] != 'I') &&
        ((current + 2 <= last && original[current + 2] != 'E') ||
         search(BACHER_MACHER_LIST, original.substr(current+1, 5))))) {
        primary += "K";
    //    secondary += "K";
        current += 2;
        return;
    }
    if ((current == pad) && original.substr(current + 1, 5) == "AESAR") {
        primary += "S";
      //  secondary += "S";
        current += 2;
        return;
    }
    if (original.substr(current + 1, 3) == "HIA") {
        primary += "K";
        //secondary += "K";
        current += 2;
        return;
    }
    if (original[current + 1] == 'H') {
        if ((current > pad) && original.substr(current + 2, 2) == "AE") {
            primary += "K";
          //  secondary += "X";
            current += 2;
            return;
        }
        if ((current == pad) &&
            (search(HARAC_HARIS_LIST, original.substr(current + 2, 4)) ||
             search(HOR_HYM_HIA_HEM_LIST, original.substr(current + 2, 2))) &&
             original.substr(pad + 2, 3) != "ORE") {
            primary += "K";
            //secondary += "K";
            current += 2;
            return; 
        }
        if (((current > pad) && search(VAN_VON_LIST, original.substr(pad + 1, 3))) ||
            (current > pad && original.substr(pad, 3) == "SCH") ||
            (search(ORCHES_ARCHIT_ORCHID_LIST, original.substr(current - 2, 6))) ||
            search(T_S_LIST, original[current + 2]) ||
            ((search(A_O_U_E_LIST, original[current - 1])) &&
             search(L_R_N_M_B_H_F_V_W_SPACE_LIST, original[current + 3]))) {
            primary += "K";
            //secondary += "K";
        } else {
            if (current > pad) {
                if (search(MC_LIST, original[pad + 1])) {
                    primary += "K";
              //      secondary += "K";
                } else {
                    primary += "X";
                //    secondary += "K";
                }
            } else {
                primary += "X";
                //secondary += "X";
            }
        }
        current += 2;
        return; 
    }
    if (original.substr(current, 2) == "CZ" && (current < pad + 2 || original.substr(current - 2, 4) != "WICZ")) {
        primary += "S";
        //secondary += "X";
        current += 2;
        return; 
    }
    if (current + 3 <= last && original[current + 1] == 'C' && 
        original[current + 2] == 'I' && original[current + 3] == 'A') {
        primary += "X";
        current += 3;
        return;
    }
    if (search(CC_LIST, original[current + 2]) &&
        !((current == pad + 1) && (original[pad] == 'M'))) {
        if (search(I_E_H_LIST, original[current + 4]) &&
           !search(HU_LIST, original[current + 4])) {
            if (((current == pad + 1) && (original[current - 1] == 'A')) ||
                search(UCCEE_UCCES_LIST, original.substr(current - 1, 5))) {  // Fixed line
                primary += "KS";
          //      secondary += "KS";
            } else {
                primary += "X";
            //    secondary += "X";
            }
            current += 3;
            return; 
        } else {
            primary += "K";
            //secondary += "K";
            current += 2;
            return; 
        }
    }
    if (search(CK_CG_CQ_LIST, original[current + 2])) {
        primary += "K";
        //secondary += "K";
        current += 2;
        return; 
    }
    if (search(CI_CE_CY_LIST, original[current + 2])) {
        if (search(CIO_CIE_CIA_LIST, original.substr(current + 2, 2))) {
            primary += "S";
          //  secondary += "X";
        } else {
            primary += "S";
            //secondary += "S";
        }
        current += 2;
        return; 
    }
    primary += "K";
    //secondary += "K";
    if (search(C_Q_G_LIST, original[current + 3]))
        current += 3;
    else if (search(C_K_Q_LIST, original[current + 3]) &&
             !(original[current + 1] == 'C' && (original[current + 2] == 'E' || original[current + 2] == 'I')))
        current += 2;
    else
        current += 1;
}

void HandleD(std::string& primary, const std::string& original, int& current, int last) {       
    if (current + 2 <= last && original[current + 1] == 'G' && 
        (original[current + 2] == 'E' || original[current + 2] == 'I' || original[current + 2] == 'Y')) {
        primary += "J";
      //  secondary += "J";
        current += 2;
        return;
    }
    primary += "T";
    //secondary += "T";
    if (original[current + 1] == 'D')
        current += 2;
    else
        current += 1;
}

void HandleF(std::string& primary, const std::string& original, int& current, int /*last*/) {       
    primary += "F";
    //secondary += "F";
    if (original[current + 1] == 'F')
        current += 2;
    else
        current += 1;
}

void HandleG(std::string& primary, const std::string& original, int& current, int last) {       
    if (original[current + 1] == 'H') {
        if ((current > pad + 1) && !vowel[static_cast<unsigned char>(original[current - 1])]) {
            primary += "K";
      //      secondary += "K";
            current += 2;
            return; 
        }
        if (current < pad + 3) {
            if (current == pad) {
                if (original[current + 2] == 'I') {
                    primary += "J";
        //            secondary += "J";
                } else {
                    primary += "K";
          //          secondary += "K";
                }
                current += 2;
                return; 
            }
        }
        if (((current > pad + 1) && search(B_H_D_LIST, original[current - 1])) ||
            ((current > pad + 2) && search(B_H_D_LIST, original[current - 2])) ||
            ((current > pad + 3) && search(B_H_LIST, original[current - 3]))) {
            current += 2;
            return; 
        } else {
            if ((current > pad + 2) && (original[current - 1] == 'U') &&
                search(C_G_L_R_T_LIST, original[current - 2])) {
                primary += "F";
            //    secondary += "F";
            } else if ((current > pad) && original[current - 1] != 'I') {
                primary += "K";
              //  secondary += "K";
            }
            current += 2;
            return; 
        }
    }
    if (original[current + 1] == 'N') {
        if ((current == pad + 1) && vowel[static_cast<unsigned char>(original[pad])] && !SlavoGermanic(original, last, pad)) {
            primary += "KN";
            //secondary += "N";
        } else if (original.substr(current + 3, 2) != "EY" && (original[current + 1] != 'Y') && !SlavoGermanic(original, last, pad)) {
            primary += "N";
            //secondary += "KN";
        } else {
            primary += "KN";
            //secondary += "KN";
        }
        current += 2;
        return; 
    }
    if (original.substr(current + 2, 2) == "LI" && !SlavoGermanic(original, last, pad)) {
        primary += "KL";
        //secondary += "L";
        current += 2;
        return; 
    }
    if ((current == pad) &&
        ((original[current + 1] == 'Y') ||
         search(ES_EP_EB_EL_EY_IB_IL_IN_IE_EI_ER_LIST, original[current + 2]))) {
        primary += "K";
        //secondary += "J";
        current += 2;
        return; 
    }
    if ((original.substr(current + 2, 2) == "ER" || original[current + 1] == 'Y') &&
        !search(DANGER_RANGER_MANGER_LIST, original.substr(pad + 1, 5)) &&
        (current > pad + 1) && !search(EI_E_LIST, original[current + 1]) &&
        (current > pad + 1) && !search(RGY_OGY_LIST, original.substr(current + 1, 2))) {
        primary += "K";
        //secondary += "J";
        current += 2;
        return; 
    }
    if (search(VOWELS, original[current + 3]) ||
        (current > pad + 1) && search(AGGI_OGGI, original.substr(current + 1, 3))) {
        if ((search(VAN_VON_LIST, original.substr(pad + 1, 3)) || original.substr(pad,3) == "SCH") ||
            original.substr(current + 2, 2) == "ET") {
           primary += "K";
          // secondary += "K";
       } else {
            if (original.substr(current + 2, 4) == "IER ") {
                primary += "J";
            //    secondary += "J";
            } else {
                primary += "J";
              //  secondary += "K";
            }
        }
        current += 2;
        return; 
    }
    if (original[current + 1] == 'G')
        current += 2;
    else
        current += 1;
    primary += "K";
    //secondary += "K";
}

void HandleH(std::string& primary, const std::string& original, int& current, int /*last*/) {       
    if (((current == pad) || vowel[static_cast<unsigned char>(original[current - 1])]) &&
        vowel[static_cast<unsigned char>(original[current + 1])]) {
        primary += "H";
      //  secondary += "H";
        current += 2;
    } else
        current += 1;
}

void HandleJ(std::string& primary, const std::string& original, int& current, int last) {     
    if (original.substr(current + 1, 3) == "OSE" || original.substr(0, 4) == "SAN ") {
        if (((current == pad) && (original[current + 4] == ' ')) || search(SAN_LIST, original.substr(pad + 1, 3))) {
            primary += "H";
        //    secondary += "H";
        } else {
            primary += "J";
        //    secondary += "H";
        }
        current += 1;
        return; 
    }
    if ((current == pad) && original.substr(current + 1, 3) != "OSE") {
        primary += "J";
        //secondary += "A";
    } else {
        if ((current > pad + 1) && vowel[static_cast<unsigned char>(original[current - 1])] && !SlavoGermanic(original, last, pad) &&
            ((original[current + 1] == 'A') || (original[current + 1] == 'O'))) {
            primary += "J";
          //  secondary += "H";
        } else {
            if (current == last) {
                primary += "J";
            //    secondary += "";
            } else {
                if (!search(LTKSNMBZ_LIST, original[current + 3]) &&
                    (current > pad + 1) && !search(SKL_LIST, original[current + 1])) {
                    primary += "J";
              //      secondary += "J";
                }
            }
        }
    }
    if (original[current + 1] == 'J')
        current += 2;
    else
        current += 1;
}

void HandleK(std::string& primary, const std::string& original, int& current, int /*last*/) {
    if (current == pad && original[current + 1] == 'N') {
        primary += "N";
        //secondary += "N";
    } else {
        primary += "K";
        //secondary += "K";
    }
    if (original[current + 1] == 'K')
        current += 2;
    else
        current += 1;
}

void HandleL(std::string& primary, const std::string& original, int& current, int last) {       
    primary += "L";
    if (original[current + 1] == 'L') {
        if (((current == last - 2) &&
             search(ILLO_ILLA_ALLE_LIST, original.substr(current - 1, 4))) ||
            ((search(AS_OS_LIST, original[last - 1]) || search(A_O_LIST, original[last])) &&
             original.substr(current - 1, 4) == "ALLE")) {
          //  secondary += "";
        } else {
            //secondary += "L";
        }
        current += 2;
    } else {
        //secondary += "L";
        current += 1;
    }
}

void HandleM(std::string& primary, const std::string& original, int& current, int last) {
    primary += "M";
    //secondary += "M";
    if (current > pad && original[current + 1] == 'B' && (current + 1 == last || original[current + 2] == ' '))
        current += 2; // "MB" at end or before space
    else if (original[current + 1] == 'M')
        current += 2;
    else
        current += 1;
}

void HandleN(std::string& primary, const std::string& original, int& current, int /*last*/) {
    primary += "N";
    //secondary += "N";
    if (original[current + 1] == 'N')
        current += 2;
    else
        current += 1;
}

void HandleP(std::string& primary, const std::string& original, int& current, int /*last*/) {
    if (original[current + 1] == 'H') {
        primary += "F";
      //  secondary += "F";
        current += 2;
        return;
    }
    primary += "P";
    //secondary += "P";
    if (search(P_B_LIST, original[current + 3]))
        current += 2;
    else
        current += 1;
}

void HandleQ(std::string& primary, const std::string& original, int& current, int /*last*/) {
    primary += "K";
    //secondary += "K";
    if (original[current + 1] == 'Q')
        current += 2;
    else
        current += 1;
}

void HandleR(std::string& primary, const std::string& original, int& current, int last) {   
    if ((current == last) && !SlavoGermanic(original, last, pad) &&
        (current >= pad + 2) && search(IE_LIST, original[current - 1]) &&
        (current >= pad + 4) && !search(ME_LIST, original[current - 3])) {
        primary += "";
      //  secondary += "R";
    } else {
        primary += "R";
        //secondary += "R";
    }
    if (original[current + 1] == 'R')
        current += 2;
    else
        current += 1;
}

void HandleS(std::string& primary, const std::string& original, int& current, int last) {       
    if (search(ISL_YSL_LIST, original.substr(current + 1, 2))) { // std::string[]
        current += 1;
        return; 
    }
    if ((current == pad) && original.substr(current + 1, 4) == "UGAR") {
        primary += "X";
        //secondary += "S";
        current += 1;
        return; 
    }
    if (original[current + 1] == 'C' && original[current + 2] == 'H') { // "SCH"
        if (current == pad && !vowel[static_cast<unsigned char>(original[pad + 3])] && original[pad + 3] != 'W') {
            primary += "X";
          //  secondary += "S"; // Initial "SCHOOL"
        } else if (search(OO_ER_EN_UY_ED_EM, original[current + 4])) { // char[]
            if (search(ER_EN, original[current + 4])) { // char[]
                primary += "X";
            //    secondary += "SK"; // "SCHERMERHORN"
            } else {
                primary += "SK";
              //  secondary += "SK"; // "SCHOOL"
            }
        } else {
            primary += "X";
            //secondary += "X"; // "SCHULTZ"
        }
        current += 3;
        return; 
    }
    if (original[current + 1] == 'H') {
        if (search(HEIM_HOEK_HOLM_HOLZ_LIST, original.substr(current + 2, 3))) { // std::string[]
            primary += "S";
            //secondary += "S";
        } else {
            primary += "X";
            //secondary += "X";
        }
        current += 2;
        return; 
    }
    if (search(SIO_SIA_LIST, original.substr(current + 2, 2)) || original.substr(current + 2, 3) == "IAN") { // std::string[]
        if (!SlavoGermanic(original, last, pad)) {
            primary += "S";
            //secondary += "X";
        } else {
            primary += "S";
            //secondary += "S";
        }
        current += 3;
        return; 
    }
    if ((current == pad && search(M_N_L_W_LIST, original[current + 3])) || // char[]
        search(Z_LIST, original[current + 3])) { // char[]
        primary += "S";
        //secondary += "X";
        if (search(Z_LIST, original[current + 3])) // char[]
            current += 2;
        else
            current += 1;
        return; 
    }
    if (original[current + 1] == 'C') {
        if (search(I_E_Y_LIST, original[current + 4])) { // char[]
            primary += "S";
          //  secondary += "S";
            current += 3;
            return; 
        }
        primary += "SK";
        //secondary += "SK";
        current += 3;
        return; 
    }
    if (current == last && search(AI_OI, original[current - 1])) { // char[], check first char
        //secondary += "S";
    } else {
        primary += "S";
        //secondary += "S";
    }
    if (search(S_Z, original[current + 3])) // char[]
        current += 2;
    else
        current += 1;
}

void HandleT(std::string& primary, const std::string& original, int& current, int last) {       
    if (original.substr(current + 2, 2) == "ON") { // "TION" approximation
        primary += "X";
        //secondary += "X";
        current += 3;
        return; 
    }
    if (search(TIA_TCH, original.substr(current + 2, 2))) { // std::string[]
        primary += "X";
        //secondary += "X";
        current += 3;
        return; 
    }
    if (original[current + 1] == 'H' || original.substr(current + 2, 2) == "TH") {
        if (search(OM_AM, original[current + 4]) || // char[]
            search(VAN_VON_LIST, original.substr(pad + 1, 3)) || // std::string[]
            original.substr(pad, 3) == "SCH") {
            primary += "T";
          //  secondary += "T"; // "THOMAS", "VAN THIEL"
        } else {
            primary += "0";
            //secondary += "T"; // "THIN"
        }
        current += 2;
        return; 
    }
    primary += "T";
    //secondary += "T";
    if (search(T_D_LIST, original[current + 3])) // char[]
        current += 2;
    else
        current += 1;
}

void HandleV(std::string& primary, const std::string& original, int& current, int /*last*/) {  
    primary += "F";
    //secondary += "F";
    if (original[current + 1] == 'V')
        current += 2;
    else
        current += 1;
}

void HandleW(std::string& primary, const std::string& original, int& current, int last) {      
    if (original.substr(current, 2) == "WR") {
        primary += "R";
      //  secondary += "R";
        current += 2;
        return;
    }
    if ((current == pad) &&
        (vowel[static_cast<unsigned char>(original[current + 1])] || original.substr(current, 2) == "WH")) {
        if (vowel[static_cast<unsigned char>(original[current + 1])]) {
            primary += "A";
        //    secondary += "F";
        } else {
            primary += "A";
          //  secondary += "A";
        }
    }
    if (((current > pad + 1) && (current == last) && vowel[static_cast<unsigned char>(original[current - 1])]) ||
        search(EWSKI, original.substr(current + 1, 4)) ||
        search(SCH_LIST, original.substr(pad + 1, 2))) {
        primary += "";
        //secondary += "F";
        current += 1;
        return; 
    }
    if (search(WICZ_WITZ_LIST, original.substr(current + 2, 3))) {
        primary += "TS";
        //secondary += "FX";
        current += 4;
        return; 
    }
    current += 1;
}

void HandleX(std::string& primary, const std::string& original, int& current, int last) { 
    if (!((current == last) &&
          (current >= pad + 3 && search(IAU_EAU, original.substr(current - 2, 2)) ||
           current >= pad + 2 && search(AU_OU, original[current - 1])))) {
        primary += "KS";
        //secondary += "KS";
    }
    if (search(C_X_LIST, original[current + 3]))
        current += 2;
    else
        current += 1;
}

void HandleZ(std::string& primary, const std::string& original, int& current, int last) {       
    if (original[current + 1] == 'H') {
        primary += "J";
        //secondary += "J";
        current += 2;
        return; 
    }
    if (search(ZO_ZI_ZA_LIST, original.substr(current + 1, 2).c_str()) ||
        (SlavoGermanic(original, last - pad, pad) && (current > pad + 1) && original[current - 1] != 'T')) {
        primary += "S";
        //secondary += "TS";
    } else {
        primary += "S";
        //secondary += "S";
    }
    if (original[current + 1] == 'Z')
        current += 2;
    else
        current += 1;
}

void HandleDefault(std::string& primary, const std::string& original, int& current, int /*last*/) {
    current += 1;
}

// Function pointer lookup table (indexed by character A-Z)
std::array<void(*)(std::string&, const std::string&, int&, int), 26> charHandlers = {
    HandleVowel, HandleB, HandleC, HandleD, HandleVowel,  // A, B, C, D, E
    HandleF, HandleG, HandleH, HandleVowel, HandleJ,      // F, G, H, I, J
    HandleK, HandleL, HandleM, HandleN, HandleVowel,      // K, L, M, N, O
    HandleP, HandleQ, HandleR, HandleS, HandleT,          // P, Q, R, S, T
    HandleVowel, HandleV, HandleW, HandleX, HandleVowel,  // U, V, W, X, Y
    HandleZ  // Z
};

// Declare NewcharHandlers globally
std::array<void(*)(std::string&, const std::string&, int&, int), 256> NewcharHandlers;

void InitializeNewCharHandlers() {
    for (int i = 0; i < 256; i++) {
        NewcharHandlers[i] = HandleDefault;
    }
    for (int i = 0; i < 26; i++) {
        NewcharHandlers['A' + i] = charHandlers[i];
    }
}

void DoubleMetaphone(const std::string &str, std::string *code) {
    static bool initialized = false;
    if (!initialized) {
        InitializeNewCharHandlers();
        initialized = true;
    }

    // Reset SlavoGermanic flag
    SlavoDone = false;

    // Local variables
    int current = pad;
    int length = str.length();
    int last = pad + length;
    std::string original = std::string(pad, ' ') + str + "     ";
    std::string primary = "";

    MakeUpper(original);

    if (current == pad) {
        if (original[pad] == 'X') {
            primary += "S";
          //  secondary += "S";
            current++;
        } 
        else if (search(GN_KN_PN_WR_PS, original.substr(pad, 2))) {
            current++;
        }
    }
    
    // Use a smaller target length for early termination
    const unsigned int target_length = 4;

    while (primary.length() < max_length && current < last) {
        NewcharHandlers[original[current]](primary, original, current, last);
        if (current >= last) break; // Prevent overrun
    }

   // if (primary.length() > max_length)
     //   primary[max_length] = '\0';
    //if (secondary.length() > max_length)
      //  secondary[max_length] = '\0';

      *code = primary;
}