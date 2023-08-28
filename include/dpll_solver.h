#ifndef DPLL_SOLVER_H
#define DPLL_SOLVER_H

#include "cnf_parser.h"
#include "utils.h"
#include "list.h"
#include <chrono>
#include <iostream>
#include <vector>
#include <mutex>
#include <future>
#include <algorithm>
#include <atomic>

std::atomic<bool> cancelFlag(false);

class DPLLSolver
{
public:
    DPLLSolver(int minThreads, int maxThreads)
        : min_threads(minThreads), max_threads(maxThreads), threadPool(minThreads, maxThreads), flag(false)
    {
    }

    bool BasicSolve(DeLinkList<cnf_parser::clause> clauses, int BoolCount)
    {
        LiteralStatus assignment[BoolCount + 1] = {LiteralStatus::Unassigned};

        // Create a new thread for the solving process
        std::future<bool> solverFuture = std::async(std::launch::async, [&]() {
            return BasicSolveRecursively(clauses, assignment, BoolCount);
        });

        // Wait for the solving thread to finish or timeout
        auto status = solverFuture.wait_for(std::chrono::seconds(1800));

        if (status == std::future_status::timeout)
        {
            // Set the cancel flag to true
            cancelFlag.store(true, std::memory_order_relaxed);

            // Wait for the solving thread to finish
            solverFuture.wait();

            // reset the cancel flag to false
            cancelFlag.store(false, std::memory_order_relaxed);

            return false;
        }

        return solverFuture.get(); // Get the result of solving
    }

    bool OptimizedSolve(DeLinkList<cnf_parser::clause> clauses, int BoolCount)
    {
        LiteralStatus assignment[BoolCount + 1] = {LiteralStatus::Unassigned};

        // Create a new thread for the solving process
        std::future<bool> solverFuture = std::async(std::launch::async, [&]() {
            return OptimizedSolveRecursively(clauses, assignment, BoolCount);
        });

        // Wait for the solving thread to finish or timeout
        auto status = solverFuture.wait_for(std::chrono::seconds(1800));

        if (status == std::future_status::timeout)
        {
            // Set the cancel flag to true
            cancelFlag.store(true, std::memory_order_relaxed);

            // Wait for the solving thread to finish
            solverFuture.wait();

            // reset the cancel flag to false
            cancelFlag.store(false, std::memory_order_relaxed);

            return false;
        }

        return solverFuture.get(); // Get the result of solving
    }

    std::vector<LiteralStatus> GetResult()
    {
        return this->assignment_result;
    }

private:
    int min_threads, max_threads;
    std::string filename;
    bool flag;
    ThreadPool threadPool;
    std::vector<LiteralStatus> assignment_result;
    typedef std::pair<int, int> PII;

    // 基础分裂策略选择器
    // 选择出现次数最多的未赋值的文字作为分裂的变元
    int SelectLiteral(const PII counter[], const int BoolCount)
    {
        int max_occurence = 0;
        int select_literal = 0;

        for (size_t i = 1; i <= BoolCount; ++i)
        {
            int occurence = counter[i].first + counter[i].second;
            if (occurence > max_occurence)
            {
                max_occurence = occurence;
                select_literal = i;
            }
        }

        return select_literal;
    }

    // 优化后的分裂策略选择器
    // 选择出现次数最多的未赋值的文字作为分裂的变元(区分真假)
    int OptimizedSelectLiteral(const PII counter[], const int BoolCount)
    {
        int max_occurence = 0;
        int select_literal = 0;

        for (size_t i = 1; i <= BoolCount; ++i)
        {
            int occurence = counter[i].first + counter[i].second;
            if (occurence > max_occurence)
            {
                max_occurence = occurence;
                select_literal = (counter[i].first > counter[i].second ? i : -i);
            }
        }

        return select_literal;
    }

    // 单子句传播
    void UnitClausePropagation(DeLinkList<cnf_parser::clause> &clauses, LiteralStatus *assigenment, PII *counter)
    {
        cnf_parser parser;

        while (true)
        {
            bool HaveUnitClause = false;
            int UnitLiteral;

            for (auto it_clause = clauses.begin(); it_clause != clauses.end(); ++it_clause)
            {
                if (parser.IsUnitClause(*it_clause))
                {
                    HaveUnitClause = true;
                    UnitLiteral = (*it_clause).literals.front();
                    assigenment[std::abs(UnitLiteral)] = (UnitLiteral > 0 ? LiteralStatus::True : LiteralStatus::False);
                    break;
                }
            }

            if (!HaveUnitClause)
                return;

            for (auto it_clause = clauses.begin(); it_clause != clauses.end();)
            {
                bool remove_flag = false;

                if (parser.HaveLiteral(*it_clause, UnitLiteral))
                {
                    for (auto it_literal = (*it_clause).literals.begin(); it_literal != (*it_clause).literals.end(); ++it_literal)
                    {
                        *it_literal > 0 ? counter[std::abs(*it_literal)].first-- : counter[std::abs(*it_literal)].second--;
                    }
                    remove_flag = true;
                    it_clause = clauses.remove_node(it_clause);
                }
                else if (parser.HaveLiteral(*it_clause, -UnitLiteral))
                {
                    (*it_clause).literals.remove_node(-UnitLiteral);
                }

                if (!remove_flag)
                    ++it_clause;
            }

            counter[std::abs(UnitLiteral)] = {0, 0};
        }
    }

    // 纯文字传播
    void PureLiteralPropagation(DeLinkList<cnf_parser::clause> &clauses, PII counter[], const int BoolCount)
    {
        for (size_t i = 1; i <= BoolCount; ++i)
        {
            if (counter[i].first != 0 && counter[i].second == 0)
            {
                cnf_parser::clause temp;
                temp.literals.push_back(i);
                clauses.push_front(temp);
                counter[i].first++;
            }
            else if (counter[i].first == 0 && counter[i].second != 0)
            {
                cnf_parser::clause temp;
                temp.literals.push_back(-i);
                clauses.push_front(temp);
                counter[i].second++;
            }
        }
    }

    // 基础DPLL求解器
    bool BasicSolveRecursively(DeLinkList<cnf_parser::clause> clauses, LiteralStatus assignment[], const int BoolCount)
    {
        if (cancelFlag) return true;

        // 统计出现次数
        PII counter[BoolCount + 1] = {{0, 0}};

        for (auto it_clause = clauses.begin(); it_clause != clauses.end(); ++it_clause)
        {
            for (auto it_literal = (*it_clause).literals.begin(); it_literal != (*it_clause).literals.end(); ++it_literal)
            {
                // 文字为真则第一个自增，否则第二个自增
                *it_literal > 0 ? counter[std::abs(*it_literal)].first++ : counter[std::abs(*it_literal)].second++;
            }
        }

        // 纯文字传播
        PureLiteralPropagation(clauses, counter, BoolCount);

        // 单子句传播
        UnitClausePropagation(clauses, assignment, counter);

        // 所有子句被满足，返回true
        if (clauses.empty())
        {
            this->assignment_result = std::vector<LiteralStatus>(assignment, assignment + BoolCount);
            return true;
        }

        // 含有空子句，返回false
        for (auto it_clause = clauses.begin(); it_clause != clauses.end(); ++it_clause)
        {
            if ((*it_clause).literals.empty())
                return false;
        }

        // 分裂策略
        int select_literal = SelectLiteral(counter, BoolCount);

        // 构建左子句集合
        cnf_parser::clause clause_left;
        clause_left.literals.push_back(select_literal);
        clauses.push_front(clause_left);
        LiteralStatus assignemnt_left[BoolCount + 1];
        for (size_t i = 0; i <= BoolCount; ++i)
        {
            assignemnt_left[i] = assignment[i];
        }
        if (BasicSolveRecursively(clauses, assignemnt_left, BoolCount))
            return true;

        clauses.pop_front();

        // 构建右子句集合
        cnf_parser::clause clause_right;
        clause_right.literals.push_back(-select_literal);
        clauses.push_front(clause_right);
        LiteralStatus assignemnt_right[BoolCount + 1];
        for (size_t i = 0; i <= BoolCount; ++i)
        {
            assignemnt_right[i] = assignment[i];
        }
        return BasicSolveRecursively(clauses, assignemnt_right, BoolCount);
    }

    // 多线程优化后的DPLL求解器
    bool OptimizedSolveRecursively(DeLinkList<cnf_parser::clause> clauses, LiteralStatus assignment[], int BoolCount)
    {
        if (cancelFlag) return true;

        // 统计出现次数
        PII counter[BoolCount + 1] = {{0, 0}};

        for (auto it_clause = clauses.begin(); it_clause != clauses.end(); ++it_clause)
        {
            for (auto it_literal = (*it_clause).literals.begin(); it_literal != (*it_clause).literals.end(); ++it_literal)
            {
                // 文字为真则第一个自增，否则第二个自增
                *it_literal > 0 ? counter[std::abs(*it_literal)].first++ : counter[std::abs(*it_literal)].second++;
            }
        }

        // 纯文字传播
        PureLiteralPropagation(clauses, counter, BoolCount);

        // 单子句传播
        UnitClausePropagation(clauses, assignment, counter);

        // 所有子句被满足，返回true
        if (clauses.empty())
        {
            this->assignment_result = std::vector<LiteralStatus>(assignment, assignment + BoolCount);
            return true;
        }

        // 含有空子句，返回false
        for (auto it_clause = clauses.begin(); it_clause != clauses.end(); ++it_clause)
        {
            if ((*it_clause).literals.empty())
                return false;
        }

        // 分裂策略
        int select_literal = OptimizedSelectLiteral(counter, BoolCount);

        // 构建左子句集合
        cnf_parser::clause clause_left;
        clause_left.literals.push_back(select_literal);
        DeLinkList<cnf_parser::clause> clauses_left = clauses;
        clauses_left.push_front(clause_left);
        LiteralStatus assignemnt_left[BoolCount + 1];
        for (size_t i = 0; i <= BoolCount; ++i)
        {
            assignemnt_left[i] = assignment[i];
        }

        // 构建右子句集合
        cnf_parser::clause clause_right;
        clause_right.literals.push_back(-select_literal);
        DeLinkList<cnf_parser::clause> clauses_right = clauses;
        clauses_right.push_front(clause_right);
        LiteralStatus assignemnt_right[BoolCount + 1];
        for (size_t i = 0; i <= BoolCount; ++i)
        {
            assignemnt_right[i] = assignment[i];
        }

        // 加入线程池的任务队列
        std::vector<std::future<bool>> resultFutures;

        resultFutures.push_back(threadPool.EnqueueTask([this, clauses_left, &assignemnt_left, BoolCount]()
                                                       { return BasicSolveRecursively(clauses_left, assignemnt_left, BoolCount); }));

        resultFutures.push_back(threadPool.EnqueueTask([this, clauses_right, &assignemnt_right, BoolCount]()
                                                       { return BasicSolveRecursively(clauses_right, assignemnt_right, BoolCount); }));

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