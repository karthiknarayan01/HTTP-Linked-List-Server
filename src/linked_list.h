#pragma once

#include <iostream>
#include <vector>
#include <stdexcept>

struct Node {
    int value;
    Node* next;
    explicit Node(int v) : value(v), next(nullptr) {}
};

class LinkedList {
public:
    LinkedList();
    LinkedList(const LinkedList& other);
    ~LinkedList();

    LinkedList& operator=(const LinkedList& other);
    bool operator==(const LinkedList& other) const;
    bool operator!=(const LinkedList& other) const;
    LinkedList operator+(const LinkedList& other) const;
    friend std::ostream& operator<<(std::ostream& os, const LinkedList& list);

    bool empty() const;
    int size() const;

    void push_front(int value);
    int pop_front();

    void push_back(int value);
    int pop_back();

    // 1-indexed position operations
    int get_at(int position) const;
    void insert_after(int position, int value);
    int remove_at(int position);
    void update_at(int position, int value);

    void sort(bool ascending = true);
    void clear();
    std::vector<int> to_vector() const;

private:
    Node* head_;
    Node* tail_;
    Node* node_at(int position) const;
};
