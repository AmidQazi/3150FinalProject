#include "../include/TimedSharedPtr.h"
#include <iostream>
#include <string>

template<typename T_obj>
void display_ptr_status(std::ostream& output, TimeLimitedSharedPtr<T_obj>& t_ptr) {
    T_obj* actual_ptr = t_ptr.get();
    if (actual_ptr) {
        output << static_cast<void*>(actual_ptr);
    } else {
        output << "Yeo! Expired 0"; 
    }
}

int main() {
    // Create a TimeLimitedSharedPtr managing a ListNode with a 100 ms lifetime
    TimeLimitedSharedPtr<ListNode> firstNode(new ListNode(7), 100);
    TimeLimitedSharedPtr<ListNode> copyNode = firstNode;

    std::this_thread::sleep_until(HighResClock::now() + std::chrono::milliseconds(50)); // Wait 50 ms
    std::cout << "firstNode.get() address: <";
    display_ptr_status(std::cout, firstNode);
    std::cout << ">" << std::endl;

    std::cout << "firstNode.use_count(): " << firstNode.use_count() << std::endl;
    std::cout << "copyNode.use_count(): " << copyNode.use_count() << std::endl;

    std::this_thread::sleep_until(HighResClock::now() + std::chrono::milliseconds(25)); // Wait 25 ms more
    std::cout << "firstNode.get() address: <";
    display_ptr_status(std::cout, firstNode);
    std::cout << ">" << std::endl;

    std::this_thread::sleep_until(HighResClock::now() + std::chrono::milliseconds(75)); // Total wait: 150 ms
    std::cout << "Expecting expiration:" << std::endl;
    std::cout << "firstNode.get() address: <";
    display_ptr_status(std::cout, firstNode);
    std::cout << ">" << std::endl;

    std::cout << "-----------" << std::endl;

    TimeLimitedSharedPtr<int> intPtr(new int(42));

    if (intPtr.get()) {
        std::cout << static_cast<void*>(intPtr.get()) << std::endl;
    } else {
        std::cout << "Yeo! Expired 0" << std::endl;
    }

    std::cout << "intPtr.use_count(): " << intPtr.use_count() << std::endl;

    TimeLimitedSharedPtr<int> intPtrCopy = intPtr;
    std::cout << "intPtr.use_count(): " << intPtr.use_count() << std::endl;
    std::cout << "intPtrCopy.use_count(): " << intPtrCopy.use_count() << std::endl;

    return 0;
}
