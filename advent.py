from __future__ import print_function
from ortools.linear_solver import pywraplp


def main():
    solver = pywraplp.Solver('Advent', pywraplp.Solver.GLOP_LINEAR_PROGRAMMING)
    w = 4
    h = 3
    positions = [complex(x, y) for x in range(w) for y in range(h)]
    n = len(positions)
    assignment_indicators = [[
        solver.BoolVar('{}|->{}'.format(i, positions[p])) for p in range(n)
    ] for i in range(n)]
    for i in range(n):
        c = solver.Constraint(1, 1)
        for p in range(n):
            c.SetCoefficient(assignment_indicators[i][p], 1)
    for p in range(n):
        c = solver.Constraint(1, 1)
        for i in range(n):
            c.SetCoefficient(assignment_indicators[i][p], 1)
    joint_indicators = [[[[
        (solver.BoolVar('{}&{}'.format(assignment_indicators[i1][p1].name(),
                                       assignment_indicators[i2][p2].name()))
         if i1 < i2 and p1 != p2 else None) for p2 in range(n)
    ] for i2 in range(n)] for p1 in range(n)] for i1 in range(n)]
    for i1 in range(n):
        for i2 in range(n):
            if i1 < i2:
                for p1 in range(n):
                    for p2 in range(n):
                        if p1 != p2:
                            joint_indicators[i2][p2][i1][
                                p1] = joint_indicators[i1][p1][i2][p2]
    for i1 in range(n):
        for p1 in range(n):
            for i2 in range(n):
                if i1 != i2:
                    c = solver.Constraint(0, 0)
                    c.SetCoefficient(assignment_indicators[i1][p1], -1)
                    for p2 in range(n):
                        if p1 != p2:
                            c.SetCoefficient(joint_indicators[i1][p1][i2][p2],
                                             1)
            for p2 in range(n):
                if p1 != p2:
                    c = solver.Constraint(0, 0)
                    c.SetCoefficient(assignment_indicators[i1][p1], -1)
                    for i2 in range(n):
                        if i1 != i2:
                            c.SetCoefficient(joint_indicators[i1][p1][i2][p2],
                                             1)
    objective = solver.Objective()
    max_distance = max(
        abs(positions[p1] - positions[p2]) for p1 in range(n)
        for p2 in range(n))
    for i1 in range(n):
        for i2 in range(n):
            if i1 != i2:
                for p1 in range(n):
                    for p2 in range(n):
                        objective.SetCoefficient(
                            joint_indicators[i1][p1][i2][p2],
                            (max_distance - abs(positions[p1] - positions[p2]))
                            / abs(i1 - i2))
    objective.SetMinimization()
    solver.EnableOutput()
    result_status = solver.Solve()
    print(result_status)
    assert result_status == pywraplp.Solver.OPTIMAL
    assert solver.VerifySolution(1e-7, True)
    print('Number of variables =', solver.NumVariables())
    print('Number of constraints =', solver.NumConstraints())
    print('Optimal objective value =', solver.Objective().Value())
    solution = [[None for x in range(w)] for y in range(h)]
    for i in range(n):
        for p in range(n):
            v = assignment_indicators[i][p]
            if round(v.solution_value()):
                pp = positions[p]
                solution[round(pp.imag)][round(pp.real)] = i + 1
    for line in solution:
        print(*line)


if __name__ == '__main__':
    main()
