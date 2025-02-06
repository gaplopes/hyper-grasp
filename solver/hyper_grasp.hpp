#ifndef HYPER_GRASP_HPP
#define HYPER_GRASP_HPP

#include <cstdlib>
#include <ctime>
#include <tuple>
#include <vector>

#include <problem_base.hpp>
#include <dominance.hpp>
#include <hypervolume_indicator.hpp>
#include <statistics.hpp>
#include <stopping_criteria.hpp>

template <typename Problem, typename Solution, typename Candidate>
class HyperGRASP {
 public:
  HyperGRASP(Problem& problem, StoppingCriteria& stopping_criteria, double alpha = 0.0, bool local_search = false, bool is_maximization = true)
      : problem(problem), stopping_criteria(stopping_criteria), alpha(alpha), local_search(local_search), is_maximization(is_maximization) {}

  [[nodiscard]] Statistics<Problem, Solution> solve() {
    // Set seed with timestamp
    int64_t seed = std::time(0);
    // int64_t seed = 1;
    std::srand(seed);

    // Initialize variables for the algorithm
    std::vector<Solution> solutions;
    Solution ref_point = problem.getReferencePoint();
    HypervolumeIndicator<int64_t, Solution> hvc_space = HypervolumeIndicator<int64_t, Solution>(ref_point, is_maximization);

    // Initialize variables for statistics
    std::vector<std::tuple<int32_t, int64_t>> statistics;
    int64_t iterations = 0, skipped_iterations = 0;
    auto start_time = std::chrono::high_resolution_clock::now();
    statistics.emplace_back(std::make_tuple(0, 0));

    // Start the algorithm
    stopping_criteria.start();
    while (!stopping_criteria.shouldStop()) {
      iterations++;

      problem.reset();
      Solution current_solution = problem.emptySolution();
      std::vector<Candidate> candidates = problem.generateCandidates(current_solution, hvc_space);

      while (!candidates.empty()) {
        std::vector<Candidate> selected_candidates = problem.selectCandidates(candidates, alpha);
        current_solution = problem.chooseCandidate(selected_candidates);
        candidates = problem.generateCandidates(current_solution, hvc_space);
      }

      if (problem.isFeasible(current_solution) &&
          hvc_space.contribution(current_solution) > 0) {
        std::vector<Solution> other_solutions;
        if (local_search) {
          auto improvement_results = problem.improveSolution(current_solution, solutions);
          current_solution = std::get<0>(improvement_results);
          other_solutions = std::get<1>(improvement_results);
        }
        // Remove weakly dominated solutions
        remove_weakly_dominated(solutions, current_solution, is_maximization);
        remove_weakly_dominated(solutions, other_solutions, is_maximization);
        // Add the solutions to the list
        solutions.push_back(current_solution);
        for (int i = 0; i < (int)other_solutions.size(); i++) {
          solutions.push_back(other_solutions[i]);
        }
        // Insert the solutions in the hypervolume space
        hvc_space.insert(current_solution);
        for (int i = 0; i < (int)other_solutions.size(); i++) {
          hvc_space.insert(other_solutions[i]);
        }
      } else {
        skipped_iterations++;
        // Special case (Greedy algorithm)
        if (alpha == 0.0) {
          break;
        }
      }

      stopping_criteria.increment();
      if (stopping_criteria.shouldRetrieve()) {
        statistics.emplace_back(std::make_tuple(solutions.size(), hvc_space.value()));
        stopping_criteria.resetRetrieveCriteria();
      }
    }

    auto now = std::chrono::high_resolution_clock::now();
    auto elapsed_time = std::chrono::duration<double>(now - start_time).count();

    if (!valid_solutions(solutions, problem.getNondominatedSet(), is_maximization)) {
      throw std::runtime_error("Invalid solutions!");
    }

    return Statistics<Problem, Solution>(problem, solutions, seed, statistics, elapsed_time, iterations, skipped_iterations, is_maximization);
  }

 private:
  Problem& problem;
  StoppingCriteria& stopping_criteria;
  double alpha;
  bool local_search;
  bool is_maximization;
};

#endif  // HYPER_GRASP_HPP
