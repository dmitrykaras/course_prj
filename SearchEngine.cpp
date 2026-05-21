#include "SearchEngine.h"

SearchEngine::SearchEngine(int n) : nGramSize(n) {}

void SearchEngine::addWord(const string& word) {
    string cleanWord = Utils::toLower(word);

    int wordId = (int)dictionary.size();
    dictionary.push_back(cleanWord);

    auto ngrams = Utils::generateNgrams(cleanWord, nGramSize);
    set<string> uniqueGrams(ngrams.begin(), ngrams.end());
    for (const auto& gram : uniqueGrams) {
        index[gram].push_back(wordId);
    }
}

void SearchEngine::loadFromFile(string filename) {
    auto start = chrono::high_resolution_clock::now();

    ifstream file(filename);
    if (!file.is_open()) {
        cout << "Не удалось открыть файл: " << filename << endl;
        return;
    }

    // Резервируем место под 50к+ слов, чтобы вектор не "пересобирался"
    

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

    auto end = chrono::high_resolution_clock::now();
    chrono::duration<double> elapsed = end - start;

    cout << "Готово!" << endl;
    cout << "-----------------------------------" << endl;
    cout << "Загружено слов: " << count << endl;
    cout << "Время индексации: " << elapsed.count() << " сек." << endl;
    cout << "-----------------------------------" << endl;

    // Автоматический вывод аудита хеш-таблицы сразу после индексации
    printCollisionStats();
}

void SearchEngine::printCollisionStats() {
    size_t totalBuckets = index.bucket_count(); // Общее количество ячеек (корзин) в RAM
    size_t busyBuckets = 0;                     // Сколько ячеек занято биграммами
    size_t collisionBuckets = 0;                // Сколько ячеек содержат коллизии (> 1 биграммы)
    size_t maxBucketSize = 0;                   // Максимальная глубина коллизии

    for (size_t i = 0; i < totalBuckets; ++i) {
        size_t bSize = index.bucket_size(i);
        if (bSize > 0) {
            busyBuckets++;
            if (bSize > 1) {
                collisionBuckets++;
            }
            if (bSize > maxBucketSize) {
                maxBucketSize = bSize;
            }
        }
    }

    // Процент коллизий считаем как отношение конфликтных ячеек к общему числу занятых ячеек
    double collisionPercent = (busyBuckets > 0) ? ((double)collisionBuckets / busyBuckets * 100.0) : 0.0;

    cout << "\n----- Анализ хеш-таблицы индекса -----" << endl;
    cout << "Выделено ячеек: " << totalBuckets << endl;
    cout << "Занято уникальными биграммами: " << busyBuckets << endl;
    cout << "Ячеек с коллизиями: " << collisionBuckets << endl;
    cout << "Процент коллизий среди занятых ячеек: " << collisionPercent << "%" << endl;
    cout << "Макс. коллизий в одной цепочке: " << maxBucketSize << " эл." << endl;
    cout << "Коэффициент загрузки (Load Factor): " << index.load_factor() << endl;
    cout << "Макс. предел загрузки (Max Load Factor): " << index.max_load_factor() << endl;
    cout << "---------------------------------------\n" << endl;
}

void SearchEngine::search(const string& query) {
    string cleanQuery = Utils::toLower(query);
    auto queryNgramsRaw = Utils::generateNgrams(cleanQuery, nGramSize);

    cout << "\nРазбиение запроса на биграммы: ";
    for (const auto& gram : queryNgramsRaw) {
        cout << "[" << gram << "] ";
    }
    cout << endl;

    set<string> queryNgrams(queryNgramsRaw.begin(), queryNgramsRaw.end());
    if (queryNgrams.empty()) return;

    // =================================================================
    // 1. НАИВНЫЙ ЛИНЕЙНЫЙ ПОИСК (Полный перебор словаря за O(N))
    // =================================================================
    cout << "\n 1. Наивный линейный поиск..." << endl;
    auto startLinear = chrono::high_resolution_clock::now();

    vector<pair<int, double>> linearResults;

    for (size_t wordId = 0; wordId < dictionary.size(); ++wordId) {
        const string& word = dictionary[wordId];

        auto wordNgramsRaw = Utils::generateNgrams(word, nGramSize);
        set<string> wordNgrams(wordNgramsRaw.begin(), wordNgramsRaw.end());

        int intersection = 0;
        for (const auto& gram : queryNgrams) {
            if (wordNgrams.count(gram)) {
                intersection++;
            }
        }

        if (intersection > 0) {
            size_t wordLen = word.length();
            size_t wordNgramsCount = (wordLen >= (size_t)nGramSize) ? (wordLen - nGramSize + 1) : 1;

            double similarity = (double)intersection / (queryNgrams.size() + wordNgramsCount - intersection);
            linearResults.push_back({ (int)wordId, similarity });
        }
    }

    sort(linearResults.begin(), linearResults.end(), [](const auto& a, const auto& b) {
        return a.second > b.second;
        });

    auto endLinear = chrono::high_resolution_clock::now();
    chrono::duration<double, milli> elapsedLinear = endLinear - startLinear;

    for (size_t i = 0; i < min(linearResults.size(), (size_t)5); ++i) {
        cout << " - " << dictionary[linearResults[i].first] << " [" << (int)(linearResults[i].second * 100) << "%]" << endl;
    }
    cout << "Время линейного поиска: " << elapsedLinear.count() << " мс" << endl;


    // =================================================================
    // 2. ОПТИМИЗИРОВАННЫЙ ПОИСК (Через инвертированный индекс за O(1))
    // =================================================================
    cout << "\n 2. Оптимизированный поиск через индекс..." << endl;
    auto startOpt = chrono::high_resolution_clock::now();

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
        size_t wordLen = dictionary[wordId].length();
        size_t wordNgramsCount = (wordLen >= (size_t)nGramSize) ? (wordLen - nGramSize + 1) : 1;

        double similarity = (double)count / (queryNgrams.size() + wordNgramsCount - count);
        rankedResults.push_back({ wordId, similarity });
    }

    sort(rankedResults.begin(), rankedResults.end(), [](const auto& a, const auto& b) {
        return a.second > b.second;
        });

    auto endOpt = chrono::high_resolution_clock::now();
    chrono::duration<double, milli> elapsedOpt = endOpt - startOpt;

    for (size_t i = 0; i < min(rankedResults.size(), (size_t)5); ++i) {
        cout << " - " << dictionary[rankedResults[i].first] << " [" << (int)(rankedResults[i].second * 100) << "%]" << endl;
    }
    cout << "Время поиска через Индекс: " << elapsedOpt.count() << " мс" << endl;

    if (elapsedOpt.count() > 0) {
        double speedup = elapsedLinear.count() / elapsedOpt.count();
        cout << "Инвертированный индекс быстрее в " << speedup << " раз!" << endl;
    }
    else {
        cout << "РЕЗУЛЬТАТ: Индекс отработал практически мгновенно (~0 мс)!" << endl;
    }
}