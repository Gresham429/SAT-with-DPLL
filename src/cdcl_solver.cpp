#include "../include/cdcl_solver.h"

CDCLSolver::CDCLSolver(const DeLinkList<cnf_parser::clause>& clauses, LiteralStatus assignment[], int BoolCount) : clauses_(clauses), assignment_(assignment), BoolCount_(BoolCount) 
{
    // 初始化 CDCLSolver
    // 初始非时许回溯级别为0
    this->targetNonChronoLevel_ = 0;
}

void CDCLSolver::InitializeSolver()
{
    // 初始化 CDCLSolver 的各个成员
    this->decisions_.clear();
    this->learnedClauses_.clear();
    this->vsidsScores_.clear();
    this->currentLevel_ = 0;
    this->conflictCounter_ = 0;
    this->restartInterval_ = 0;

    // 初始化文字的 VSIDS 分数
    for (int i = 1; i <= BoolCount_; ++i)
    {
        vsidsScores_[i] = 0;
        vsidsScores_[-i] = 0;
    }
}

bool CDCLSolver::PerformUnitPropagation()
{
    // 单子句传播和纯文字传播逻辑
    // 返回是否发生冲突
    bool confict = false;

    while (true)
    {
        bool unitPropagation = false;

        for (auto it_clause = clauses_.begin(); it_clause != clauses_.end(); ++it_clause)
        {

        }
    }
}

void CDCLSolver::LearnClause(const cnf_parser::clause& conflictingClause)
{
    // 实现学习子句的逻辑
}

cnf_parser::clause CDCLSolver::AnalyzeConflict()
{
    // 实现冲突分析的逻辑
    // 返回冲突子句
}

int CDCLSolver::SelectNextVariable()
{
    // 实现基于VSIDS策略的变量选择
    // 返回选择的文字
}

void CDCLSolver::Backtrack(int targetLevel)
{
    // 实现非时序回溯逻辑
}

bool CDCLSolver::Solve()
{
    InitializeSolver();

    while (true)
    {
        bool conflict = PerformUnitPropagation();

        if (clauses_.empty())
        {
            // 所有子句都被满足，返回true
            return true;
        }

        if (conflict)
        {
            cnf_parser::clause learnedClause = AnalyzeConflict();

            if (learnedClause.literals.empty())
            {
                // 冲突分析未能学到新的子句，返回false
                return false;
            } else
            {
                clauses_.push_back(learnedClause);
                LearnClause(learnedClause);
                Backtrack(somePreviousLevel);
                conflictCounter_++;

                if (conflictCounter_ >= restartInterval_)
                {
                    targetNonChronoLevel_ = 0;
                    Backtrack(0);
                    restartInterval_ *= someMultiplier;
                }
            }
        }
        else
        {
            int select_literal = SelectNextVariable();
            decisions_.insert(select_literal);
            currentLevel_++;
        }
    }
}