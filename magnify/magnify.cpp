#include "magnify.h"

#include <algorithm>
#include <cassert>
#include <iostream>
#include <vector>

std::string magnify(const std::string &Input) {
  std::vector<std::pair<std::string, unsigned>> SubstringToCount;
  std::vector<bool> IsBlocked(Input.size(), false);
  // Итерация от самой длинной подстроки к кратчайшему.
  for (size_t SubstringLength = Input.size() / 2; SubstringLength >= 2;
       --SubstringLength) {
    for (size_t SubstringBegin = 0;
         SubstringBegin < Input.size() - SubstringLength; SubstringBegin++) {
      for (unsigned i = 0; i < SubstringLength; ++i) {
        if (IsBlocked[SubstringBegin + i]) {
          continue;
        }
      }
      std::string Substring = Input.substr(SubstringBegin, SubstringLength);
      // Проверяет, есть ли еще одно появление текущей подстроки.
      if (Input.find(Substring, SubstringBegin + SubstringLength) !=
          std::string::npos) {
        unsigned Occurances = 0;
        size_t LeftEnd = 0;
        std::vector<unsigned> BlockList;
        while (Input.find(Substring, LeftEnd) != std::string::npos) {
          size_t Begin = Input.find(Substring, LeftEnd);
          bool BlockedSubstring = false;
          for (size_t i = 0; i < SubstringLength; ++i) {
            if (IsBlocked[Begin + i]) {
              BlockedSubstring = true;
              break;
            }
          }
          if (!BlockedSubstring) {
            Occurances++;
            for (size_t i = 0; i < SubstringLength; ++i) {
              BlockList.push_back(Begin + i);
            }
          }
          LeftEnd = Begin + SubstringLength;
        }
        if (Occurances > 1) {
          for (const auto &BlockedItem : BlockList) {
            IsBlocked[BlockedItem] = true;
          }
          std::sort(begin(Substring), end(Substring));
          SubstringToCount.emplace_back(Substring, Occurances);
        }
      }
    }
  }

  std::string Result;
  for (const auto &SubstringAndCount : SubstringToCount) {
    Result += std::to_string(SubstringAndCount.second) + "(" +
              SubstringAndCount.first + ")";
  }
  for (size_t i = 0; i < Input.size(); ++i) {
    if (!IsBlocked[i]) {
      Result += Input[i];
    }
  }

  return Result;
}

