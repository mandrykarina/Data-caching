#pragma once
#include "Sequence.h"
#include <chrono>
#include <stdexcept>

template <typename T>
class BTree
{
private:
    static constexpr int ORDER = 4;

    struct BNode
    {
        Sequence<T> keys;           // ключи
        Sequence<BNode *> children; // дети
        bool is_leaf;

        BNode(bool leaf = true) : is_leaf(leaf) {}

        ~BNode()
        {
            if (!is_leaf)
            {
                for (size_t i = 0; i < children.get_size(); ++i)
                    delete children[i];
            }
        }
    };

    BNode *root;
    size_t size;

    // Поиск ключа
    BNode *search_node(BNode *node, const T &key, size_t &idx) const
    {
        size_t i = 0;
        while (i < node->keys.get_size() && key > node->keys[i])
            i++;

        if (i < node->keys.get_size() && key == node->keys[i])
        {
            idx = i;
            return node;
        }

        if (node->is_leaf)
            return nullptr;

        return search_node(node->children[i], key, idx);
    }

    // Разбиение ребенка
    void split_child(BNode *parent, int i)
    {
        BNode *full = parent->children[i];
        BNode *new_child = new BNode(full->is_leaf);

        int t = ORDER / 2; // =2

        // перенос правой части в new_child
        for (int j = t; j < ORDER - 1; j++)
        {
            new_child->keys.push_back(full->keys[j]);
        }

        // перенос детей
        if (!full->is_leaf)
        {
            for (int j = t; j < ORDER; j++)
            {
                new_child->children.push_back(full->children[j]);
            }
        }

        // медианный ключ
        T median = full->keys[t - 1];

        // урезаем full
        while (full->keys.get_size() > (size_t)(t - 1))
            full->keys.pop_back();

        if (!full->is_leaf)
        {
            while (full->children.get_size() > (size_t)t)
                full->children.pop_back();
        }

        // вставляем медиану в родителя
        parent->keys.insert(i, median);

        // вставляем new_child справа
        parent->children.insert(i + 1, new_child);
    }

    // вставка, когда узел не полный
    void insert_non_full(BNode *node, const T &key)
    {
        int i = node->keys.get_size() - 1;

        if (node->is_leaf)
        {
            while (i >= 0 && key < node->keys[i])
                i--;
            node->keys.insert(i + 1, key);
        }
        else
        {
            while (i >= 0 && key < node->keys[i])
                i--;
            i++;

            if (node->children[i]->keys.get_size() == ORDER - 1)
            {
                split_child(node, i);
                if (key > node->keys[i])
                    i++;
            }

            insert_non_full(node->children[i], key);
        }
    }

public:
    BTree() : size(0)
    {
        root = new BNode(true);
    }

    ~BTree()
    {
        clear();
    }

    void insert(const T &key)
    {
        if (root->keys.get_size() == ORDER - 1)
        {
            BNode *new_root = new BNode(false);
            new_root->children.push_back(root);

            split_child(new_root, 0);

            root = new_root;
        }
        insert_non_full(root, key);
        size++;
    }

    bool contains(const T &key) const
    {
        size_t idx = 0;
        return search_node(root, key, idx) != nullptr;
    }

    T *search(const T &key) const
    {
        size_t idx = 0;
        BNode *n = search_node(root, key, idx);
        return n ? &n->keys[idx] : nullptr;
    }

    bool search_slow(const T &key) const
    {
        volatile int x = 0;
        for (int i = 0; i < 50000; i++)
            x += i;
        return contains(key);
    }

    void clear()
    {
        delete root;
        root = new BNode(true);
        size = 0;
    }

    size_t get_size() const { return size; }
};
