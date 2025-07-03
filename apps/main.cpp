#include <CLI/App.hpp>
#include <CLI/Config.hpp>
#include <CLI/Formatter.hpp>
#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <hyper_grasp.hpp>
#include <iostream>
#include <mokp.hpp>
#include <momst.hpp>
#include <statistics.hpp>
#include <stopping_criteria.hpp>

void parseCommandLineArguments(CLI::App& app, bool& local_search, bool& is_maximization, double& alpha,
                               std::string& criteria, int& criteria_limit, std::string& input_file, std::string& moco_problem) {
  app.add_option("--local-search", local_search, "Enable local search (true/false)")
      ->default_val(true)
      ->check(CLI::IsMember({true, false}));  // Ensure the value is either true or false

  app.add_option("--maximization", is_maximization, "Set problem type (bool): maximization (true) or minimization (false)")
      ->required()
      ->default_val(false)
      ->check(CLI::IsMember({true, false}));  // Ensure the value is either true or false

  app.add_option("--alpha", alpha, "Set alpha value (double)")
      ->required()
      ->default_val(0.05)
      ->check(CLI::Range(0.0, 1.0));  // Ensure alpha is between 0.0 and 1.0

  app.add_option("--criteria", criteria, "Set criteria (string)")
      ->default_val("time")
      ->check(CLI::IsMember({"time", "iterations"}));  // Ensure criteria is either time or iterations

  app.add_option("--criteria_limit", criteria_limit, "Set criteria limit (int): time in seconds or iterations")
      ->default_val(100)
      ->check(CLI::PositiveNumber);  // Ensure criteria_limit is a positive number

  app.add_option("--input-file", input_file, "Input file with the problem instance")
      ->required()
      ->check(CLI::ExistingFile);  // Ensure the file exists

  app.add_option("--moco-problem", moco_problem, "MOCO problem (string): 'mokp' or 'momst'")
      ->default_val("mokp")
      ->check(CLI::IsMember({"mokp", "momst"}));  // Ensure the problem is either mokp or momst
}

std::unique_ptr<StoppingCriteria> createStoppingCriteria(const std::string& criteria, int criteria_limit) {
  return std::unique_ptr<StoppingCriteria>(StoppingCriteria::create(criteria, criteria_limit));
}

int main(int argc, char* argv[]) {
  CLI::App app{"Hyper-GRASP for Multiobjective Combinatorial Optimization"};

  bool local_search = false;
  bool is_maximization = false;
  double alpha = 0.0;
  std::string criteria;
  int criteria_limit = 0;
  std::string input_file;
  std::string moco_problem;

  // Parse the command line arguments
  parseCommandLineArguments(app, local_search, is_maximization, alpha, criteria, criteria_limit, input_file, moco_problem);
  CLI11_PARSE(app, argc, argv);

  // Output the parsed values
  std::cerr << "Command line arguments:\n";
  std::cerr << " - is_maximization: " << std::boolalpha << is_maximization << "\n";
  std::cerr << " - local_search: " << std::boolalpha << local_search << "\n";
  std::cerr << " - alpha: " << alpha << "\n";
  std::cerr << " - criteria: " << criteria << "\n";
  std::cerr << " - criteria_limit: " << criteria_limit << "\n";
  std::cerr << " - input_file: " << input_file << "\n";
  std::cerr << " - moco_problem: " << moco_problem << "\n";

  // Create the stopping criteria
  auto stopping_criteria = createStoppingCriteria(criteria, criteria_limit);

  // Solve the problem
  if (moco_problem == "mokp") {
    MOKP<Solution, Candidate> problem = MOKP<Solution, Candidate>::from_stream(std::ifstream(input_file));
    HyperGRASP<MOKP<Solution, Candidate>, Solution, Candidate> hyper_grasp(problem, *stopping_criteria, alpha, local_search, is_maximization);
    Statistics<MOKP<Solution, Candidate>, Solution> statistics = hyper_grasp.solve();
    std::cout << "Statistics:\n";
    std::cout << statistics.to_string() << std::endl;
    statistics.to_debug_file();
  } else if (moco_problem == "momst") {
    MOMST<Solution, Candidate> problem = MOMST<Solution, Candidate>::from_stream(std::ifstream(input_file));
    HyperGRASP<MOMST<Solution, Candidate>, Solution, Candidate> hyper_grasp(problem, *stopping_criteria, alpha, local_search, is_maximization);
    Statistics<MOMST<Solution, Candidate>, Solution> statistics = hyper_grasp.solve();
    std::cout << "Statistics:\n";
    std::cout << statistics.to_string() << std::endl;
    statistics.to_debug_file();
  } else {
    std::cerr << "Invalid problem: " << moco_problem << "\n";
    return 1;
  }

  return 0;
}

// Example usage:
// ./hyper-grasp --maximization=true --local-search=true --alpha=0.05 --criteria=iterations 
//               --criteria_limit=100 --moco-problem=mokp --input-file=../instances/mokp/random/2D/100_1.in
