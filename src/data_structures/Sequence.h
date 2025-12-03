#pragma once

#include <stdexcept>
#include <algorithm>

template <typename T>
class Sequence
{
private:
    T *data;
    size_t size;
    size_t capacity;
    static constexpr size_t INITIAL_CAPACITY = 16;

    void resize(size_t new_capacity)
    {
        if (new_capacity < INITIAL_CAPACITY)
            new_capacity = INITIAL_CAPACITY;

        T *new_data = new T[new_capacity];

        if (data != nullptr && size > 0)
        {
            std::copy(data, data + size, new_data);
        }

        delete[] data;
        data = new_data;
        capacity = new_capacity;
    }

public:
    Sequence() : data(nullptr), size(0), capacity(0)
    {
        resize(INITIAL_CAPACITY);
    }

    ~Sequence()
    {
        delete[] data;
    }

    Sequence(const Sequence &other) : data(nullptr), size(0), capacity(0)
    {
        resize(other.capacity);
        size = other.size;
        std::copy(other.data, other.data + other.size, data);
    }

    Sequence(Sequence &&other) noexcept
        : data(other.data), size(other.size), capacity(other.capacity)
    {
        other.data = nullptr;
        other.size = 0;
        other.capacity = 0;
    }

    Sequence &operator=(const Sequence &other)
    {
        if (this != &other)
        {
            resize(other.capacity);
            size = other.size;
            std::copy(other.data, other.data + other.size, data);
        }
        return *this;
    }

    Sequence &operator=(Sequence &&other) noexcept
    {
        if (this != &other)
        {
            delete[] data;

            data = other.data;
            size = other.size;
            capacity = other.capacity;

            other.data = nullptr;
            other.size = 0;
            other.capacity = 0;
        }
        return *this;
    }

    void push_back(const T &value)
    {
        if (capacity == 0)
            resize(INITIAL_CAPACITY);

        if (size >= capacity)
            resize(capacity * 2);
        data[size++] = value;
    }

    void pop_back()
    {
        if (size > 0)
        {
            size--;
            if (capacity > INITIAL_CAPACITY && size < capacity / 4)
                resize(capacity / 2);
        }
    }

    void insert(size_t index, const T &value)
    {
        if (index > size)
            throw std::out_of_range("Index out of range");

        if (size >= capacity)
            resize(capacity * 2);

        for (size_t i = size; i > index; --i)
            data[i] = data[i - 1];

        data[index] = value;
        size++;
    }

    void erase(size_t index)
    {
        if (index >= size)
            throw std::out_of_range("Index out of range");

        for (size_t i = index; i < size - 1; ++i)
            data[i] = data[i + 1];

        size--;

        if (capacity > INITIAL_CAPACITY && size < capacity / 4)
            resize(capacity / 2);
    }

    int find(const T &value) const
    {
        for (size_t i = 0; i < size; ++i)
        {
            if (data[i] == value)
                return static_cast<int>(i);
        }
        return -1;
    }

    void clear()
    {
        size = 0;
        if (capacity > INITIAL_CAPACITY)
            resize(INITIAL_CAPACITY);
    }

    T &operator[](size_t index)
    {
        if (index >= size)
            throw std::out_of_range("Index out of range");
        return data[index];
    }

    const T &operator[](size_t index) const
    {
        if (index >= size)
            throw std::out_of_range("Index out of range");
        return data[index];
    }

    size_t get_size() const { return size; }
    size_t get_capacity() const { return capacity; }
    bool is_empty() const { return size == 0; }

    T *begin() { return data; }
    T *end() { return data + size; }
    const T *begin() const { return data; }
    const T *end() const { return data + size; }
};
