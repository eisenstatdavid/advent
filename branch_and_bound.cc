#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <limits>
#include <numeric>
#include <queue>
#include <vector>

namespace {

constexpr int kHeight = 4;
constexpr int kWidth = 4;
constexpr int kN = kHeight * kWidth;
constexpr int kPrecision = 40;
static_assert(kN * (kN - 1) * (kHeight + kWidth - 2) <=
                  (std::numeric_limits<long long>::max() >> kPrecision),
              "must reduce precision");

long long g_costs[kHeight][kWidth][kN];

void InitializeCosts() {
  double max = std::hypot(kHeight - 1, kWidth - 1);
  for (int dy = 0; dy < kHeight; dy++) {
    for (int dx = 0; dx < kWidth; dx++) {
      double base_cost = max - std::hypot(dx, dy);
      for (int dday = 1; dday < kN; dday++) {
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

int SquaredDistance(Place p1, Place p2) {
  return (p1.x - p2.x) * (p1.x - p2.x) + (p1.y - p2.y) * (p1.y - p2.y);
}

struct Assignment {
  Place place;
  int day;
};

long long CostTerm(Assignment a1, Assignment a2) {
  return g_costs[std::abs(a1.place.y - a2.place.y)]
                [std::abs(a1.place.x - a2.place.x)][std::abs(a1.day - a2.day)];
}

using PartialSolution = std::vector<Assignment>;

void Availability(const PartialSolution& sol, std::vector<Place>* places,
                  std::vector<int>* days) {
  bool loc_available[kHeight][kWidth];
  for (int y = 0; y < kHeight; y++) {
    for (int x = 0; x < kWidth; x++) {
      loc_available[y][x] = true;
    }
  }
  bool day_available[kN];
  for (int day = 0; day < kN; day++) {
    day_available[day] = true;
  }
  for (Assignment a : sol) {
    loc_available[a.place.y][a.place.x] = false;
    day_available[a.day] = false;
  }
  for (int y = 0; y < kHeight; y++) {
    for (int x = 0; x < kWidth; x++) {
      if (loc_available[y][x]) {
        places->push_back({y, x});
      }
    }
  }
  for (int day = 0; day < kN; day++) {
    if (day_available[day]) {
      days->push_back(day);
    }
  }
}

long long PartialCost(const PartialSolution& sol) {
  long long cost = 0;
  for (Assignment a1 : sol) {
    for (Assignment a2 : sol) {
      cost += CostTerm(a1, a2);
    }
  }
  return cost;
}

int SquareMatrixSize(const std::vector<std::vector<long long>>& matrix) {
  assert(!matrix.empty());
  assert(matrix.size() <= std::numeric_limits<int>::max());
  int n = matrix.size();
  for (const std::vector<long long>& row : matrix) {
    assert(row.size() == n);
  }
  return n;
}

std::vector<int> FindNegativeCycle(
    const std::vector<std::vector<long long>>& graph) {
  int n = SquareMatrixSize(graph);
  std::vector<long long> distance = graph[0];
  std::vector<int> parent(n, 0);
  int last;
  for (int i = 0; i < n; i++) {
    last = -1;
    for (int v = 0; v < n; v++) {
      for (int w = 0; w < n; w++) {
        long long new_distance = distance[v] + graph[v][w];
        if (new_distance < distance[w]) {
          distance[w] = new_distance;
          parent[w] = v;
          last = w;
        }
      }
    }
    if (last == -1) {
      return {};
    }
  }
  std::vector<int> position(n, -1);
  std::vector<int> cycle;
  int v = last;
  while (position[v] == -1) {
    position[v] = cycle.size();
    cycle.push_back(v);
    v = parent[v];
  }
  cycle.erase(cycle.begin(), cycle.begin() + position[v]);
  std::reverse(cycle.begin(), cycle.end());
  return cycle;
}

long long MinAssignmentCost(const std::vector<std::vector<long long>>& matrix) {
  int n = SquareMatrixSize(matrix);
  std::vector<long long> mates(n);
  std::iota(mates.begin(), mates.end(), 0);
  while (true) {
    std::vector<std::vector<long long>> graph(n, std::vector<long long>(n));
    for (int row2 = 0; row2 < n; row2++) {
      int col = mates[row2];
      long long back_entry = matrix[row2][col];
      for (int row1 = 0; row1 < n; row1++) {
        graph[row1][row2] = matrix[row1][col] - back_entry;
      }
    }
    std::vector<int> cycle = FindNegativeCycle(graph);
    if (cycle.empty()) {
      break;
    }
    for (int i = 0; i < cycle.size() - 1; i++) {
      std::swap(mates[cycle[i]], mates[cycle[i + 1]]);
    }
  }
  long long objective = 0;
  for (int row = 0; row < n; row++) {
    objective += matrix[row][mates[row]];
  }
  return objective;
}

long long CostLowerBound(const PartialSolution& sol) {
  std::vector<Place> places;
  std::vector<int> days;
  Availability(sol, &places, &days);
  if (places.empty()) {
    return PartialCost(sol);
  }
  std::vector<std::vector<long long>> matrix(
      places.size(), std::vector<long long>(days.size()));
  for (int i = 0; i < places.size(); i++) {
    std::vector<Place> other_places = places;
    other_places.erase(other_places.begin() + i);
    Place p0 = places[i];
    std::stable_sort(other_places.begin(), other_places.end(),
                     [p0](const Place& p1, const Place& p2) {
                       return SquaredDistance(p0, p1) > SquaredDistance(p0, p2);
                     });
    for (int j = 0; j < days.size(); j++) {
      std::vector<int> other_days = days;
      other_days.erase(other_days.begin() + j);
      int day0 = days[j];
      std::stable_sort(other_days.begin(), other_days.end(),
                       [day0](const int& day1, const int& day2) {
                         return std::abs(day0 - day1) < std::abs(day0 - day2);
                       });
      long long entry = 0;
      Assignment a1 = {p0, day0};
      for (Assignment a2 : sol) {
        entry += 2 * CostTerm(a1, a2);
      }
      assert(other_places.size() == other_days.size());
      for (int k = 0; k < other_places.size() && k < other_days.size(); k++) {
        entry += CostTerm(a1, {other_places[k], other_days[k]});
      }
      matrix[i][j] = entry;
    }
  }
  return PartialCost(sol) + MinAssignmentCost(matrix);
}

struct BoundedPartialSolution {
  long long cost_lower_bound;
  PartialSolution solution;
};

struct BoundGreater {
  bool operator()(const BoundedPartialSolution& bps1,
                  const BoundedPartialSolution& bps2) {
    return bps1.cost_lower_bound > bps2.cost_lower_bound;
  }
};

std::vector<BoundedPartialSolution> Children(const PartialSolution& sol) {
  std::vector<Place> places;
  std::vector<int> days;
  Availability(sol, &places, &days);
  if (places.empty()) {
    return {};
  }
  long long best_centrality = std::numeric_limits<long long>::max();
  Place place;
  for (Place p1 : places) {
    long long centrality = 0;
    for (Place p2 : places) {
      centrality += g_costs[std::abs(p1.y - p2.y)][std::abs(p1.x - p2.x)][1];
    }
    if (centrality <= best_centrality) {
      best_centrality = centrality;
      place = p1;
    }
  }
  std::vector<BoundedPartialSolution> children;
  for (int day : days) {
    PartialSolution child = sol;
    child.push_back({place, day});
    children.push_back({CostLowerBound(child), child});
  }
  return children;
}

}  // namespace

int main() {
  InitializeCosts();
  std::priority_queue<BoundedPartialSolution,
                      std::vector<BoundedPartialSolution>, BoundGreater>
      queue;
  queue.push({CostLowerBound({}), {}});
  for (int i = 0; true; i++) {
    const BoundedPartialSolution& bps = queue.top();
    if (i % 1000 == 0) {
      std::cerr << std::setw(8) << i
                << ": cost_lower_bound=" << RealCost(bps.cost_lower_bound)
                << '\n';
    }
    std::vector<BoundedPartialSolution> children = Children(bps.solution);
    if (children.empty()) {
      std::cerr << "objective=" << RealCost(bps.cost_lower_bound) << '\n';
      for (Assignment a : bps.solution) {
        std::cerr << a.place.y << ',' << a.place.x << ':' << a.day + 1 << '\n';
      }
      break;
    }
    queue.pop();
    for (BoundedPartialSolution& child : children) {
      queue.push(std::move(child));
    }
  }
}
