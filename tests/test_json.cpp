#include "test_runner.h"
#include "json.h"

#include <string>

void test_json() {
    begin_suite("json::get – string values");
    {
        std::string j = R"({"key":"mylist","order":"asc"})";
        EXPECT_EQ(json::get(j, "key"),   "mylist");
        EXPECT_EQ(json::get(j, "order"), "asc");
        EXPECT_EQ(json::get(j, "missing"), "");
    }

    begin_suite("json::get – numeric values");
    {
        std::string j = R"({"value":42,"position":3})";
        EXPECT_EQ(json::get(j, "value"),    "42");
        EXPECT_EQ(json::get(j, "position"), "3");
    }

    begin_suite("json::get – edge cases");
    {
        // Empty object
        EXPECT_EQ(json::get("{}", "key"), "");

        // Key present with empty string value
        std::string j = R"({"key":""})";
        EXPECT_EQ(json::get(j, "key"), "");

        // Negative number
        std::string jneg = R"({"value":-5})";
        EXPECT_EQ(json::get(jneg, "value"), "-5");

        // Spaces around colon
        std::string jspaced = R"({"key" : "hello"})";
        EXPECT_EQ(json::get(jspaced, "key"), "hello");
    }

    begin_suite("json::make_error");
    {
        std::string r = json::make_error("not found");
        EXPECT_NE(r.find("\"error\""), std::string::npos);
        EXPECT_NE(r.find("not found"), std::string::npos);
    }

    begin_suite("json::make_created");
    {
        std::string r = json::make_created("scores");
        EXPECT_NE(r.find("\"scores\""),  std::string::npos);
        EXPECT_NE(r.find("\"created\""), std::string::npos);
        EXPECT_NE(r.find("true"),        std::string::npos);
    }

    begin_suite("json::make_list");
    {
        // Empty list
        std::string r = json::make_list("k", {});
        EXPECT_NE(r.find("\"k\""),  std::string::npos);
        EXPECT_NE(r.find("\"values\""), std::string::npos);
        EXPECT_NE(r.find("\"size\":0"), std::string::npos);

        // Non-empty list
        r = json::make_list("k", {1, 2, 3});
        EXPECT_NE(r.find("1"),          std::string::npos);
        EXPECT_NE(r.find("2"),          std::string::npos);
        EXPECT_NE(r.find("3"),          std::string::npos);
        EXPECT_NE(r.find("\"size\":3"), std::string::npos);
    }

    begin_suite("json::make_size");
    {
        std::string r = json::make_size("mykey", 7);
        EXPECT_NE(r.find("\"mykey\""),  std::string::npos);
        EXPECT_NE(r.find("\"size\":7"), std::string::npos);
    }

    begin_suite("json::make_node_value");
    {
        std::string r = json::make_node_value("mykey", 2, 99);
        EXPECT_NE(r.find("\"mykey\""),      std::string::npos);
        EXPECT_NE(r.find("\"position\":2"), std::string::npos);
        EXPECT_NE(r.find("\"value\":99"),   std::string::npos);
    }

    begin_suite("json::make_popped");
    {
        std::string r = json::make_popped("q", 55);
        EXPECT_NE(r.find("\"q\""),         std::string::npos);
        EXPECT_NE(r.find("\"popped\":55"), std::string::npos);
    }

    begin_suite("json::make_removed");
    {
        std::string r = json::make_removed("q", 77);
        EXPECT_NE(r.find("\"q\""),          std::string::npos);
        EXPECT_NE(r.find("\"removed\":77"), std::string::npos);
    }

    begin_suite("json::make_keys");
    {
        // Empty
        std::string r = json::make_keys({});
        EXPECT_NE(r.find("\"keys\""), std::string::npos);
        EXPECT_NE(r.find("[]"),       std::string::npos);

        // Multiple keys
        r = json::make_keys({"a", "b", "c"});
        EXPECT_NE(r.find("\"a\""), std::string::npos);
        EXPECT_NE(r.find("\"b\""), std::string::npos);
        EXPECT_NE(r.find("\"c\""), std::string::npos);
    }
}
