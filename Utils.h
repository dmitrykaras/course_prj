#pragma once

// Вспомогательные функции(обработка строк, UTF - 8)

#include <string>
#include <vector>
#include <algorithm>
#include <iostream>
#include <unordered_map>
#include <fstream>

#include <Windows.h>

using namespace std;

class Utils {
public:
    // Разбивает строку на N-граммы заданного размера
    static vector<string> generateNgrams(const string& text, size_t n);

    // Приведение к нижнему регистру (с учетом кириллицы)
    static string toLower(string text);
};