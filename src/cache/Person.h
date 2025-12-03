#pragma once

#include <string>
#include <iostream>

struct Person
{
    int id;
    std::string name;
    int age;
    std::string email;

    Person() : id(0), age(0) {}
    Person(int id, const std::string &name, int age, const std::string &email)
        : id(id), name(name), age(age), email(email) {}

    bool operator<(const Person &other) const { return id < other.id; }
    bool operator>(const Person &other) const { return id > other.id; }
    bool operator==(const Person &other) const { return id == other.id; }
    bool operator<=(const Person &other) const { return id <= other.id; }
    bool operator>=(const Person &other) const { return id >= other.id; }
    bool operator!=(const Person &other) const { return id != other.id; }

    std::string to_string() const
    {
        return "Person(id=" + std::to_string(id) + ", name=" + name +
               ", age=" + std::to_string(age) + ", email=" + email + ")";
    }
};

namespace std
{
    template <>
    struct hash<Person>
    {
        size_t operator()(const Person &p) const
        {
            return hash<int>()(p.id);
        }
    };
}
