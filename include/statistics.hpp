#ifndef STATISTICS_HPP
#define STATISTICS_HPP

#include <indicator.hpp>
#include <string>
#include <tuple>
#include <vector>

template <typename Problem, typename Solution>
class Statistics {
 public:
  Problem problem;
  std::vector<Solution> nondominated_set;
  std::vector<Solution> solutions;
  std::vector<std::tuple<double, int32_t, int64_t>> statistics;
  double elapsed_time;
  int64_t seed;
  int iterations;
  int skipped_iterations;
  bool is_maximization;

  Statistics(Problem problem,
             std::vector<Solution> solutions,
             int64_t seed,
             std::vector<std::tuple<double, int32_t, int64_t>> statistics,
             double elapsed_time,
             int iterations,
             int skipped_iterations,
             bool is_maximization) : problem(problem),
                                     solutions(solutions),
                                     seed(seed),
                                     statistics(statistics),
                                     elapsed_time(elapsed_time),
                                     iterations(iterations),
                                     skipped_iterations(skipped_iterations),
                                     is_maximization(is_maximization) {
    solutions_set_hv = calculate_hv(problem.getReferencePoint(), this->solutions);
    n_solutions = this->solutions.size();
    nondominated_set = problem.getNondominatedSet();
    if (!nondominated_set.empty()) {
      n_nondominated_set = nondominated_set.size();
      nondominated_set_hv = calculate_hv(problem.getReferencePoint(), nondominated_set);
      ratio_hv = static_cast<double>(solutions_set_hv) / nondominated_set_hv;
      Solution nadir_nset = compute_nadir_nset(nondominated_set);
      nadir_nset_hv = calculate_hv(nadir_nset, nondominated_set);
      nadir_set_hv = calculate_hv(nadir_nset, this->solutions);
      ratio_nadir_hv = static_cast<double>(nadir_set_hv) / nadir_nset_hv;
      n_matching = matching(nondominated_set, this->solutions);
    }
  }

  std::string to_string() {
    std::string stats_str;
    stats_str += std::to_string(seed) + "\n";
    for (const auto& stat : statistics) {
      stats_str += "(" + std::to_string(std::get<0>(stat)) + "," + std::to_string(std::get<1>(stat)) + "," + std::to_string(std::get<2>(stat)) + ") ";
    }
    stats_str.pop_back();  // Remove the trailing space
    stats_str += "\n";

    stats_str += std::to_string(iterations) + " " + std::to_string(skipped_iterations) + "\n";

    for (const auto& sol : solutions) {
      stats_str += "(";
      for (std::size_t j = 0; j < sol.size(); ++j) {
        stats_str += std::to_string(sol[j]);
        if (j < sol.size() - 1) {
          stats_str += ",";
        }
      }
      stats_str += ") ";
    }
    stats_str.pop_back();  // Remove the trailing space
    stats_str += "\n";

    stats_str += std::to_string(nondominated_set_hv) + " " + std::to_string(solutions_set_hv) + " " + std::to_string(ratio_hv) + " ";
    stats_str += std::to_string(nadir_nset_hv) + " " + std::to_string(nadir_set_hv) + " " + std::to_string(ratio_nadir_hv) + "\n";

    stats_str += std::to_string(n_nondominated_set) + " " + std::to_string(n_solutions) + " ";
    stats_str += std::to_string(n_matching) + " " + std::to_string(elapsed_time);
    return stats_str;
  }

  void to_file(const std::string& filename) {
    std::ofstream file(filename);
    file << to_string();
    file.close();
  }

  void to_debug_file() {
    std::ofstream file("debug.out");
    file << "Non-dominated set\n";
    for (const auto& sol : nondominated_set) {
      for (std::size_t j = 0; j < sol.size(); ++j) {
        file << sol[j];
        if (j != sol.size() - 1) {
          file << " ";
        }
      }
      file << "\n";
    }
    file << "GRASP set\n";
    for (const auto& sol : solutions) {
      for (std::size_t j = 0; j < sol.size(); ++j) {
        file << sol[j];
        if (j != sol.size() - 1) {
          file << " ";
        }
      }
      file << "\n";
    }
    file << "Hypervolume growth\n";
    file << nondominated_set_hv << "\n";
    for (const auto& stat : statistics) {
      file << std::get<1>(stat) << "\n";
    }
    file.close();
  }

 private:
  int n_nondominated_set = 0;
  int n_solutions = 0;
  int64_t nondominated_set_hv = 0;
  int64_t solutions_set_hv;
  double ratio_hv = 0;
  int64_t nadir_nset_hv = 0;
  int64_t nadir_set_hv = 0;
  double ratio_nadir_hv = 0;
  int n_matching = 0;

  int matching(const std::vector<Solution>& nondominated_set, const std::vector<Solution>& solutions) const {
    int matching = 0;
    for (const auto& sol : solutions) {
      if (std::find(nondominated_set.begin(), nondominated_set.end(), sol) != nondominated_set.end()) {
        matching++;
      }
    }
    return matching;
  }

  Solution compute_nadir_nset(const std::vector<Solution>& nondominated_set) const {
    if (nondominated_set.empty()) {
      return problem.getReferencePoint();
    }
    Solution nadir = problem.emptySolution();
    if (is_maximization) {
      for (auto& val : nadir) {
        val = std::numeric_limits<int64_t>::max();
      }
    } else {
      for (auto& val : nadir) {
        val = std::numeric_limits<int64_t>::min();
      }
    }
    for (const auto& sol : nondominated_set) {
      for (std::size_t i = 0; i < sol.size(); ++i) {
        if (is_maximization) {
          nadir[i] = std::min(nadir[i], sol[i]);
        } else {
          nadir[i] = std::max(nadir[i], sol[i]);
        }
      }
    }
    return nadir;
  }

  int64_t calculate_hv(const Solution& ref_point, const std::vector<Solution>& solutions) const {
    return HypervolumeIndicator<int64_t, Solution>(ref_point, is_maximization).set_hvc(solutions);
  }
};

#endif  // STATISTICS_HPP
