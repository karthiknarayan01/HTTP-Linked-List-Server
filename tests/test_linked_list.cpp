#include "test_runner.h"
#include "linked_list.h"

#include <stdexcept>

void test_linked_list() {
    begin_suite("LinkedList – construction & empty state");
    {
        LinkedList l;
        EXPECT_TRUE(l.empty());
        EXPECT_EQ(l.size(), 0);
        EXPECT_EQ(l.to_vector().size(), 0u);
    }

    begin_suite("LinkedList – push_front / pop_front");
    {
        LinkedList l;
        l.push_front(1);
        EXPECT_FALSE(l.empty());
        EXPECT_EQ(l.size(), 1);

        l.push_front(2);
        l.push_front(3);
        // order should be [3, 2, 1]
        EXPECT_EQ(l.get_at(1), 3);
        EXPECT_EQ(l.get_at(2), 2);
        EXPECT_EQ(l.get_at(3), 1);

        int v = l.pop_front();
        EXPECT_EQ(v, 3);
        EXPECT_EQ(l.size(), 2);
    }
    {
        LinkedList l;
        EXPECT_THROW(l.pop_front(), std::runtime_error);
    }

    begin_suite("LinkedList – push_back / pop_back");
    {
        LinkedList l;
        l.push_back(10);
        l.push_back(20);
        l.push_back(30);
        // order: [10, 20, 30]
        EXPECT_EQ(l.get_at(3), 30);

        int v = l.pop_back();
        EXPECT_EQ(v, 30);
        EXPECT_EQ(l.size(), 2);
        EXPECT_EQ(l.get_at(2), 20);
    }
    {
        LinkedList l;
        EXPECT_THROW(l.pop_back(), std::runtime_error);
    }

    begin_suite("LinkedList – single element edge cases");
    {
        LinkedList l;
        l.push_back(42);
        EXPECT_EQ(l.size(), 1);

        int v = l.pop_front();   // should leave list empty
        EXPECT_EQ(v, 42);
        EXPECT_TRUE(l.empty());
    }
    {
        LinkedList l;
        l.push_front(42);
        int v = l.pop_back();    // should leave list empty
        EXPECT_EQ(v, 42);
        EXPECT_TRUE(l.empty());
    }

    begin_suite("LinkedList – get_at");
    {
        LinkedList l;
        l.push_back(1); l.push_back(2); l.push_back(3);
        EXPECT_EQ(l.get_at(1), 1);
        EXPECT_EQ(l.get_at(2), 2);
        EXPECT_EQ(l.get_at(3), 3);
        EXPECT_THROW(l.get_at(4), std::runtime_error);
        EXPECT_THROW(l.get_at(0), std::runtime_error);
    }

    begin_suite("LinkedList – insert_after");
    {
        LinkedList l;
        l.push_back(1); l.push_back(3);

        l.insert_after(1, 2);  // [1, 2, 3]
        EXPECT_EQ(l.size(), 3);
        EXPECT_EQ(l.get_at(2), 2);

        l.insert_after(3, 4);  // [1, 2, 3, 4]  — after tail
        EXPECT_EQ(l.size(), 4);
        EXPECT_EQ(l.get_at(4), 4);

        l.insert_after(0, 0);  // position <= 0 → push_front
        EXPECT_EQ(l.get_at(1), 0);
        EXPECT_EQ(l.size(), 5);
    }

    begin_suite("LinkedList – remove_at");
    {
        LinkedList l;
        l.push_back(10); l.push_back(20); l.push_back(30);

        int v = l.remove_at(2);  // remove middle
        EXPECT_EQ(v, 20);
        EXPECT_EQ(l.size(), 2);
        EXPECT_EQ(l.get_at(1), 10);
        EXPECT_EQ(l.get_at(2), 30);

        v = l.remove_at(1);     // remove head
        EXPECT_EQ(v, 10);
        EXPECT_EQ(l.size(), 1);

        v = l.remove_at(1);     // remove last remaining
        EXPECT_EQ(v, 30);
        EXPECT_TRUE(l.empty());
    }
    {
        LinkedList l;
        EXPECT_THROW(l.remove_at(1), std::runtime_error);
    }

    begin_suite("LinkedList – update_at");
    {
        LinkedList l;
        l.push_back(1); l.push_back(2); l.push_back(3);
        l.update_at(2, 99);
        EXPECT_EQ(l.get_at(2), 99);
        EXPECT_EQ(l.get_at(1), 1);
        EXPECT_EQ(l.get_at(3), 3);
        EXPECT_THROW(l.update_at(4, 0), std::runtime_error);
    }

    begin_suite("LinkedList – sort");
    {
        LinkedList l;
        l.push_back(30); l.push_back(10); l.push_back(50); l.push_back(20); l.push_back(40);

        l.sort(true); // ascending
        auto v = l.to_vector();
        EXPECT_EQ(v[0], 10);
        EXPECT_EQ(v[1], 20);
        EXPECT_EQ(v[2], 30);
        EXPECT_EQ(v[3], 40);
        EXPECT_EQ(v[4], 50);

        l.sort(false); // descending
        v = l.to_vector();
        EXPECT_EQ(v[0], 50);
        EXPECT_EQ(v[1], 40);
        EXPECT_EQ(v[2], 30);
        EXPECT_EQ(v[3], 20);
        EXPECT_EQ(v[4], 10);
    }
    {
        // Sort already-sorted list
        LinkedList l;
        l.push_back(1); l.push_back(2); l.push_back(3);
        l.sort(true);
        EXPECT_EQ(l.get_at(1), 1);
        EXPECT_EQ(l.get_at(3), 3);
    }
    {
        // Sort single-element list
        LinkedList l;
        l.push_back(7);
        l.sort(true);
        EXPECT_EQ(l.get_at(1), 7);
    }

    begin_suite("LinkedList – clear");
    {
        LinkedList l;
        l.push_back(1); l.push_back(2); l.push_back(3);
        l.clear();
        EXPECT_TRUE(l.empty());
        EXPECT_EQ(l.size(), 0);
        // Should be safe to push again after clear
        l.push_back(99);
        EXPECT_EQ(l.get_at(1), 99);
    }

    begin_suite("LinkedList – copy constructor");
    {
        LinkedList a;
        a.push_back(1); a.push_back(2); a.push_back(3);

        LinkedList b(a);
        EXPECT_EQ(b.size(), 3);
        EXPECT_EQ(b.get_at(1), 1);
        EXPECT_EQ(b.get_at(3), 3);

        // Mutations to b must not affect a
        b.push_front(0);
        EXPECT_EQ(a.size(), 3);
    }

    begin_suite("LinkedList – operator=");
    {
        LinkedList a;
        a.push_back(10); a.push_back(20);

        LinkedList b;
        b = a;
        EXPECT_EQ(b.size(), 2);
        EXPECT_EQ(b.get_at(1), 10);

        // Self-assignment must not corrupt
        b = b;
        EXPECT_EQ(b.size(), 2);
    }

    begin_suite("LinkedList – operator== / operator!=");
    {
        LinkedList a, b;
        a.push_back(1); a.push_back(2);
        b.push_back(1); b.push_back(2);
        EXPECT_TRUE(a == b);
        EXPECT_FALSE(a != b);

        b.push_back(3);
        EXPECT_FALSE(a == b);
        EXPECT_TRUE(a != b);

        LinkedList c;
        EXPECT_FALSE(a == c);
    }

    begin_suite("LinkedList – operator+ (concatenation)");
    {
        LinkedList a, b;
        a.push_back(1); a.push_back(2);
        b.push_back(3); b.push_back(4);

        LinkedList c = a + b;
        EXPECT_EQ(c.size(), 4);
        EXPECT_EQ(c.get_at(1), 1);
        EXPECT_EQ(c.get_at(3), 3);
        EXPECT_EQ(c.get_at(4), 4);

        // Originals unchanged
        EXPECT_EQ(a.size(), 2);
        EXPECT_EQ(b.size(), 2);
    }

    begin_suite("LinkedList – to_vector");
    {
        LinkedList l;
        l.push_back(5); l.push_back(10); l.push_back(15);
        auto v = l.to_vector();
        EXPECT_EQ(static_cast<int>(v.size()), 3);
        EXPECT_EQ(v[0], 5);
        EXPECT_EQ(v[1], 10);
        EXPECT_EQ(v[2], 15);
    }
}
