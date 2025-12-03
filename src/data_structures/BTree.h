#pragma once

#include "Sequence.h"

template <typename T>
class BTree
{
private:
    static constexpr int ORDER = 4; // max children per node

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

    BNode *search_node(BNode *node, const T &key, size_t &out_index) const
    {
        size_t i = 0;
        while (i < node->keys.get_size() && key > node->keys[i])
            i++;

        if (i < node->keys.get_size() && key == node->keys[i])
        {
            out_index = i;
            return node;
        }

        if (node->is_leaf)
            return nullptr;
        return search_node(node->children[i], key, out_index);
    }

    void insert_non_full(BNode *node, const T &key)
    {
        int i = static_cast<int>(node->keys.get_size()) - 1;

        if (node->is_leaf)
        {
            while (i >= 0 && key < node->keys[i])
                i--;
            node->keys.insert(static_cast<size_t>(i + 1), key);
        }
        else
        {
            while (i >= 0 && key < node->keys[i])
                i--;
            i++;
            BNode *child = node->children[static_cast<size_t>(i)];

            if (static_cast<int>(child->keys.get_size()) >= ORDER - 1)
            {
                split_child(node, static_cast<int>(i));
                if (key > node->keys[static_cast<size_t>(i)])
                    i++;
                child = node->children[static_cast<size_t>(i)];
            }
            insert_non_full(child, key);
        }
    }

    void split_child(BNode *parent, int i)
    {
        BNode *full_child = parent->children[static_cast<size_t>(i)];
        BNode *new_child = new BNode(full_child->is_leaf);

        int mid = (ORDER - 1) / 2; // e.g., ORDER=4 -> mid=1

        // save median
        T median_key = full_child->keys[static_cast<size_t>(mid)];

        // move right half keys to new_child
        for (size_t j = static_cast<size_t>(mid + 1); j < full_child->keys.get_size(); ++j)
        {
            new_child->keys.push_back(full_child->keys[j]);
        }

        // move children if any
        if (!full_child->is_leaf)
        {
            for (size_t j = static_cast<size_t>(mid + 1); j < full_child->children.get_size(); ++j)
            {
                new_child->children.push_back(full_child->children[j]);
            }
        }

        // shrink full_child
        while (full_child->keys.get_size() > static_cast<size_t>(mid))
            full_child->keys.pop_back();

        if (!full_child->is_leaf)
        {
            while (full_child->children.get_size() > static_cast<size_t>(mid + 1))
                full_child->children.pop_back();
        }

        // insert median into parent
        parent->keys.insert(static_cast<size_t>(i), median_key);

        // insert new_child as parent's child at position i+1
        parent->children.insert(static_cast<size_t>(i + 1), new_child);
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
        size_t idx = 0;
        return search_node(root, key, idx) != nullptr;
    }

    // Return pointer to stored key (valid until node deletion)
    T *search(const T &key) const
    {
        size_t idx = 0;
        BNode *node = search_node(root, key, idx);
        if (node)
            return const_cast<T *>(&node->keys[idx]);
        return nullptr;
    }

    bool search_slow(const T &key) const
    {
        volatile int dummy = 0;
        for (int i = 0; i < 100000; ++i)
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
