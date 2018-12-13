#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <limits>
#include <numeric>
#include <vector>

namespace {

constexpr int kHeight = 4;
constexpr int kWidth = 3;
constexpr int kDays = kHeight * kWidth;
constexpr int kPrecision = 50;
static_assert(kDays * (kDays - 1) * (kHeight + kWidth - 2) <=
                  (std::numeric_limits<long long>::max() >> kPrecision),
              "must reduce precision");

long long g_costs[kHeight][kWidth][kDays];

void InitializeCosts() {
  const double max = std::hypot(kHeight - 1, kWidth - 1);
  for (int dy = 0; dy < kHeight; dy++) {
    for (int dx = 0; dx < kWidth; dx++) {
      const double base_cost = max - std::hypot(dx, dy);
      for (int dday = 1; dday < kDays; dday++) {
        g_costs[dy][dx][dday] =
            std::llrint(std::ldexp(base_cost / dday, kPrecision));
      }
    }
  }
}

double RealCost(long long cost) { return std::ldexp(cost, -kPrecision); }

struct Place {
  int y;
  int x;
};

struct Assignment {
  Place place;
  int day;
};

long long CostTerm(const Assignment& a1, const Assignment& a2) {
  return g_costs[std::abs(a1.place.y - a2.place.y)]
                [std::abs(a1.place.x - a2.place.x)][std::abs(a1.day - a2.day)];
}

using PartialSolution = std::vector<Assignment>;

long long Cost(const PartialSolution& sol) {
  long long cost = 0;
  for (const Assignment& a1 : sol) {
    for (const Assignment& a2 : sol) {
      cost += CostTerm(a1, a2);
    }
  }
  return cost;
}

long long LowerBound(const PartialSolution& sol) {
  bool loc_available[kHeight][kWidth];
  for (int y = 0; y < kHeight; y++) {
    for (int x = 0; x < kWidth; x++) {
      loc_available[y][x] = true;
    }
  }
  bool day_available[kDays];
  for (int day = 0; day < kDays; day++) {
    day_available[day] = true;
  }
  for (const Assignment& a : sol) {
    loc_available[a.place.y][a.place.x] = false;
    day_available[a.day] = false;
  }
  std::vector<Place> places;
  for (int y = 0; y < kHeight; y++) {
    for (int x = 0; x < kWidth; x++) {
      if (loc_available[y][x]) {
        places.push_back({y, x});
      }
    }
  }
  std::vector<int> days;
  for (int day = 0; day < kDays; day++) {
    if (day_available[day]) {
      days.push_back(day);
    }
  }
  return 0;
}

}  // namespace

int main() {
  InitializeCosts();
  int perm[kDays];
  std::iota(perm, perm + kDays, 0);
  double min_cost = std::numeric_limits<double>::max();
  int best[kDays];
  PartialSolution sol(kDays);
  int g = 0;
  do {
    g++;
    if (g % 1000000 == 0) {
      std::cerr << g << '\n';
    }
    for (int y = 0; y < kHeight; y++) {
      for (int x = 0; x < kWidth; x++) {
        const int i = y * kWidth + x;
        sol[i] = {{y, x}, perm[i]};
      }
    }
    const double cost = RealCost(Cost(sol));
    if (cost < min_cost) {
      min_cost = cost;
      std::copy(perm, perm + kDays, best);
    }
  } while (std::next_permutation(perm, perm + kDays));
  std::cout << "min_cost is " << min_cost << '\n';
  for (int y = 0; y < kHeight; y++) {
    for (int x = 0; x < kWidth; x++) {
      std::cout << std::setw(x == 0 ? 2 : 3) << best[y * kWidth + x] + 1;
    }
    std::cout << '\n';
  }
}
