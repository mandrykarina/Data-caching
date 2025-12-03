#pragma once

template <typename T>
class Iterator
{
private:
    T *ptr;

public:
    Iterator(T *p = nullptr) : ptr(p) {}

    T &operator*() { return *ptr; }
    const T &operator*() const { return *ptr; }
    T *operator->() { return ptr; }

    Iterator &operator++()
    {
        ++ptr;
        return *this;
    }
    Iterator operator++(int)
    {
        Iterator tmp(ptr);
        ++ptr;
        return tmp;
    }
    Iterator &operator--()
    {
        --ptr;
        return *this;
    }

    bool operator==(const Iterator &other) const { return ptr == other.ptr; }
    bool operator!=(const Iterator &other) const { return ptr != other.ptr; }
    bool operator<(const Iterator &other) const { return ptr < other.ptr; }
};
