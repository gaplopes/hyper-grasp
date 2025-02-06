#ifndef STOPPING_CRITERIA_HPP
#define STOPPING_CRITERIA_HPP

#include <chrono>
#include <stdexcept>
#include <string>

class StoppingCriteria {
 public:
  virtual ~StoppingCriteria() = default;
  virtual bool shouldStop() const = 0;
  virtual void start() = 0;
  virtual void increment() = 0;
  virtual double current() const = 0;
  virtual bool shouldRetrieve() const = 0;
  virtual void resetRetrieveCriteria() = 0;

  static StoppingCriteria* create(const std::string& criteria, double limit);
};

class TimeStoppingCriteria : public StoppingCriteria {
 public:
  TimeStoppingCriteria(double time_limit, double retrieve_interval = 5)
      : time_limit(time_limit), retrieve_interval(retrieve_interval) {}

  void start() override {
    start_time = std::chrono::high_resolution_clock::now();
    last_retrieve_time = start_time;
  }

  bool shouldStop() const override {
    auto now = std::chrono::high_resolution_clock::now();
    auto elapsed = std::chrono::duration<double>(now - start_time).count();
    return elapsed >= time_limit;
  }

  void increment() override {
    // Do nothing
  }

  double current() const override {
    auto now = std::chrono::high_resolution_clock::now();
    return std::chrono::duration<double>(now - start_time).count();
  }

  bool shouldRetrieve() const override {
    auto now = std::chrono::high_resolution_clock::now();
    auto elapsed = std::chrono::duration<double>(now - last_retrieve_time).count();
    return elapsed >= retrieve_interval;
  }

  void resetRetrieveCriteria() override {
    last_retrieve_time = std::chrono::high_resolution_clock::now();
  }

 private:
  double time_limit;
  double retrieve_interval;
  std::chrono::time_point<std::chrono::high_resolution_clock> start_time;
  std::chrono::time_point<std::chrono::high_resolution_clock> last_retrieve_time;
};

class IterationStoppingCriteria : public StoppingCriteria {
 public:
  IterationStoppingCriteria(int iteration_limit, double retrieve_interval = 50)
      : iteration_limit(iteration_limit), retrieve_interval(retrieve_interval), current_iteration(0) {}

  void start() override {
    current_iteration = 0;
    last_retrieve_iteration = 0;
  }

  bool shouldStop() const override {
    return current_iteration >= iteration_limit;
  }

  void increment() override {
    ++current_iteration;
  }

  double current() const override {
    return static_cast<double>(current_iteration);
  }

  bool shouldRetrieve() const override {
    return (current_iteration - last_retrieve_iteration) >= retrieve_interval;
  }

  void resetRetrieveCriteria() override {
    last_retrieve_iteration = current_iteration;
  }

 private:
  int iteration_limit;
  double retrieve_interval;
  int current_iteration;
  int last_retrieve_iteration;
};

StoppingCriteria* StoppingCriteria::create(const std::string& criteria, double limit) {
  if (criteria == "time") {
    return new TimeStoppingCriteria(limit);
  } else if (criteria == "iterations") {
    return new IterationStoppingCriteria(static_cast<int>(limit));
  } else {
    throw std::invalid_argument("Invalid criteria: " + criteria);
  }
}

#endif  // STOPPING_CRITERIA_HPP
