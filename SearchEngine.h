#pragma once
// Логика ранжирования и поиска
#include "Utils.h"

#include <fstream>
#include <algorithm>
#include <set>
#include <chrono>

using namespace std;

class SearchEngine {
private:
    vector<string> dictionary;
    unordered_map<string, vector<int>> index;
    int nGramSize;

public:
    SearchEngine(int n);

    void addWord(const string& word);
    void search(const string& query);
    void loadFromFile(string filename);
};