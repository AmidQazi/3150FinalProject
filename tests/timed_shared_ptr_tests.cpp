#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "../include/doctest.h"
#include "../include/TimedSharedPtr.h"

#include <thread>
#include <chrono>

// Testing TimeLimitedSharedPtr behaviors
TEST_CASE("Test 1: Creation, Access, Expiry Behavior") {
    TimeLimitedSharedPtr<ListNode> testPtr(new ListNode(10), 50);

    REQUIRE(testPtr.get() != nullptr);
    CHECK(testPtr->value == 10);
    CHECK(testPtr.use_count() == 1);

    std::this_thread::sleep_for(std::chrono::milliseconds(25));
    REQUIRE(testPtr.get() != nullptr);

    std::this_thread::sleep_for(std::chrono::milliseconds(30));
    CHECK(testPtr.get() == nullptr);
    CHECK(testPtr.use_count() == 1);
}

TEST_CASE("Test 2: Copy Constructor & Use Count Validation") {
    TimeLimitedSharedPtr<int> ptr1(new int(100), 1000);
    REQUIRE(ptr1.use_count() == 1);

    TimeLimitedSharedPtr<int> ptr2 = ptr1;
    CHECK(ptr1.use_count() == 2);
    CHECK(ptr2.use_count() == 2);
    REQUIRE(ptr2.get() != nullptr);
    CHECK(*ptr1 == *ptr2);
    CHECK(ptr1.get() == ptr2.get());
}

TEST_CASE("Test 3: Default Timeout and Unlimited Lifetime") {
    TimeLimitedSharedPtr<ListNode> timeoutDefault(new ListNode(20));
    REQUIRE(timeoutDefault.get() != nullptr);
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    CHECK(timeoutDefault.get() != nullptr);

    TimeLimitedSharedPtr<ListNode> noExpiry(new ListNode(30), -1);
    REQUIRE(noExpiry.get() != nullptr);
    std::this_thread::sleep_for(std::chrono::milliseconds(60));
    CHECK(noExpiry.get() != nullptr);
}

TEST_CASE("Test 4: Reset Functionality Verification") {
    TimeLimitedSharedPtr<ListNode> resettablePtr(new ListNode(70), 200);
    REQUIRE(resettablePtr.get() != nullptr);
    CHECK(resettablePtr.use_count() == 1);

    resettablePtr.reset(new ListNode(71), 50);
    REQUIRE(resettablePtr.get() != nullptr);
    CHECK(resettablePtr->value == 71);
    CHECK(resettablePtr.use_count() == 1);

    std::this_thread::sleep_for(std::chrono::milliseconds(60));
    CHECK(resettablePtr.get() == nullptr);

    resettablePtr.reset();
    CHECK(resettablePtr.get() == nullptr);
    CHECK(resettablePtr.use_count() == 0);
}

TEST_CASE("Test 5: Resource Management on Scope Exit") {
    TimeLimitedSharedPtr<int> outerPtr(new int(80), 1000);
    CHECK(outerPtr.use_count() == 1);
    {
        TimeLimitedSharedPtr<int> innerPtr = outerPtr;
        CHECK(outerPtr.use_count() == 2);
        CHECK(innerPtr.use_count() == 2);
    }
    CHECK(outerPtr.use_count() == 1);
}
