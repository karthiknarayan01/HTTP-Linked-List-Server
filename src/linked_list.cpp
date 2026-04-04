#include "linked_list.h"

#include <algorithm>
#include <stdexcept>

LinkedList::LinkedList() : head_(nullptr), tail_(nullptr) {}

LinkedList::LinkedList(const LinkedList& other) : head_(nullptr), tail_(nullptr) {
    for (Node* cur = other.head_; cur != nullptr; cur = cur->next)
        push_back(cur->value);
}

LinkedList::~LinkedList() {
    clear();
}

LinkedList& LinkedList::operator=(const LinkedList& other) {
    if (this == &other) return *this;
    clear();
    for (Node* cur = other.head_; cur != nullptr; cur = cur->next)
        push_back(cur->value);
    return *this;
}

bool LinkedList::operator==(const LinkedList& other) const {
    Node* a = head_;
    Node* b = other.head_;
    while (a != nullptr && b != nullptr) {
        if (a->value != b->value) return false;
        a = a->next;
        b = b->next;
    }
    return a == nullptr && b == nullptr;
}

bool LinkedList::operator!=(const LinkedList& other) const {
    return !(*this == other);
}

LinkedList LinkedList::operator+(const LinkedList& other) const {
    LinkedList result(*this);
    for (Node* cur = other.head_; cur != nullptr; cur = cur->next)
        result.push_back(cur->value);
    return result;
}

std::ostream& operator<<(std::ostream& os, const LinkedList& list) {
    os << "[";
    for (Node* cur = list.head_; cur != nullptr; cur = cur->next) {
        os << cur->value;
        if (cur->next != nullptr) os << " -> ";
    }
    os << "]";
    return os;
}

bool LinkedList::empty() const {
    return head_ == nullptr;
}

int LinkedList::size() const {
    int count = 0;
    for (Node* cur = head_; cur != nullptr; cur = cur->next)
        ++count;
    return count;
}

void LinkedList::push_front(int value) {
    Node* node = new Node(value);
    if (empty()) {
        head_ = tail_ = node;
    } else {
        node->next = head_;
        head_ = node;
    }
}

int LinkedList::pop_front() {
    if (empty())
        throw std::runtime_error("Cannot pop from an empty list");
    int val = head_->value;
    Node* old = head_;
    head_ = head_->next;
    if (head_ == nullptr) tail_ = nullptr;
    delete old;
    return val;
}

void LinkedList::push_back(int value) {
    Node* node = new Node(value);
    if (empty()) {
        head_ = tail_ = node;
    } else {
        tail_->next = node;
        tail_ = node;
    }
}

int LinkedList::pop_back() {
    if (empty())
        throw std::runtime_error("Cannot pop from an empty list");
    int val = tail_->value;
    if (head_ == tail_) {
        delete tail_;
        head_ = tail_ = nullptr;
    } else {
        Node* cur = head_;
        while (cur->next != tail_) cur = cur->next;
        delete tail_;
        tail_ = cur;
        tail_->next = nullptr;
    }
    return val;
}

Node* LinkedList::node_at(int position) const {
    Node* cur = head_;
    for (int i = 1; i < position && cur != nullptr; ++i)
        cur = cur->next;
    return cur;
}

int LinkedList::get_at(int position) const {
    Node* node = node_at(position);
    if (node == nullptr)
        throw std::runtime_error("Position out of range");
    return node->value;
}

void LinkedList::insert_after(int position, int value) {
    if (position <= 0) {
        push_front(value);
        return;
    }
    Node* target = node_at(position);
    if (target == nullptr)
        throw std::runtime_error("Position out of range");
    Node* node = new Node(value);
    node->next = target->next;
    target->next = node;
    if (node->next == nullptr) tail_ = node;
}

int LinkedList::remove_at(int position) {
    if (empty())
        throw std::runtime_error("Cannot remove from an empty list");
    if (position <= 0)
        throw std::runtime_error("Position must be >= 1");
    if (position == 1)
        return pop_front();

    Node* prev = node_at(position - 1);
    if (prev == nullptr || prev->next == nullptr)
        throw std::runtime_error("Position out of range");

    Node* target = prev->next;
    int val = target->value;
    prev->next = target->next;
    if (target->next == nullptr) tail_ = prev;
    delete target;
    return val;
}

void LinkedList::update_at(int position, int value) {
    Node* node = node_at(position);
    if (node == nullptr)
        throw std::runtime_error("Position out of range");
    node->value = value;
}

void LinkedList::sort(bool ascending) {
    // Selection sort by swapping values (O(n²), matches original algorithm)
    for (Node* i = head_; i != nullptr; i = i->next) {
        for (Node* j = i->next; j != nullptr; j = j->next) {
            bool out_of_order = ascending ? (i->value > j->value) : (i->value < j->value);
            if (out_of_order) std::swap(i->value, j->value);
        }
    }
}

void LinkedList::clear() {
    while (head_ != nullptr) {
        Node* temp = head_;
        head_ = head_->next;
        delete temp;
    }
    tail_ = nullptr;
}

std::vector<int> LinkedList::to_vector() const {
    std::vector<int> result;
    for (Node* cur = head_; cur != nullptr; cur = cur->next)
        result.push_back(cur->value);
    return result;
}
