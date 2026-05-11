#include "SearchEngine.h"

SearchEngine::SearchEngine(int n) : nGramSize(n) {}

// Обязательно указываем SearchEngine:: перед каждым методом
void SearchEngine::addWord(const string& word) {
    string cleanWord = Utils::toLower(word);

    // Вместо медленного find, просто добавляем. 
    // Если в словаре russian_nouns.txt нет дублей, проверка не нужна.
    int wordId = (int)dictionary.size();
    dictionary.push_back(cleanWord);

    auto ngrams = Utils::generateNgrams(cleanWord, nGramSize);
    set<string> uniqueGrams(ngrams.begin(), ngrams.end());
    for (const auto& gram : uniqueGrams) {
        index[gram].push_back(wordId);
    }
}

void SearchEngine::loadFromFile(string filename) {
    // Начало замера
    auto start = chrono::high_resolution_clock::now();

    ifstream file(filename);
    if (!file.is_open()) {
        cout << "[ОШИБКА] Не удалось открыть файл: " << filename << endl;
        return;
    }

    // Небольшая оптимизация: резервируем место под 50к+ слов, чтобы вектор не "пересобирался"
    dictionary.reserve(55000);

    string word;
    int count = 0;
    cout << "Загрузка базы слов... " << flush;

    while (getline(file, word)) {
        if (!word.empty() && word.back() == '\r') word.pop_back();
        if (!word.empty()) {
            this->addWord(word);
            count++;
        }
    }
    file.close();

    // Конец замера
    auto end = chrono::high_resolution_clock::now();
    chrono::duration<double> elapsed = end - start;

    cout << "Готово!" << endl;
    cout << "-----------------------------------" << endl;
    cout << "Загружено слов: " << count << endl;
    cout << "Время индексации: " << elapsed.count() << " сек." << endl;
    cout << "-----------------------------------" << endl;
}

void SearchEngine::search(const string& query) {
    auto start = chrono::high_resolution_clock::now();

    string cleanQuery = Utils::toLower(query);
    auto queryNgramsRaw = Utils::generateNgrams(cleanQuery, nGramSize);
    set<string> queryNgrams(queryNgramsRaw.begin(), queryNgramsRaw.end());

    if (queryNgrams.empty()) return;

    unordered_map<int, int> scores;
    for (const auto& gram : queryNgrams) {
        if (index.count(gram)) {
            for (int wordId : index[gram]) {
                scores[wordId]++;
            }
        }
    }

    vector<pair<int, double>> rankedResults;
    for (auto const& [wordId, count] : scores) {
        // ОПТИМИЗАЦИЯ: Вместо генерации N-грамм считаем их количество по формуле
        // Для слова длиной L количество уникальных N-грамм (обычно) = L - N + 1
        // Это гораздо быстрее, чем создавать объекты string и vector
        size_t wordLen = dictionary[wordId].length();
        size_t wordNgramsCount = (wordLen >= (size_t)nGramSize) ? (wordLen - nGramSize + 1) : 1;

        double similarity = (double)count / (queryNgrams.size() + wordNgramsCount - count);
        rankedResults.push_back({ wordId, similarity });
    }

    sort(rankedResults.begin(), rankedResults.end(), [](const auto& a, const auto& b) {
        return a.second > b.second;
        });

    auto end = chrono::high_resolution_clock::now();
    chrono::duration<double, milli> elapsed = end - start;

    cout << "\nРезультаты (N=" << nGramSize << "):" << endl;
    for (size_t i = 0; i < min(rankedResults.size(), (size_t)5); ++i) {
        cout << " - " << dictionary[rankedResults[i].first] << " [" << (int)(rankedResults[i].second * 100) << "%]" << endl;
    }
    cout << "-----------------------------------" << endl;
    cout << "Время поиска: " << elapsed.count() << " мс" << endl;
}