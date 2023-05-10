# Introduction

- this repo contains an implementation for a simple thread-safe `malloc()` and `free()` function.
- `first fit` is chosen for allocation policy since it's relatively fast and simple. To avoid external fragmentation, alternative such as `next fit`, `quick fit` or `buddy system` should be explored (see below in [Memory allocation policies](#memory-allocation-policies))
- How `first fit` works: in this implementation, an in-place doubly linked-list is used to keep track of memory blocks. When a memory allocation is requested, the algorithm will scan through the linked list from `head` to `tail` and return the first memory block that satisfies the requested size. If such block doesn't exist, the data segment is increased via a call to `sbrk()`, then the search is repeated.

# Requirements

- C++20

# How to run

- the following command will build everything and run the unittests. Please make sure that you're in the project directory

```
make && ./unittests
```

# Memory allocation policies

- `OS Design & Implementation (Tanenbaum)` : chapter 4.2.1
  - first fit: simplest, fast; list often kept sorted in order of address => cause allocations to be clustered toward the low memory addresses => low memory area tends to get fragmented; while upper memory area tends to have larger free blocks
  - next fit: same as first fit, keep track of where it is whenever it finds a suitable hole
  - best fit: search entire list, find smallest hole, slower than first fit; resulted in more wasted memory -> fill up memory with tiny, useless holes
  - worst fit: take the largest hole. simulation show that worst fit is not a very good idea.
    - hole list sorted by size => first fit & best fit are equally fast, next fit is pointless
  - quick fit: keep of list of fixed size blocks


- `principle of OS: design & application (Stuart)` : chapter 9.5 (https://www.cs.drexel.edu/~bls96/excerpt3.pdf)
  - buddy system management: all allocated blocks are a power of 2 in size