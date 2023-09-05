#ifndef CDCL_SOLVER_H
#define CDCL_SOLVER_H

#include "cnf_parser.h"
#include "list.h"
#include <iostream>
#include <vector>
#include <unordered_set>
#include <unordered_map>
#include <cstdlib>

// 定义学到的子句结构
struct LearnedClause {
    cnf_parser::clause clause;
    int level;
};

class CDCLSolver {
public:
    CDCLSolver(const DeLinkList<cnf_parser::clause>& clauses, LiteralStatus assignment[], int BoolCount);

    bool Solve();

private:
    void InitializeSolver();
    bool PerformUnitPropagation();
    void LearnClause(const cnf_parser::clause& conflictingClause);
    cnf_parser::clause AnalyzeConflict();
    int SelectNextVariable();
    void Backtrack(int targetLevel);

    DeLinkList<cnf_parser::clause> clauses_;
    LiteralStatus* assignment_;
    int BoolCount_;

    std::unordered_set<int> decisions_;
    std::unordered_map<int, LearnedClause> learnedClauses_;
    std::unordered_map<int, int> vsidsScores_;
    int currentLevel_;
    int targetNonChronoLevel_;
    int conflictCounter_;
    int restartInterval_;

    // 其他私有成员和辅助函数
};

#endif