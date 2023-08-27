#ifndef DPLL_SOLVER_H
#define DPLL_SOLVER_H

#include "cnf_parser.h"
#include "utils.h"
#include "list.h"
#include <iostream>
#include <vector>
#include <mutex>
#include <algorithm>

class DPLLSolver
{
public:
    DPLLSolver(int minThreads, int maxThreads)
        : min_threads(minThreads), max_threads(maxThreads), threadPool(minThreads, maxThreads)
    {
    }

    bool BasicSolve(DeLinkList<cnf_parser::clause> clauses, int BoolCount)
    {
        LiteralStatus assignment[BoolCount + 1] = {LiteralStatus::Unassigned};
        return BasicSolveRecursively(clauses, assignment, BoolCount);
    }

    bool OptimizedSolve(DeLinkList<cnf_parser::clause> clauses, int BoolCount)
    {
        LiteralStatus assignment[BoolCount + 1] = {LiteralStatus::Unassigned};
        return OptimizedSolveRecursively(clauses, assignment, BoolCount);
    }

private:
    int min_threads, max_threads;
    ThreadPool threadPool;
    typedef std::pair<int, int> PII;

    // 分裂策略选择器
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

                if (!remove_flag) ++it_clause;
            }

            counter[std::abs(UnitLiteral)] = {0, 0};
        }
    }

    // 纯文字传播
    void PureLiteralPropagation(DeLinkList<cnf_parser::clause> &clauses, PII counter[],const int BoolCount)
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
    bool BasicSolveRecursively(DeLinkList<cnf_parser::clause> clauses, LiteralStatus assignemnt[],const int BoolCount)
    {
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
        UnitClausePropagation(clauses, assignemnt, counter);

        // 所有子句被满足，返回true
        if (clauses.empty()) return true;
        
        // 含有空子句，返回false
        for (auto it_clause = clauses.begin(); it_clause != clauses.end(); ++it_clause)
        {
            if ((*it_clause).literals.empty()) return false;
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
            assignemnt_left[i] = assignemnt[i];
        }
        if (BasicSolveRecursively(clauses, assignemnt_left, BoolCount)) return true;
    
        clauses.pop_front();

        // 构建右子句集合
        cnf_parser::clause clause_right;
        clause_right.literals.push_back(-select_literal);
        clauses.push_front(clause_right);
        LiteralStatus assignemnt_right[BoolCount + 1];
        for (size_t i = 0; i <= BoolCount; ++i)
        {
            assignemnt_right[i] = assignemnt[i];
        }
        return BasicSolveRecursively(clauses, assignemnt_right, BoolCount);
    }

    // 多线程优化后的DPLL求解器
    bool OptimizedSolveRecursively(DeLinkList<cnf_parser::clause> clauses, LiteralStatus assignemnt[], int BoolCount)
    {
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
        UnitClausePropagation(clauses, assignemnt, counter);

        // 所有子句被满足，返回true
        if (clauses.empty()) return true;
        
        // 含有空子句，返回false
        for (auto it_clause = clauses.begin(); it_clause != clauses.end(); ++it_clause)
        {
            if ((*it_clause).literals.empty()) return false;
        }

        // 分裂策略
        int select_literal = SelectLiteral(counter, BoolCount);

        // 构建左子句集合
        cnf_parser::clause clause_left;
        clause_left.literals.push_back(select_literal);
        DeLinkList<cnf_parser::clause> clauses_left = clauses;
        clauses_left.push_front(clause_left);
        LiteralStatus assignemnt_left[BoolCount + 1];
        for (size_t i = 0; i <= BoolCount; ++i)
        {
            assignemnt_left[i] = assignemnt[i];
        }
        if (BasicSolveRecursively(clauses_left, assignemnt_left, BoolCount)) return true;
    
        // 构建右子句集合
        cnf_parser::clause clause_right;
        clause_right.literals.push_back(-select_literal);
        DeLinkList<cnf_parser::clause> clauses_right = clauses;
        clauses_right.push_front(clause_right);
        LiteralStatus assignemnt_right[BoolCount + 1];
        for (size_t i = 0; i <= BoolCount; ++i)
        {
            assignemnt_right[i] = assignemnt[i];
        }
        return BasicSolveRecursively(clauses_right, assignemnt_right, BoolCount);
    }
};

#endif