#ifndef PROBLEM_BASE_HPP
#define PROBLEM_BASE_HPP

#include <indicator.hpp>
#include <utility>
#include <vector>

using Solution = std::vector<int64_t>;
// Overload the << operator for std::vector<int64_t>
std::ostream& operator<<(std::ostream& os, const Solution& sol) {
  os << "Solution: (";
  for (size_t i = 0; i < sol.size(); ++i) {
    os << sol[i];
    if (i < sol.size() - 1) {
      os << ", ";
    }
  }
  os << ")";
  return os;
}

using Candidate = std::pair<int32_t, int64_t>;
// Overload the << operator for std::pair<int32_t, int64_t>
std::ostream& operator<<(std::ostream& os, const Candidate& cand) {
  os << "Candidate: (" << cand.first << ", " << cand.second << ")";
  return os;
}

template <typename Solution, typename Candidate>
class ProblemBase {
 public:
  virtual ~ProblemBase() = default;

  virtual void reset() = 0;
  virtual std::vector<Solution> getNondominatedSet() const = 0;
  virtual std::vector<int64_t> getReferencePoint() const = 0;
  virtual std::vector<Candidate> generateCandidates(const Solution& current_solution,
                                                    const std::vector<Solution>& solution_set,
                                                    const HypervolumeIndicator<int64_t, Solution>& hvc_space) const = 0;
  virtual Solution chooseCandidate(const std::vector<Candidate>& selected_candidates) = 0;
  virtual std::tuple<Solution, std::vector<Solution>> improveSolution(const Solution& current_solution, const std::vector<Solution>& solution_set) const = 0;
  virtual Solution emptySolution() const = 0;
  virtual bool isFeasible(const Solution& solution) const = 0;

  std::vector<Candidate> selectCandidates(const std::vector<Candidate>& candidates, const double& alpha) const {
    int64_t diff = static_cast<int64_t>((candidates[0].second - candidates[candidates.size() - 1].second) * (alpha));
    int64_t threshold = static_cast<int64_t>(candidates[0].second - diff);
    std::vector<Candidate> selected_candidates;
    for (int i = 0; i < (int)candidates.size(); i++) {
      if (candidates[i].second >= threshold) {
        selected_candidates.push_back(candidates[i]);
      } else {
        break;
      }
    }
    return selected_candidates;
  }

  Solution compute_nadir_nset(const std::vector<Solution>& nondominated_set, const bool& is_maximization) const {
    if (nondominated_set.empty()) {
      return this->getReferencePoint();
    }
    Solution nadir = this->emptySolution();
    if (is_maximization) {
      std::fill(nadir.begin(), nadir.end(), std::numeric_limits<int64_t>::max());
    } else {
      std::fill(nadir.begin(), nadir.end(), std::numeric_limits<int64_t>::min());
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
};

#endif  // PROBLEM_BASE_HPP
