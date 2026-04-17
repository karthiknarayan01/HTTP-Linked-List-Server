#include "test_runner.h"

void test_linked_list();
void test_store();
void test_json();

int main() {
    test_linked_list();
    test_store();
    test_json();
    return report();
}
