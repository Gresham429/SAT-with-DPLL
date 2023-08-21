#ifndef DPLL_SOLVER_H
#define DPLL_SOLVER_H

#include "cnf_parser.h"
#include "utils.h"
#include <iostream>
#include <vector>
#include <mutex>
#include <unordered_map>

class DPLLSolver
{
public:
    DPLLSolver(int minThreads, int maxThreads)
        : min_threads(minThreads), max_threads(maxThreads), threadPool(minThreads, maxThreads)
    {
    }

    bool BasicSolve(std::vector<cnf_parser::clause> clauses, int BoolCount)
    {
        this->clauses = clauses;
        this->assignment = std::vector<LiteralStatus>(BoolCount + 1, LiteralStatus::Unassigned);
        return BasicSolveRecursively(clauses, assignment);
    }

    bool OptimizedSolve(std::vector<cnf_parser::clause> clauses, int BoolCount)
    {
        this->clauses = clauses;
        this->assignment = std::vector<LiteralStatus>(BoolCount + 1, LiteralStatus::Unassigned);
        return OptimizedSolveRecursively(clauses, assignment);
    }

private:
    std::vector<cnf_parser::clause> clauses;
    std::vector<LiteralStatus> assignment;
    int min_threads, max_threads;
    ThreadPool threadPool;

    // 分裂策略选择器
    // 选择出现次数最多的未赋值的文字作为分裂的变元
    int SelectLiteral(const std::vector<cnf_parser::clause> clauses, const std::vector<LiteralStatus> &assignment)
    {
        extern int BoolCount;

        std::vector<int> literalOccurrences(BoolCount + 1, 0); // 初始化变元出现次数数组

        for (const auto &clause : clauses)
        {
            for (int literal : clause.literals)
            {
                int absLiteral = std::abs(literal);
                if (assignment[absLiteral] == LiteralStatus::Unassigned)
                {
                    literalOccurrences[absLiteral]++;
                }
            }
        }

        int selectedLiteral = -1;
        int maxOccurrences = 0;

        for (int i = 1; i <= BoolCount; ++i)
        {
            if (assignment[i] == LiteralStatus::Unassigned && literalOccurrences[i] > maxOccurrences)
            {
                selectedLiteral = i;
                maxOccurrences = literalOccurrences[i];
            }
        }

        return selectedLiteral;
    }

    // 化简子句集合
    void SimplifyClauses(std::vector<cnf_parser::clause> &clauses, const cnf_parser &parser, int unit_literal)
    {
        for (auto clause_it = clauses.begin(); clause_it != clauses.end();)
        {
            bool RemoveClause = false;

            for (auto literal_it = clause_it->literals.begin(); literal_it != clause_it->literals.end(); ++literal_it)
            {
                if (*literal_it == unit_literal)
                {
                    RemoveClause = true;
                    break;
                }
                else if (*literal_it == -unit_literal)
                {
                    clause_it->literals.erase(literal_it);
                    break;
                }
            }

            if (RemoveClause)
            {
                clause_it = clauses.erase(clause_it);
            }
            else
            {
                ++clause_it;
            }
        }
    }

    // 纯文字传播
    void PureLiteralPropagation(std::vector<cnf_parser::clause> &clauses, std::unordered_map<int, int> counter)
    {
        extern int BoolCount;

        for (int i = 1; i <= BoolCount; ++i)
        {
            if (counter.find(i) != counter.end() && counter.find(-i) == counter.end())
            {
                cnf_parser::clause tmp;
                tmp.literals = {i};
                tmp.satisfied_flag = true;
                clauses.push_back(tmp);
            }
            else if (counter.find(i) == counter.end() && counter.find(-i) != counter.end())
            {
                cnf_parser::clause tmp;
                tmp.literals = {-i};
                tmp.satisfied_flag = true;
                clauses.push_back(tmp);
            }
        }
    }

    void HashCount(const std::vector<cnf_parser::clause> clauses, std::unordered_map<int, int> &counters)
    {
        for (const auto &clause : clauses)
        {
            for (int literal : clause.literals)
            {
                if (counters.find(literal) != counters.end())
                {
                    ++counters[literal];
                }
                else
                {
                    counters.emplace(literal, 1);
                }
            }
        }
    }

    // 基础DPLL求解器
    bool BasicSolveRecursively(std::vector<cnf_parser::clause> clauses, std::vector<LiteralStatus> assignment)
    {
        cnf_parser parser;

        // 用HashMap作为计数器
        std::unordered_map<int, int> counters;

        HashCount(clauses, counters);

        // 纯文字传播
        PureLiteralPropagation(clauses, counters);

        // 单子句传播，化简子句集合
        while (true)
        {
            bool unit_clause_found = false;
            int unit_literal = 0;

            // 查找单子句
            for (const auto &clause : clauses)
            {
                if (parser.IsUnitClause(clause))
                {
                    unit_clause_found = true;
                    unit_literal = clause.literals[0];
                    break;
                }
            }

            // 没有单子句了，退出循环
            if (!unit_clause_found)
            {
                break;
            }

            // 更新 literal 赋值
            assignment[std::abs(unit_literal)] = (unit_literal > 0) ? LiteralStatus::True : LiteralStatus::False;

            // 化简子句集
            SimplifyClauses(clauses, parser, unit_literal);
        }

        // 所有子句都满足，返回 true
        if (clauses.empty())
        {
            return true;
        }

        // clauses 中有空子句，返回 false
        for (const auto &clause : clauses)
        {
            if (clause.literals.empty())
            {
                return false;
            }
        }

        // 选择一个变元 v
        int select_literal = SelectLiteral(clauses, assignment);

        // 在第一分支中搜索
        std::vector<cnf_parser::clause> clauses_left = clauses;

        // 构建单子句
        cnf_parser::clause clause_left;
        clause_left.literals = {select_literal};
        clause_left.satisfied_flag = true;

        clauses_left.push_back(clause_left);

        if (BasicSolveRecursively(clauses_left, assignment)) return true;

        // 回溯到对 v 执行分支策略的初态进入另一分支
        std::vector<cnf_parser::clause> clauses_right = clauses;

        // 构建单子句
        cnf_parser::clause clause_right;
        clause_right.literals = {-select_literal};
        clause_right.satisfied_flag = true;

        clauses_right.push_back(clause_right); // 添加单子句

        return BasicSolveRecursively(clauses_right, assignment);
    }

    // 多线程优化后的DPLL求解器
    bool OptimizedSolveRecursively(std::vector<cnf_parser::clause> clauses, std::vector<LiteralStatus> assignment)
    {
        cnf_parser parser;

        // 用HashMap作为计数器
        std::unordered_map<int, int> counter;

        for (const auto &clause : clauses)
        {
            for (int literal : clause.literals)
            {
                if (counter.find(literal) != counter.end())
                {
                    ++counter[literal];
                }
                else
                {
                    counter.emplace(literal, 1);
                }
            }
        }

        // 纯文字传播
        PureLiteralPropagation(clauses, counter);

        // 单子句传播，化简子句集合
        while (true)
        {
            bool unit_clause_found = false;
            int unit_literal = 0;

            // 查找单子句
            for (const auto &clause : clauses)
            {
                if (parser.IsUnitClause(clause))
                {
                    unit_clause_found = true;
                    unit_literal = clause.literals[0];
                    break;
                }
            }

            // 没有单子句了，退出循环
            if (!unit_clause_found)
            {
                break;
            }

            // 更新 literal 赋值
            assignment[std::abs(unit_literal)] = (unit_literal > 0) ? LiteralStatus::True : LiteralStatus::False;

            // 化简子句集
            SimplifyClauses(clauses, parser, unit_literal);
        }

        // 所有子句都满足，返回 true
        if (clauses.empty())
        {
            return true;
        }

        // clauses 中有空子句，返回 false
        for (const auto &clause : clauses)
        {
            if (clause.literals.empty())
            {
                return false;
            }
        }

        // 选择一个变元 v
        int select_literal = SelectLiteral(clauses, assignment);

        // 在第一分支中搜索
        std::vector<cnf_parser::clause> clauses_left = clauses;

        // 构建单子句
        cnf_parser::clause clause_left;
        clause_left.literals = {select_literal};
        clause_left.satisfied_flag = true;

        clauses_left.push_back(clause_left);

        // 回溯到对 v 执行分支策略的初态进入另一分支
        std::vector<cnf_parser::clause> clauses_right = clauses;

        // 构建单子句
        cnf_parser::clause clause_right;
        clause_right.literals = {-select_literal};
        clause_right.satisfied_flag = true;

        clauses_right.push_back(clause_right); // 添加单子句

        // 尝试开辟新线程搜索
        std::vector<std::future<bool>> resultFutures; // 构建结果数组
        std::vector<LiteralStatus> assignment_tmp1 = assignment;
        std::vector<LiteralStatus> assignment_tmp2 = assignment;

        resultFutures.push_back(threadPool.EnqueueTask([this, clauses_left, assignment_tmp1]() {
            return  BasicSolveRecursively(clauses_left, assignment_tmp1);
        }));
        resultFutures.push_back(threadPool.EnqueueTask([this, clauses_right, assignment_tmp2]() {
            return  BasicSolveRecursively(clauses_right, assignment_tmp2);
        }));

        // resultFutures.push_back(std::async(std::launch::async | std::launch::deferred, [this, clauses_left, assignment_tmp2]() {
        //     return  BasicSolveRecursively(clauses_left, assignment_tmp2);
        // }));

        // resultFutures.push_back(std::async(std::launch::async | std::launch::deferred, [this, clauses_right, assignment_tmp2]() {
        //     return  BasicSolveRecursively(clauses_right, assignment_tmp2);
        // }));

        // 并行的去获取结果
        std::vector<std::shared_future<bool>> sharedFutures;
        for (auto &result : resultFutures)
            sharedFutures.push_back(result.share());

        for (auto &shared_result : sharedFutures)
        {
            if (shared_result.get())
            {
                return true;
            }
        }

        return false;
    }
};

#endif