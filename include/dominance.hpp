#ifndef DOMINANCE_HPP
#define DOMINANCE_HPP

#include <algorithm>
#include <iostream>
#include <vector>

template <typename T>
std::vector<T> negative_vector(const std::vector<T>& v) {
  std::vector<T> res(v.size());
  std::transform(v.begin(), v.end(), res.begin(), std::negate<T>());
  return res;
}

template <typename T>
bool weakly_dominates(const std::vector<T>& v1, const std::vector<T>& v2) {
  for (size_t i = 0; i < v1.size(); ++i) {
    if (v1[i] < v2[i]) {
      return false;
    }
  }
  return true;
}

template <typename T>
bool strictly_dominates(const std::vector<T>& v1, const std::vector<T>& v2) {
  for (size_t i = 0; i < v1.size(); ++i) {
    if (v1[i] <= v2[i]) {
      return false;
    }
  }
  return true;
}

template <typename T>
bool weakly_dominates_min(const std::vector<T>& v1, const std::vector<T>& v2) {
  return weakly_dominates(negative_vector(v1), negative_vector(v2));
}

template <typename T>
bool strictly_dominates_min(const std::vector<T>& v1, const std::vector<T>& v2) {
  return strictly_dominates(negative_vector(v1), negative_vector(v2));
}

template <typename T>
bool weakly_dominates(const std::vector<T>& v1, const std::vector<T>& v2, const bool& is_maximization) {
  return is_maximization ? weakly_dominates(v1, v2) : weakly_dominates_min(v1, v2);
}

template <typename T>
bool strictly_dominates(const std::vector<T>& v1, const std::vector<T>& v2, const bool& is_maximization) {
  return is_maximization ? strictly_dominates(v1, v2) : strictly_dominates_min(v1, v2);
}

template <typename T>
bool is_non_dominated(const std::vector<T>& sol, const std::vector<std::vector<T>>& solutions, const bool& is_maximization) {
  return std::none_of(solutions.begin(), solutions.end(), [&](const std::vector<T>& s) {
    return weakly_dominates(s, sol, is_maximization);
  });
}

template <typename T>
void remove_weakly_dominated(std::vector<std::vector<T>>& solutions, const std::vector<T>& sol, const bool& is_maximization) {
  solutions.erase(std::remove_if(solutions.begin(), solutions.end(), [&](const std::vector<T>& s) {
                    return weakly_dominates(sol, s, is_maximization);
                  }),
                  solutions.end());
}

template <typename T>
void remove_weakly_dominated(std::vector<std::vector<T>>& solutions, const std::vector<std::vector<T>>& sols, const bool& is_maximization) {
  for (const auto& sol : sols) {
    remove_weakly_dominated(solutions, sol, is_maximization);
  }
}

template <typename Solution>
bool valid_solutions(const std::vector<Solution>& solutions, const std::vector<Solution>& n_set, const bool& is_maximization) {
  for (size_t i = 0; i < solutions.size(); ++i) {
    for (size_t j = 0; j < solutions.size(); ++j) {
      if (i != j && weakly_dominates(solutions[j], solutions[i], is_maximization)) {
        std::cout << "Solution " << i << " is dominated by solution " << j << std::endl;
        std::cout << "Solution " << i << ": ";
        for (const auto& val : solutions[i]) {
          std::cout << val << " ";
        }
        std::cout << std::endl;
        std::cout << "Solution " << j << ": ";
        for (const auto& val : solutions[j]) {
          std::cout << val << " ";
        }
        std::cout << std::endl;
        return false;
      }
    }
  }

  for (size_t i = 0; i < solutions.size(); ++i) {
    for (size_t j = 0; j < n_set.size(); ++j) {
      if (strictly_dominates(solutions[i], n_set[j], is_maximization)) {
        std::cout << "Non-dominated set solution " << j << " is dominated by solution " << i << std::endl;
        std::cout << "Solution " << i << ": ";
        for (const auto& val : solutions[i]) {
          std::cout << val << " ";
        }
        std::cout << std::endl;
        std::cout << "Non-dominated solution " << j << ": ";
        for (const auto& val : n_set[j]) {
          std::cout << val << " ";
        }
        std::cout << std::endl;
        return false;
      }
    }
  }
  return true;
}

#endif  // DOMINANCE_HPP
