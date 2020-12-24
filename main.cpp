#include <iostream>
#include <fstream>
#include <ctime>
#include <windows.h>
#include <vector>
#include <map>
#include <algorithm>
#include <chrono> //для бенчмарков

//конвертирование словаря которого я нашел по популярности
//обязательный критерий работы данного дерева - слова, упорядоченные
//по популярности/частотности, иначе подсказки могут быть кривые
//например, вводишь "прив", а он предлагает "приведение" или "привозной"
std::vector<std::string> convertDicts()
{
    std::vector<std::string> ans;
    std::string filelist[] = {"dict_sp.txt", "dict_lit.txt", "dict_pub.txt"};
    for (std::string filename: filelist)
    {
        std::map<std::string, double> newDict;
        std::ifstream fin(filename);
        if (!fin.is_open())
        {
            std::cerr << "Failed to open " << filename << std::endl;
            continue;
        }
        while (true)
        {
            int id;
            std::string word1, word2, partOfSpeech;
            double freq;
            fin >> id >> word1;
            if (filename == "dict_sp.txt")
                fin >> word2;
            fin >> partOfSpeech >> freq;
            if (fin.eof())
                break;

            newDict.insert({word1, freq});
        }
        std::vector<std::pair<std::string, double>> sortedDict;
        for (auto i = newDict.begin(); i != newDict.end(); ++i)
            sortedDict.push_back(std::move(*i));
        std::sort(sortedDict.begin(), sortedDict.end(), [](std::pair<std::string, double> a, std::pair<std::string, double> b){return a.second > b.second;});
        for (auto i: sortedDict)
            ans.push_back(i.first);
        fin.close();
    }
    return ans;
}

class Trie
{
private:

    struct Node
    {
        char value;
        std::vector<Node*> children;

        Node()
        {
            this->value = 0;
        }

        Node(char value)
        {
            this->value = value;
        }

        virtual ~Node()
        {
            for (size_t i = 0; i < children.size(); ++i)
                delete children[i];
        }
    } *root = nullptr;

    void _insert(const std::string &word, Node *node, size_t index = 0)
    {
        if (index == word.length())
        {
            node->children.push_back(nullptr);
            return;
        }

        auto f = std::find_if(node->children.begin(), node->children.end(), [&](Node *a){if (a != nullptr) return a->value == word[index]; return false;});

        if (f == node->children.end())
        {
            node->children.push_back(new Node(word[index]));
            f = node->children.end() - 1;
        }

        _insert(word, *f, index + 1);
    }

    void _recommendWord(const std::string &word, Node *node, std::string &rec, size_t index = 0)
    {
        //выход на случай, если не получилось найти нужную букву
        if (node == nullptr)
        {
            rec = word;
            return;
        }

        if (index >= word.length())
        {
            if (node->children[0] != nullptr)
            {
                rec += node->children[0]->value;
                _recommendWord(word, node->children[0], rec, index + 1);
            }
            return;
        }

        auto f = std::find_if(node->children.begin(), node->children.end(), [&](Node *a){if (a != nullptr) return a->value == word[index]; return false;});

        if (f != node->children.end())
        {
            rec += word[index];
            _recommendWord(word, *f, rec, index + 1);
            return;
        }

        //INSERT CODE HERE
        //обработка случая, когда буква не найдена здесь
        //
        //то есть перебираем еще и буквы, которые
        //теоретически могут быть на ее месте
        //допустим находятся рядом на клавиатуре
        //или же просто грамматическая ошибка

        //не получилось найти нужную букву
        _recommendWord(word, nullptr, rec, index + 1);
    }

public:

    Trie()
    {
        root = new Node;
    }
    virtual ~Trie()
    {
        delete root;
    }

    void insert(const std::string &word)
    {
        this->_insert(word, this->root);
    }

    std::string recommendWord(const std::string &word)
    {
        std::string rec;
        this->_recommendWord(word, this->root, rec);
        return rec;
    }
};

int main()
{
    SetConsoleCP(1251);
    SetConsoleOutputCP(1251);
    setlocale(LC_ALL, "Russian");
    auto starttime = std::chrono::high_resolution_clock::now();
    std::vector<std::string> dictionary = convertDicts();

    Trie trie;

    for (auto i: dictionary)
        trie.insert(std::move(i));
    dictionary.clear();

    std::cout << "Dictionary created in " << std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - starttime).count() / 1000.0 << " seconds" << std::endl;

    while (true)
    {
        std::string word;
        std::cin >> word;
        std::cout << trie.recommendWord(word) << std::endl << std::endl;
    }
    return 0;
}
