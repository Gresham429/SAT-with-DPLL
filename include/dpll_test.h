#include <iostream>
#include <vector>
#include <unordered_set>
#include <unordered_map>
#include <algorithm>

enum class LiteralStatus {
    Unassigned,
    True,
    False
};

struct Clause {
    std::vector<int> literals;
    bool isActive;
};

class DPLLSolver {
public:
    DPLLSolver(const std::vector<Clause>& clauses) : clauses(clauses), assignment(clauses.size(), LiteralStatus::Unassigned) {
        // 初始化literalsToClauses
        for (int i = 0; i < clauses.size(); ++i) {
            for (int literal : clauses[i].literals) {
                literalsToClauses[literal].insert(i);
            }
        }
    }

    bool solve() {
        return solveRecursively(0);
    }

private:
    std::vector<Clause> clauses;
    std::vector<LiteralStatus> assignment;
    std::unordered_map<int, std::unordered_set<int>> literalsToClauses;

    bool solveRecursively(int decisionLevel) {
        if (conflictFound()) {
            int conflictClauseIndex = findConflictClause();
            lazyClauseUpdate(conflictClauseIndex, decisionLevel);
            return false;
        }

        if (allLiteralsAssigned()) {
            return true;
        }

        int literalToAssign = selectLiteral();

        assignment[literalToAssign] = LiteralStatus::True;
        if (solveRecursively(decisionLevel + 1)) {
            return true;
        }

        assignment[literalToAssign] = LiteralStatus::False;
        if (solveRecursively(decisionLevel + 1)) {
            return true;
        }

        assignment[literalToAssign] = LiteralStatus::Unassigned;
        return false;
    }

    bool conflictFound() {
        for (const Clause& clause : clauses) {
            if (clause.isActive) {
                bool clauseSatisfied = false;
                for (int literal : clause.literals) {
                    int absLiteral = std::abs(literal);
                    if ((literal > 0 && assignment[absLiteral] == LiteralStatus::True) ||
                        (literal < 0 && assignment[absLiteral] == LiteralStatus::False)) {
                        clauseSatisfied = true;
                        break;
                    }
                }
                if (!clauseSatisfied) {
                    return true;
                }
            }
        }
        return false;
    }

    int findConflictClause() {
        for (int i = 0; i < clauses.size(); ++i) {
            if (clauses[i].isActive) {
                bool clauseSatisfied = false;
                for (int literal : clauses[i].literals) {
                    int absLiteral = std::abs(literal);
                    if ((literal > 0 && assignment[absLiteral] == LiteralStatus::True) ||
                        (literal < 0 && assignment[absLiteral] == LiteralStatus::False)) {
                        clauseSatisfied = true;
                        break;
                    }
                }
                if (!clauseSatisfied) {
                    return i;
                }
            }
        }
        return -1;
    }

    void lazyClauseUpdate(int conflictClauseIndex, int decisionLevel) {
        clauses[conflictClauseIndex].isActive = false;

        for (int literal : clauses[conflictClauseIndex].literals) {
            int absLiteral = std::abs(literal);
            assignment[absLiteral] = LiteralStatus::Unassigned;
            if (literal > 0 && assignment[absLiteral] == LiteralStatus::Unassigned) {
                for (int clauseIndex : literalsToClauses[-literal]) {
                    clauses[clauseIndex].isActive = true;
                }
            }
        }

        backtrack(decisionLevel);
    }

    void backtrack(int decisionLevel) {
        for (int i = 1; i < assignment.size(); ++i) {
            if (i <= decisionLevel && assignment[i] != LiteralStatus::Unassigned) {
                assignment[i] = LiteralStatus::Unassigned;
            }
        }
        for (Clause& clause : clauses) {
            clause.isActive = true;
        }
    }

    int selectLiteral() {
        // 添加更智能的变元选择策略，例如选择出现次数最少的未分配文字
        int minOccurrences = std::numeric_limits<int>::max();
        int selectedLiteral = -1;

        for (int i = 1; i < assignment.size(); ++i) {
            if (assignment[i] == LiteralStatus::Unassigned) {
                int occurrences = 0;
                for (int clauseIndex : literalsToClauses[i]) {
                    if (clauses[clauseIndex].isActive) {
                        occurrences++;
                    }
                }
                if (occurrences < minOccurrences) {
                    minOccurrences = occurrences;
                    selectedLiteral = i;
                }
            }
        }

        return selectedLiteral;
    }

    bool allLiteralsAssigned() {
        for (int i = 1; i < assignment.size(); ++i) {
            if (assignment[i] == LiteralStatus::Unassigned) {
                return false;
            }
        }
        return true;
    }
};

int main() {
    std::vector<Clause> clauses = {
        {{1, -2}},
        {{-1, 2}},
        {{-1, -2}}
    };

    DPLLSolver solver(clauses);
    if (solver.solve()) {
        std::cout << "可满足" << std::endl;
    } else {
        std::cout << "不可满足" << std::endl;
    }

    return 0;
}
