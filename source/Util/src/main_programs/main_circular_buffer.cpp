#include <stdio.h>
#include <iostream>
#include "circular_buffer.hpp"

// printdivider() - local utility function
// message can be null
void printdivider(const char *message)
{
	if (message)
	{
		std::cout << "\n====================== " << message << "\n" << std::endl;
	} else {
		std::cout << "\n=====================================\n" << std::endl;
	}
}

int main(void)
{
	using namespace Util;

    circular_buffer<uint32_t> ringbuf(10);

    std::cout << "\n   === CPP circular buffer check ===\n" << std::endl;
    std::cout << "Size: " << ringbuf.size() << std::endl;
    std::cout << "Capacity: " << ringbuf.capacity() << std::endl;
    std::cout << "Empty: " << ringbuf.empty() << std::endl;
    std::cout << "Full: " << ringbuf.full() << std::endl;

    uint32_t x = 1;
    std::cout << "Push one, val: " << x << std::endl;

    ringbuf.put(x);
    x = ringbuf.get();
    std::cout << "Popped: " << x << std::endl;
    std::cout << "Empty: " << ringbuf.empty() << std::endl;
    std::cout << "Full: " << ringbuf.full() << std::endl;

    std::cout << "\nAdding " << ringbuf.capacity() << " values" << std::endl;

    for(auto i = 0; i < ringbuf.capacity() - 1; i++)
    {
        ringbuf.put(i);
    }

    std::cout << "Size: " << ringbuf.size() << std::endl;
    std::cout << "Capacity: " << ringbuf.capacity() << std::endl;
    std::cout << "Empty: " << ringbuf.empty() << std::endl;
    std::cout << "Full: " << ringbuf.full() << std::endl;

    printdivider("circular buffer reset");
    ringbuf.reset();

    printf("Full: %d\n", ringbuf.full());

    printf("\nAdding %zu values\n", ringbuf.capacity());
    for(uint32_t i = 0; i < ringbuf.capacity(); i++)
    {
        ringbuf.put(i);
    }

    std::cout << "Size: " << ringbuf.size() << std::endl;
    std::cout << "Capacity: " << ringbuf.capacity() << std::endl;
    std::cout << "Empty: " << ringbuf.empty() << std::endl;
    std::cout << "Full: " << ringbuf.full() << std::endl;

    printf("\nReading back values: ");
    while(!ringbuf.empty())
    {
        printf("%u ", ringbuf.get());
    }
    printf("\n");

    printf("Adding 15 values\n");
    for(uint32_t i = 0; i < ringbuf.size() + 5; i++)
    {
        ringbuf.put(i);
    }

    printf("Full: %d\n", ringbuf.full());

    printf("Reading back values (and empty out the object): ");
    while(!ringbuf.empty())
    {
        printf("%u ", ringbuf.get());
    }
    printf("\n");

    printf("Empty: %d\n", ringbuf.empty());
    printf("Full: %d\n", ringbuf.full());

    // The buffer is now empty

    printf("---------------------------------------------------------\n");

    circular_buffer<uint32_t> ringbuf15(15);
    printf("Obtaining members in ringbuf5 deque:\n");

    // Add members
    for(uint32_t i = 0; i < ringbuf15.capacity(); i++)
    {
        printf("Adding %d\n", i+10);
        ringbuf15.put(i+10);
    }
    ringbuf15.put(31); printf("Adding 31\n");
    ringbuf15.put(32); printf("Adding 32\n");

    printf("Size: %zu, Capacity: %zu\n", ringbuf15.size(), ringbuf15.capacity());

    std::deque<uint32_t> res;

    printf("Deque size: %zu\n", res.size());

    printf ("Get latest 5 members: ");
    ringbuf15.get_members_in_deque(res, 5);

    for (size_t i = 0; i < res.size(); i++)
    {
        printf("%zu ", res[i]);
    }
    printf("\n");
    printf("Deque size: %zu\n", res.size());

    printf ("Get latest all members: ");
    ringbuf15.get_members_in_deque(res);

    for (size_t i = 0; i < res.size(); i++)
    {
        printf("%zu ", res[i]);
    }
    printf("\n");

    printf("Deque size: %zu\n", res.size());

    // Partially full
    ringbuf15.reset();
    printdivider("ringbuf5 partially full - reset");

    // Add members
    for(uint32_t i = 0; i < (ringbuf15.capacity()/2); i++)
    {
        printf("Adding %d\n", i+10);
        ringbuf15.put(i+10);
    }
    ringbuf15.put(31); printf("Adding 31\n");
    ringbuf15.put(32); printf("Adding 32\n");

    printf("Size: %zu, Capacity: %zu\n", ringbuf15.size(), ringbuf15.capacity());

    printf ("Get latest 5 members: ");
    ringbuf15.get_members_in_deque(res, 5);

    for (size_t i = 0; i < res.size(); i++)
    {
        printf("%zu ", res[i]);
    }
    printf("\n");
    printf("Deque size: %zu\n", res.size());

    printf ("Get latest all members: ");
    ringbuf15.get_members_in_deque(res);

    for (size_t i = 0; i < res.size(); i++)
    {
        printf("%zu ", res[i]);
    }
    printf("\n");

    printf("Deque size: %zu\n", res.size());

    printf("\nTraversing the deque in reverse (last member to first): ");

    std::deque<uint32_t>::reverse_iterator rit;
    for (rit = res.rbegin(); rit!= res.rend(); ++rit)
    {
        printf ("%d ", *rit);
    }
    printf("\n");

    return 0;
}
