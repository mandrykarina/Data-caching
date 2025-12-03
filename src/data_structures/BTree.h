#pragma once

#include "Sequence.h"
#include <chrono>

template <typename T>
class BTree
{
private:
    static constexpr int ORDER = 4;

    struct BNode
    {
        Sequence<T> keys;
        Sequence<BNode *> children;
        bool is_leaf;

        BNode(bool leaf = true) : is_leaf(leaf) {}
        ~BNode()
        {
            if (!is_leaf)
            {
                for (size_t i = 0; i < children.get_size(); ++i)
                {
                    delete children[i];
                }
            }
        }
    };

    BNode *root;
    size_t size;

    BNode *search_node(BNode *node, const T &key) const
    {
        size_t i = 0;
        while (i < node->keys.get_size() && key > node->keys[i])
            i++;

        if (i < node->keys.get_size() && key == node->keys[i])
            return node;

        if (node->is_leaf)
            return nullptr;
        return search_node(node->children[i], key);
    }

    void insert_non_full(BNode *node, const T &key)
    {
        int i = static_cast<int>(node->keys.get_size()) - 1;

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
            BNode *child = node->children[i];

            if (static_cast<int>(child->keys.get_size()) >= ORDER - 1)
            {
                split_child(node, i);
                if (key > node->keys[i])
                    i++;
                child = node->children[i];
            }
            insert_non_full(child, key);
        }
    }

    void split_child(BNode *parent, int i)
    {
        BNode *full_child = parent->children[i];
        BNode *new_child = new BNode(full_child->is_leaf);
        int mid = (ORDER - 1) / 2;

        for (int j = 0; j < (ORDER - 1) - mid; j++)
        {
            new_child->keys.push_back(full_child->keys[mid + 1 + j]);
        }

        if (!full_child->is_leaf)
        {
            for (int j = 0; j <= (ORDER - 1) - mid; j++)
            {
                new_child->children.push_back(full_child->children[mid + 1 + j]);
            }
            for (int j = static_cast<int>(full_child->children.get_size()) - 1; j > mid; j--)
            {
                full_child->children.pop_back();
            }
        }

        for (int j = static_cast<int>(full_child->keys.get_size()) - 1; j > mid; j--)
        {
            full_child->keys.pop_back();
        }

        parent->keys.insert(i, full_child->keys[mid]);
        full_child->keys.pop_back();
        parent->children.insert(i + 1, new_child);
    }

    void clear_node(BNode *node)
    {
        if (node)
        {
            if (!node->is_leaf)
            {
                for (size_t i = 0; i < node->children.get_size(); ++i)
                {
                    clear_node(node->children[i]);
                }
            }
            delete node;
        }
    }

public:
    BTree() : root(nullptr), size(0)
    {
        root = new BNode(true);
    }

    ~BTree()
    {
        clear();
    }

    void insert(const T &key)
    {
        if (root->keys.get_size() >= ORDER - 1)
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
        return search_node(root, key) != nullptr;
    }

    bool search_slow(const T &key) const
    {
        // Имитируем задержку доступа
        volatile int dummy = 0;
        for (int i = 0; i < 100000; i++)
            dummy += i;
        return contains(key);
    }

    void clear()
    {
        clear_node(root);
        root = new BNode(true);
        size = 0;
    }

    size_t get_size() const { return size; }
};
