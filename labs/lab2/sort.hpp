#pragma once

#include <cstdint>
#include <utility> // std::move
#include <chrono> 
#include <atomic> 
#include <thread> 
#include <iterator> // use std::iterator_traits & random_access_concepts

static std::atomic<uint8_t> JustWorks = 0u;

extern uint8_t MaxWorks; // defined in test.cpp

template <std::random_access_iterator Iterator>
inline void CAS(Iterator first_iterator, Iterator second_iterator, bool increasing) {
  if (increasing) {
    if (*first_iterator > *second_iterator) {
      std::swap(*first_iterator,  *second_iterator);
    }
  } else {
    if ((*first_iterator  < *second_iterator)) {
      std::swap(*first_iterator, *second_iterator);
    }
  }
}

template <std::random_access_iterator Iterator>
void BitonicMerge(Iterator start_pos, Iterator end_pos, bool increasing) {
    auto size = std::distance(start_pos, end_pos);
    if (size < 2) {
      return;
    }
    uint64_t median = size / 2;
    for (Iterator first = start_pos, last = start_pos + median; first < last; ++first) {
      CAS(first, first + median, increasing);
    }
    Iterator last = start_pos + median;
    if (sizeof(typename std::iterator_traits<Iterator>::value_type) * size <= 64u) { // cache line size = 64 byte
      BitonicMerge(start_pos, last, increasing);
      BitonicMerge(last, end_pos, increasing);
    } else if (JustWorks.fetch_add(1, std::memory_order_acq_rel) >= MaxWorks) {
      JustWorks.fetch_sub(1, std::memory_order_release);
      BitonicMerge(start_pos, last, increasing);
      BitonicMerge(last, end_pos, increasing);
    } else {
      auto call_back = [start_pos, last, increasing] mutable {BitonicMerge(start_pos, last, increasing);};
      std::thread helper(std::move(call_back));
      BitonicMerge(last, end_pos, increasing);
      helper.join();
      JustWorks.fetch_sub(1, std::memory_order_release); 
    }
}

template <std::random_access_iterator Iterator>
void BitonicSort(Iterator start_pos, Iterator end_pos, bool increasing = true) {
  auto size = std::distance(start_pos, end_pos);
  if (size < 2) {
    return;
  }
  uint64_t median = size / 2;
  Iterator last = start_pos + median;
  if (sizeof(typename std::iterator_traits<Iterator>::value_type) * size <= 64u) { // cache line size = 64 byte
    BitonicSort(start_pos, last, increasing);
    BitonicSort(last, end_pos, !increasing);
  } else if (JustWorks.fetch_add(1, std::memory_order_acq_rel) >= MaxWorks) { 
    JustWorks.fetch_sub(1, std::memory_order_release);
    BitonicSort(start_pos, last, increasing);
    BitonicSort(last, end_pos, !increasing);
  } else {
    auto call_back = [start_pos, last, increasing] mutable {BitonicSort(start_pos, last, increasing);};
    std::thread helper(std::move(call_back));
    BitonicSort(last, end_pos, !increasing);
    helper.join();
    JustWorks.fetch_sub(1, std::memory_order_release);
  }
  BitonicMerge(start_pos, end_pos, increasing);
}

template <std::random_access_iterator Iterator>
auto GetTimeForFunctionCall(Iterator start_pos, Iterator end_pos, bool increasing = true) {
  auto start = std::chrono::high_resolution_clock::now();
  JustWorks.store(1u, std::memory_order_release);
  BitonicSort(start_pos, end_pos, increasing);
  auto end = std::chrono::high_resolution_clock::now();
  auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
  return duration.count();
}
