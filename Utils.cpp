#include "Utils.h"
#include <algorithm>

vector<string> Utils::generateNgrams(const string& text, size_t n) {
    vector<string> ngrams;
    // Если слово слишком короткое для N-граммы, оно само по себе является "граммой"
    if (text.length() < n) {
        if (!text.empty()) ngrams.push_back(text);
        return ngrams;
    }

    for (size_t i = 0; i <= text.length() - n; ++i) {
        ngrams.push_back(text.substr(i, n));
    }
    return ngrams;
}

string Utils::toLower(string text) {
    for (auto& c : text) {
        unsigned char uc = (unsigned char)c;
        // Если это заглавные русские буквы А-Я (коды 192-223)
        if (uc >= 192 && uc <= 223) {
            c = (char)(uc + 32); // Сдвигаем в диапазон а-я (224-255)
        }
        // Отдельно обрабатываем заглавную Ё (код 168)
        else if (uc == 168) {
            c = (char)184; // Маленькая ё (код 184)
        }
        else {
            c = (char)tolower(uc);
        }
    }
    return text;
}