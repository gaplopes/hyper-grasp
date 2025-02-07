#ifndef MOKP_HPP
#define MOKP_HPP

#include <algorithm>
#include <cstdint>
#include <dominance.hpp>
#include <hypervolume_indicator.hpp>
#include <iostream>
#include <problem_base.hpp>
#include <utility>
#include <vector>

struct Item {
 public:
  int32_t idx;                  // Index of the item
  int64_t weight;               // Weight of the item
  std::vector<int64_t> values;  // Values of the item for each objective

  Item(int idx, int64_t weight, std::vector<int64_t> values)
      : idx(idx), weight(weight), values(values) {}

  std::string to_string() const {
    std::string str = "Item(" + std::to_string(idx) + ", " + std::to_string(weight) + ", [";
    for (int i = 0; i < (int)values.size(); i++) {
      str += std::to_string(values[i]);
      if (i < (int)values.size() - 1) {
        str += ", ";
      }
    }
    str += "])\n";
    return str;
  }
};

struct UpperBound {
 public:
  UpperBound(const std::vector<Item>& items, int32_t M, int64_t W) : items(items), N(items.size()), M(M), W(W) {
    // Pre-process the items and sort them by the ratio of values[i]/weight
    this->sorted_items.resize(M);
    for (int i = 0; i < M; i++) {
      this->sorted_items[i] = items;
      // Sort the items by the ratio of values[i]/weight
      std::sort(sorted_items[i].begin(), sorted_items[i].end(), [i](const Item& a, const Item& b) {
        return (a.values[i] / (double)a.weight) > (b.values[i] / (double)b.weight);
      });
    }
    // Compute the upper bound positions
    this->ub_positions.resize(M, std::vector<int32_t>(N, 0));
    for (int i = 0; i < M; i++) {
      for (int j = 0; j < N; j++) {
        ub_positions[i][sorted_items[i][j].idx] = j;
      }
    }
    // Compute the upper bound auxiliary vectors
    this->ub_full.resize(M, 0);
    this->ub_partial.resize(M, 0);
    this->ub_q.resize(M, 0);
    this->ub_m.resize(M, 0);
    for (int i = 0; i < M; i++) {
      int64_t current_weight = W;
      int64_t break_point = 0;
      for (int j = 0; j < this->N; j++) {
        if (sorted_items[i][j].weight <= current_weight) {
          ub_full[i] += sorted_items[i][j].values[i];
          current_weight -= sorted_items[i][j].weight;
          break_point += 1;
        } else {
          ub_partial[i] = (current_weight * sorted_items[i][j].values[i]) / sorted_items[i][j].weight;
          break;
        }
      }
      ub_q[i] = current_weight;
      ub_m[i] = break_point;
    }
    // Compute the upper bound
    this->ub.resize(M, 0);
    for (int i = 0; i < M; i++) {
      ub[i] = ub_full[i] + ub_partial[i];
    }
    // Save the default upper bounds
    this->default_ub_full = ub_full;
    this->default_ub_partial = ub_partial;
    this->default_ub_q = ub_q;
    this->default_ub_m = ub_m;
    this->default_ub = ub;
  }

  auto reset() {
    this->ub_full = this->default_ub_full;
    this->ub_partial = this->default_ub_partial;
    this->ub_q = this->default_ub_q;
    this->ub_m = this->default_ub_m;
    this->ub = this->default_ub;
  }

  auto update_add_item(const std::vector<bool>& used_items, const int32_t item_idx) {
    std::vector<int64_t> ub_aux = this->ub;
    for (int i = 0; i < this->M; i++) {
      if (this->ub_positions[i][item_idx] < this->ub_m[i]) continue;
      this->ub_q[i] -= this->items[item_idx].weight;
      this->ub_full[i] += this->items[item_idx].values[i];
      int32_t idx = this->sorted_items[i][this->ub_m[i]].idx;
      while (this->ub_q[i] < 0) {
        this->ub_m[i] -= 1;
        idx = this->sorted_items[i][this->ub_m[i]].idx;
        if (used_items[idx] == false) {
          this->ub_q[i] += this->items[idx].weight;
          this->ub_full[i] -= this->items[idx].values[i];
        }
      }
      this->ub_partial[i] = (this->ub_q[i] * this->items[idx].values[i]) / this->items[idx].weight;
      ub_aux[i] = this->ub_full[i] + this->ub_partial[i];
    }
    this->ub = ub_aux;
    return ub_aux;
  }

  auto simulate_add_item(const std::vector<bool>& used_items, const int32_t item_idx) const {
    std::vector<int64_t> ub_aux = this->ub;
    for (int i = 0; i < this->M; i++) {
      if (this->ub_positions[i][item_idx] < this->ub_m[i]) continue;
      int64_t ub_q = this->ub_q[i];
      int64_t ub_full = this->ub_full[i];
      ub_q -= this->items[item_idx].weight;
      ub_full += this->items[item_idx].values[i];
      int32_t ub_m = this->ub_m[i];
      int32_t idx = this->sorted_items[i][ub_m].idx;
      while (ub_q < 0) {
        ub_m -= 1;
        idx = this->sorted_items[i][ub_m].idx;
        if (used_items[idx] == false) {
          ub_q += this->items[idx].weight;
          ub_full -= this->items[idx].values[i];
        }
      }
      int64_t ub_partial = (ub_q * this->items[idx].values[i]) / this->items[idx].weight;
      ub_aux[i] = ub_full + ub_partial;
    }
    return ub_aux;
  }

  std::string to_string() const {
    std::string str = "UpperBound (";
    for (int i = 0; i < (int)ub.size(); i++) {
      str += std::to_string(ub[i]);
      if (i < (int)ub.size() - 1) {
        str += ", ";
      }
    }
    str += ")\n";
    return str;
  }

 private:
  const std::vector<Item> items;                   // Original items
  const int32_t N;                                 // Number of items
  const int32_t M;                                 // Number of objectives
  const int64_t W;                                 // Maximum Knapsack weight
  std::vector<std::vector<Item>> sorted_items;     // Items sorted by the ratio of values[i]/weight
  std::vector<std::vector<int32_t>> ub_positions;  // Positions of the items in the sorted_items vector
  std::vector<int64_t> ub_full;                    // Upper bound for the full knapsack
  std::vector<int64_t> ub_partial;                 // Upper bound for the partial knapsack
  std::vector<int64_t> ub_q;                       // Remaining weight for the partial knapsack
  std::vector<int64_t> ub_m;                       // Break point for the partial knapsack
  std::vector<int64_t> ub;                         // Upper bound for the knapsack

  std::vector<int64_t> default_ub_full;
  std::vector<int64_t> default_ub_partial;
  std::vector<int64_t> default_ub_q;
  std::vector<int64_t> default_ub_m;
  std::vector<int64_t> default_ub;
};

template <typename Solution, typename Candidate>
class MOKP : public ProblemBase<Solution, Candidate> {
  MOKP(int32_t N, int32_t M, int64_t W,
       std::vector<int64_t> weights,
       std::vector<std::vector<int64_t>> values,
       std::vector<Item> items,
       std::vector<Solution> nondominated_set)
      : N(N),
        M(M),
        W(W),
        weights(weights),
        values(values),
        items(items),
        nondominated_set(nondominated_set),
        current_upper_bound(items, M, W),
        current_solution(M, 0),
        current_used_items(N, false),
        current_weight(W) {
    // Initialize the reference point
    this->reference_point = std::vector<int64_t>(M, 0);
  }

 public:
  template <typename IStream>
  static auto from_stream(IStream&& is) -> MOKP {
    // Read from file
    int32_t N, M;
    int64_t W;
    is >> N >> M;
    is >> W;
    std::vector<int64_t> weights(N);
    std::vector<std::vector<int64_t>> values(N, std::vector<int64_t>(M));
    for (int i = 0; i < N; i++) {
      is >> weights[i];
      for (int j = 0; j < M; j++) {
        is >> values[i][j];
      }
    }
    // Pre-process the items and sort them by the ratio of values[i]/weight
    std::vector<Item> items = std::vector<Item>();
    for (int i = 0; i < N; i++) {
      items.push_back(Item(i, weights[i], values[i]));
    }
    // Read the nondominated set
    std::vector<Solution> nondominated_set;
    if (!is.eof()) {
      int32_t n_nondominated_set;
      is >> n_nondominated_set;
      nondominated_set = std::vector<Solution>(n_nondominated_set, Solution(M, 0));
      for (int i = 0; i < n_nondominated_set; i++) {
        for (int j = 0; j < M; j++) {
          is >> nondominated_set[i][j];
        }
      }
    }
    return MOKP(N, M, W, weights, values, items, nondominated_set);
  }

  void reset() override {
    this->current_upper_bound.reset();
    this->current_solution = std::vector<int64_t>(M, 0);
    this->current_used_items = std::vector<bool>(N, false);
    this->current_weight = W;
  }

  [[nodiscard]] std::vector<Solution> getNondominatedSet() const {
    return this->nondominated_set;
  }

  [[nodiscard]] std::vector<int64_t> getReferencePoint() const override {
    return this->reference_point;
  }

  [[nodiscard]] std::vector<Candidate> generateCandidates(const Solution& current_solution,
                                                          const HypervolumeIndicator<int64_t, Solution>& hv_space) const override {
    std::vector<Candidate> candidate_items;
    for (int i = 0; i < N; i++) {
      if (this->current_used_items[i]) continue;
      if (this->items[i].weight > this->current_weight) continue;
      auto aux_upper_bound = this->current_upper_bound.simulate_add_item(this->current_used_items, i);
      int64_t hv = hv_space.contribution(aux_upper_bound);
      if (hv > 0) {
        candidate_items.push_back({i, hv});
      }
    }
    std::sort(candidate_items.begin(), candidate_items.end(), [](Candidate a, Candidate b) {
      return a.second > b.second;
    });
    return candidate_items;
  }

  [[nodiscard]] Solution chooseCandidate(const std::vector<Candidate>& selected_candidates) override {
    int32_t random_item = std::rand() % selected_candidates.size();
    int32_t selected_item = selected_candidates[random_item].first;
    for (int j = 0; j < M; j++) {
      this->current_solution[j] += this->items[selected_item].values[j];
    }
    this->current_weight -= this->items[selected_item].weight;
    this->current_used_items[selected_item] = true;
    this->current_upper_bound.update_add_item(this->current_used_items, selected_item);
    return this->current_solution;
  }

  [[nodiscard]] Solution emptySolution() const override {
    return Solution(M, 0);
  }

  [[nodiscard]] bool isFeasible(const Solution& solution) const override {
    return this->current_weight >= 0;
  }

  [[nodiscard]] std::tuple<Solution, std::vector<Solution>> improveSolution(const Solution& current_solution,
                                                                            const std::vector<Solution>& solution_set) const override {
    // Implement the logic to improve a solution for MOKP
    std::vector<int32_t> used_items;
    std::vector<int32_t> unused_items;
    used_items.reserve(N);
    unused_items.reserve(N);
    int64_t weight = 0;

    for (int i = 0; i < N; ++i) {
      if (current_used_items[i]) {  // Directly check the value
        used_items.push_back(i);
        weight += items[i].weight;
      } else {
        unused_items.push_back(i);
      }
    }

    // First Improvement Local Search
    std::vector<Solution> other_solutions;
    Solution best_solution = current_solution;

    const int32_t n_used = used_items.size();
    const int32_t n_unused = unused_items.size();
    const int32_t n_total = n_used * n_unused;
    std::vector<int32_t> swap_indices(n_total);
    std::iota(swap_indices.begin(), swap_indices.end(), 0);

    while (true) {
      std::random_shuffle(used_items.begin(), used_items.end());
      std::random_shuffle(unused_items.begin(), unused_items.end());
      std::random_shuffle(swap_indices.begin(), swap_indices.end());

      HypervolumeIndicator<int64_t, Solution> hv_space(best_solution, true);
      int64_t best_hv = 0;

      for (auto i : swap_indices) {
        int32_t idx_used = i / n_unused;
        int32_t idx_unused = i % n_unused;
        int32_t used_item = used_items[idx_used];
        int32_t unused_item = unused_items[idx_unused];

        if (weight - items[used_item].weight + items[unused_item].weight > this->W) {
          continue;
        }

        Solution candidate_solution = best_solution;
        for (int j = 0; j < M; j++) {
          candidate_solution[j] -= items[used_item].values[j];
          candidate_solution[j] += items[unused_item].values[j];
        }

        if (is_non_dominated(candidate_solution, solution_set, true) &&
            is_non_dominated(candidate_solution, other_solutions, true)) {
          remove_weakly_dominated(other_solutions, candidate_solution, true);
          other_solutions.push_back(candidate_solution);
        }

        const int64_t hv = hv_space.contribution(candidate_solution);
        if (weakly_dominates(candidate_solution, best_solution, true) || hv > best_hv) {
          best_solution = candidate_solution;
          best_hv = hv;
          weight = weight - items[used_item].weight + items[unused_item].weight;
          std::swap(used_items[idx_used], unused_items[idx_unused]);

          // Sort the unused items by weight
          std::vector<int32_t> sorted_unused_items = unused_items;
          std::sort(unused_items.begin(), unused_items.end(), [&](const int32_t a, const int32_t b) {
            return items[a].weight < items[b].weight;
          });
          // Try to improve the solution by adding more items
          for (int j = 0; j < n_unused; j++) {
            const int32_t unused_item = sorted_unused_items[j];
            if (weight + items[unused_item].weight <= this->W) {
              std::vector<int64_t> extended_solution = best_solution;
              for (int k = 0; k < M; k++) {
                extended_solution[k] += items[unused_item].values[k];
              }
              const int64_t extended_hv = hv_space.contribution(extended_solution);
              if (extended_hv > best_hv) {
                best_solution = extended_solution;
                best_hv = extended_hv;
                weight += items[unused_item].weight;
              }
            }
          }
          break;
        }
      }
      if (best_hv == 0) {
        break;
      }
    }
    // Remove weakly dominated solutions
    remove_weakly_dominated(other_solutions, best_solution, true);
    // Return the best solution and the other solutions
    return {best_solution, other_solutions};
  }

 private:
  const int32_t N; // Number of items
  const int32_t M; // Number of objectives
  const int64_t W; // Maximum Knapsack weight
  const std::vector<int64_t> weights; // Weights of the items
  const std::vector<std::vector<int64_t>> values; // Values of the items
  const std::vector<Item> items; // Items
  const std::vector<Solution> nondominated_set; // Nondominated set
  Solution reference_point;
  UpperBound current_upper_bound;
  Solution current_solution;
  std::vector<bool> current_used_items;
  int64_t current_weight;
};

#endif  // MOKP_HPP
