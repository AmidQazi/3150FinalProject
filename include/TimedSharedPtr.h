#ifndef TIME_LIMITED_SHARED_PTR_H
#define TIME_LIMITED_SHARED_PTR_H

#include <iostream>
#include <chrono>
#include <atomic>
#include <thread>

using HighResClock = std::chrono::high_resolution_clock;
using TimeStamp = std::chrono::time_point<std::chrono::high_resolution_clock>;
using Milliseconds = std::chrono::milliseconds;

template<typename U>
struct SharedResourceBlock {
    U* managed_ptr;
    std::atomic<long> reference_count;
    TimeStamp time_created;
    Milliseconds expiration_period;
    bool is_timed;

    SharedResourceBlock(U* ptr, long timeout_ms)
        : managed_ptr(ptr),
          reference_count(1),
          time_created(HighResClock::now()),
          expiration_period{Milliseconds(timeout_ms >= 0 ? timeout_ms : 0)},
          is_timed(timeout_ms >= 0)
    {
        std::cout << "SharedResourceBlock " << static_cast<void*>(managed_ptr)
                  << " initialized at: 0 ms" << std::endl;
    }

    ~SharedResourceBlock() {
        auto total_duration = std::chrono::duration_cast<Milliseconds>(
            HighResClock::now().time_since_epoch() - time_created.time_since_epoch()
        );
        std::cout << "SharedResourceBlock " << static_cast<void*>(managed_ptr)
                  << " destroyed at: " << total_duration.count()
                  << " ms" << std::endl;
        delete managed_ptr;
        managed_ptr = nullptr;
    }

    SharedResourceBlock(const SharedResourceBlock&) = delete;
    SharedResourceBlock& operator=(const SharedResourceBlock&) = delete;
};

template<typename U>
class TimeLimitedSharedPtr {
private:
    SharedResourceBlock<U>* control_block;

    void decrease_ref() {
        if (control_block) {
            if (control_block->reference_count.fetch_sub(1) == 1) {
                delete control_block;
            }
            control_block = nullptr;
        }
    }

public:
    TimeLimitedSharedPtr() : control_block(nullptr) {}

    explicit TimeLimitedSharedPtr(U* raw_ptr, long expiration_ms)
        : control_block(nullptr)
    {
        if (raw_ptr) {
            try {
                control_block = new SharedResourceBlock<U>(raw_ptr, expiration_ms);
            } catch (const std::bad_alloc&) {
                delete raw_ptr;
                throw;
            }
        }
    }

    explicit TimeLimitedSharedPtr(U* raw_ptr)
        : control_block(nullptr)
    {
        if (raw_ptr) {
            try {
                control_block = new SharedResourceBlock<U>(raw_ptr, 1000);
            } catch (const std::bad_alloc&) {
                delete raw_ptr;
                throw;
            }
        }
    }

    TimeLimitedSharedPtr(const TimeLimitedSharedPtr& other) : control_block(other.control_block) {
        if (control_block) {
            control_block->reference_count++;
        }
    }

    TimeLimitedSharedPtr& operator=(const TimeLimitedSharedPtr& other) {
        if (this != &other) {
            decrease_ref();
            control_block = other.control_block;
            if (control_block) {
                control_block->reference_count++;
            }
        }
        return *this;
    }

    long use_count() const noexcept {
        return control_block ? control_block->reference_count.load() : 0;
    }

    U* get() const {
        if (!control_block || !control_block->managed_ptr) {
            return nullptr;
        }

        if (control_block->is_timed) {
            auto elapsed = std::chrono::duration_cast<Milliseconds>(
                HighResClock::now() - control_block->time_created
            );
            if (elapsed >= control_block->expiration_period) {
                return nullptr;
            }
        }
        return control_block->managed_ptr;
    }

    ~TimeLimitedSharedPtr() {
        decrease_ref();
    }

    U& operator*() const {
        U* valid_ptr = get();
        if (!valid_ptr) {
            throw std::runtime_error("Attempted to dereference an expired or null TimeLimitedSharedPtr");
        }
        return *valid_ptr;
    }

    U* operator->() const {
        U* valid_ptr = get();
        if (!valid_ptr) {
            throw std::runtime_error("Attempted to access member of an expired or null TimeLimitedSharedPtr");
        }
        return valid_ptr;
    }

    void reset(U* new_ptr = nullptr, long expiration_override = -2) {
        decrease_ref();
        if (new_ptr) {
            long timeout_value = (expiration_override == -2) ? 1000 : expiration_override;
            try {
                control_block = new SharedResourceBlock<U>(new_ptr, timeout_value);
            } catch (const std::bad_alloc&) {
                delete new_ptr;
                throw;
            }
        }
    }
};

struct ListNode {
    ListNode(int number) : value(number) {}
    int value;
};

#endif // TIME_LIMITED_SHARED_PTR_H
