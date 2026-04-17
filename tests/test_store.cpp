#include "test_runner.h"
#include "store.h"

#include <algorithm>
#include <stdexcept>

void test_store() {
    begin_suite("ListStore – create / exists");
    {
        ListStore s;
        EXPECT_FALSE(s.exists("a"));

        bool created = s.create("a");
        EXPECT_TRUE(created);
        EXPECT_TRUE(s.exists("a"));

        // Duplicate create returns false
        bool again = s.create("a");
        EXPECT_FALSE(again);
    }

    begin_suite("ListStore – drop");
    {
        ListStore s;
        s.create("x");
        EXPECT_TRUE(s.exists("x"));

        bool dropped = s.drop("x");
        EXPECT_TRUE(dropped);
        EXPECT_FALSE(s.exists("x"));

        // Drop nonexistent returns false
        bool nope = s.drop("x");
        EXPECT_FALSE(nope);
    }

    begin_suite("ListStore – keys");
    {
        ListStore s;
        EXPECT_EQ(static_cast<int>(s.keys().size()), 0);

        s.create("one");
        s.create("two");
        s.create("three");
        auto ks = s.keys();
        EXPECT_EQ(static_cast<int>(ks.size()), 3);

        // All three keys must be present (order is unspecified)
        EXPECT_TRUE(std::find(ks.begin(), ks.end(), "one")   != ks.end());
        EXPECT_TRUE(std::find(ks.begin(), ks.end(), "two")   != ks.end());
        EXPECT_TRUE(std::find(ks.begin(), ks.end(), "three") != ks.end());

        s.drop("two");
        ks = s.keys();
        EXPECT_EQ(static_cast<int>(ks.size()), 2);
        EXPECT_TRUE(std::find(ks.begin(), ks.end(), "two") == ks.end());
    }

    begin_suite("ListStore – get (mutable) / mutations persist");
    {
        ListStore s;
        s.create("mylist");

        LinkedList& l = s.get("mylist");
        l.push_back(1);
        l.push_back(2);
        l.push_back(3);

        // Retrieve again via get — same object in memory
        EXPECT_EQ(s.get("mylist").size(), 3);
        EXPECT_EQ(s.get("mylist").get_at(1), 1);
        EXPECT_EQ(s.get("mylist").get_at(3), 3);
    }

    begin_suite("ListStore – get throws on missing key");
    {
        ListStore s;
        EXPECT_THROW(s.get("nope"), std::runtime_error);
    }

    begin_suite("ListStore – multiple independent keys");
    {
        ListStore s;
        s.create("a");
        s.create("b");

        s.get("a").push_back(100);
        s.get("b").push_back(200);

        EXPECT_EQ(s.get("a").get_at(1), 100);
        EXPECT_EQ(s.get("b").get_at(1), 200);
        EXPECT_EQ(s.get("a").size(), 1);
        EXPECT_EQ(s.get("b").size(), 1);
    }

    begin_suite("ListStore – drop removes list state");
    {
        ListStore s;
        s.create("tmp");
        s.get("tmp").push_back(42);

        s.drop("tmp");
        EXPECT_FALSE(s.exists("tmp"));

        // Re-create gives a fresh empty list
        s.create("tmp");
        EXPECT_EQ(s.get("tmp").size(), 0);
    }
}
