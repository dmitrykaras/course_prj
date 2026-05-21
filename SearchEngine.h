#pragma once
#include "Utils.h"
#include <fstream>
#include <algorithm>
#include <set>
#include <chrono>
#include <map> // Для сравнения производительности

using namespace std;

class SearchEngine {
private:
    vector<string> dictionary;
    unordered_map<string, vector<int>> index; // Твой основной индекс
    map<string, vector<int>> treeIndex;      // Индекс для сравнения (дерево)
    int nGramSize;

public:
    SearchEngine(int n);
    void addWord(const string& word);
    void search(const string& query);
    void loadFromFile(string filename);
    void printCollisionStats(); // Аудит коллизий
};