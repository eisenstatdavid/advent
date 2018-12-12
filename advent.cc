// BRKGA for jgc's Advent problem
//
// Author: David Eisenstat

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <random>
#include <vector>

namespace {

constexpr int kHeight = 4;
constexpr int kWidth = 6;
constexpr int kNumKeys = kHeight * kWidth;
constexpr double kProbabilityOfEliteKey = 0.7;
constexpr int kNumIndividuals = 1000;
constexpr int kNumEliteIndividuals = 200;
constexpr int kNumMutants = 100;
constexpr int kReportPeriod = 100;

struct Solution {
  double Objective() const;
  void Print() const;

  int day[kHeight][kWidth];
};

double Solution::Objective() const {
  const double kMax = std::hypot(kWidth - 1, kHeight - 1);
  double objective = 0.0;
  for (int y1 = 0; y1 < kHeight; y1++) {
    for (int x1 = 0; x1 < kWidth; x1++) {
      for (int y2 = 0; y2 < kHeight; y2++) {
        for (int x2 = 0; x2 < kWidth; x2++) {
          int day1 = day[y1][x1];
          int day2 = day[y2][x2];
          if (day1 != day2) {
            objective +=
                (kMax - std::hypot(x1 - x2, y1 - y2)) / std::abs(day1 - day2);
          }
        }
      }
    }
  }
  return objective;
}

void Solution::Print() const {
  std::cout << "Objective: " << Objective() << '\n';
  for (int y = 0; y < kHeight; y++) {
    for (int x = 0; x < kWidth; x++) {
      std::cout << std::setw(x == 0 ? 2 : 3) << day[y][x];
    }
    std::cout << '\n';
  }
}

class Individual {
 public:
  template <typename Generator>
  static Individual Mutant(Generator& generator);

  template <typename Generator>
  static Individual Crossover(const Individual& elite,
                              const Individual& non_elite,
                              Generator& generator);

  double objective() const { return objective_; }

  Solution ToSolution() const;

 private:
  Individual() = default;

  void SetObjective() { objective_ = ToSolution().Objective(); }

  double objective_;
  int key_[kNumKeys];
};

// static
template <typename Generator>
Individual Individual::Mutant(Generator& generator) {
  std::uniform_int_distribution<int> random_key;
  Individual individual;
  for (int k = 0; k < kNumKeys; k++) {
    individual.key_[k] = random_key(generator);
  }
  individual.SetObjective();
  return individual;
}

// static
template <typename Generator>
Individual Individual::Crossover(const Individual& elite,
                                 const Individual& non_elite,
                                 Generator& generator) {
  std::bernoulli_distribution biased_coin(kProbabilityOfEliteKey);
  Individual individual;
  for (int k = 0; k < kNumKeys; k++) {
    individual.key_[k] = (biased_coin(generator) ? elite : non_elite).key_[k];
  }
  individual.SetObjective();
  return individual;
}

Solution Individual::ToSolution() const {
  struct KeyedPosition {
    int key;
    int y;
    int x;
  } positions[kNumKeys];
  for (int k = 0; k < kNumKeys; k++) {
    positions[k] = {key_[k], k / kWidth, k % kWidth};
  }
  std::stable_sort(positions, positions + kNumKeys,
                   [](const KeyedPosition& lhs, const KeyedPosition& rhs) {
                     return lhs.key < rhs.key;
                   });
  Solution solution;
  for (int k = 0; k < kNumKeys; k++) {
    const KeyedPosition& p = positions[k];
    solution.day[p.y][p.x] = k + 1;
  }
  return solution;
}

}  // namespace

int main() {
  std::random_device seed_generator;
  std::mt19937 generator(seed_generator());

  // Initialize the population with mutants.
  std::vector<Individual> population;
  population.reserve(kNumIndividuals);
  for (int i = 0; i < kNumIndividuals; i++) {
    population.push_back(Individual::Mutant(generator));
  }

  // Evolve.
  std::uniform_int_distribution<int> elite_index(0, kNumEliteIndividuals - 1);
  std::uniform_int_distribution<int> non_elite_index(kNumEliteIndividuals,
                                                     kNumIndividuals - 1);
  for (int g = 0; true; g++) {
    std::stable_sort(population.begin(), population.end(),
                     [](const Individual& lhs, const Individual& rhs) {
                       return lhs.objective() < rhs.objective();
                     });
    if (g % kReportPeriod == 0) {
      std::cout << "Generation " << g << '\n';
      population.front().ToSolution().Print();
      std::cout << std::endl;
    }
    std::vector<Individual> new_population = population;
    for (int i = kNumEliteIndividuals; i < kNumIndividuals - kNumMutants; i++) {
      new_population[i] = Individual::Crossover(
          population[elite_index(generator)],
          population[non_elite_index(generator)], generator);
    }
    for (int i = kNumIndividuals - kNumMutants; i < kNumIndividuals; i++) {
      new_population[i] = Individual::Mutant(generator);
    }
    population = std::move(new_population);
  }
}
