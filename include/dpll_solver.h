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
    DPLLSolver() : flag(false) {}

    bool BasicSolve(DeLinkList<cnf_parser::clause> &clauses, int BoolCount)
    {
        LiteralStatus assignment[BoolCount + 1] = {LiteralStatus::Unassigned};

        return BasicSolveRecursively(clauses, assignment, BoolCount);
    }

    bool OptimizedSolve1(DeLinkList<cnf_parser::clause> &clauses, int BoolCount)
    {
        LiteralStatus assignment[BoolCount + 1] = {LiteralStatus::Unassigned};

        return OptimizedSolveRecursively1(clauses, assignment, BoolCount);
    }

    bool OptimizedSolve2(DeLinkList<cnf_parser::clause> &clauses, int BoolCount)
    {
        LiteralStatus assignment[BoolCount + 1] = {LiteralStatus::Unassigned};

        this->threshold = clauses.size() >> 1;

        return OptimizedSolveRecursively2(clauses, assignment, BoolCount);
    }

    std::vector<LiteralStatus> GetAssignment()
    {
        return this->assignment_result;
    }

    std::vector<size_t> GetLeftNums()
    {
        return this->left_nums;
    }

private:
    bool flag;
    int threshold;
    std::string filename;
    std::vector<LiteralStatus> assignment_result;
    std::vector<size_t> left_nums;
    typedef std::pair<int, int> PII;

    // 基础分裂策略选择器
    // 选择出现次数最多的未赋值的文字作为分裂的变元
    int SelectLiteral(const PII counter[], const int BoolCount)
    {
        int max_occurence = 0;
        int select_literal = 0;

        for (int i = 1; i <= BoolCount; ++i)
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

    // MOM分裂策略
    int Branching(int min_size, DeLinkList<cnf_parser::clause> &clauses, const int BoolCount)
    {
        // 统计出现次数
        PII counter[BoolCount + 1] = {{0, 0}};

        for (auto it_clause = clauses.begin(); it_clause != clauses.end(); ++it_clause)
        {
            if ((*it_clause).literals.size() != min_size) continue;
            for (auto it_literal = (*it_clause).literals.begin(); it_literal != (*it_clause).literals.end(); ++it_literal)
            {  
                // 文字为真则第一个自增，否则第二个自增
                *it_literal > 0 ? counter[std::abs(*it_literal)].first++ : counter[std::abs(*it_literal)].second++;
            }
        }

        int max_score = 0;
        int select_literal = 0;

        for (int literal = 1; literal <= BoolCount; ++literal)
        {
            int score = (counter[literal].first + 1) * (counter[literal].second + 1);
            if (score > max_score)
            {
                max_score = score;
                select_literal = (counter[literal].first > counter[literal].second ? literal : -literal);
            }
        }

        return select_literal;
    }

    // 单子句传播
    void UnitClausePropagation(DeLinkList<cnf_parser::clause> &clauses, LiteralStatus assigenment[])
    {
        cnf_parser parser;

        while (true)
        {
            bool HaveUnitClause = false;
            std::vector<int> UnitLiterals;

            // 搜索产生的所有的单子句
            for (auto it_clause = clauses.begin(); it_clause != clauses.end(); ++it_clause)
            {
                if (parser.IsUnitClause(*it_clause))
                {
                    HaveUnitClause = true;
                    int UnitLiteral = (*it_clause).literals.front();
                    UnitLiterals.push_back(UnitLiteral);
                    assigenment[std::abs(UnitLiteral)] = (UnitLiteral > 0 ? LiteralStatus::True : LiteralStatus::False);
                }
            }

            // 没有单子句
            if (!HaveUnitClause)
                return;

            // 化简子句集合
            for (const auto UnitLiteral : UnitLiterals)
            {
                for (auto it_clause = clauses.begin(); it_clause != clauses.end();)
                {
                    bool remove_clause_flag = false;

                    for (auto it_literal = (*it_clause).literals.begin(); it_literal != (*it_clause).literals.end();)
                    {
                        bool remove_literal_flag = false;

                        if ((*it_literal) == UnitLiteral) 
                        {
                            remove_clause_flag = true;
                            it_clause = clauses.remove_node(it_clause);
                            break;
                        }
                        else if ((*it_literal) == -UnitLiteral)
                        {
                            remove_literal_flag = true;
                            it_literal = (*it_clause).literals.remove_node(it_literal);
                        }

                        if (!remove_literal_flag) ++it_literal;
                    }

                    if (!remove_clause_flag)
                        ++it_clause;
                }
            }
        }
    }

    // 纯文字传播
    void PureLiteralPropagation(DeLinkList<cnf_parser::clause> &clauses, PII counter[], const int BoolCount)
    {
        for (int i = 1; i <= BoolCount; ++i)
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
    bool BasicSolveRecursively(DeLinkList<cnf_parser::clause> &clauses, LiteralStatus assignment[], const int BoolCount)
    {
        // 单子句传播
        UnitClausePropagation(clauses, assignment);

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

        // 所有子句被满足，返回true
        if (clauses.empty())
        {
            this->assignment_result = std::vector<LiteralStatus>(assignment, assignment + BoolCount + 1);
            left_nums.push_back(0);
            return true;
        }

        // 含有空子句，返回false
        for (auto it_clause = clauses.begin(); it_clause != clauses.end(); ++it_clause)
        {
            if ((*it_clause).literals.empty())
            {
                left_nums.push_back(clauses.size());
                return false;
            }
        }

        // 分裂策略
        int select_literal = SelectLiteral(counter, BoolCount);

        // 构建搜索副本
        DeLinkList<cnf_parser::clause> clauses_tmp = clauses;

        // 构建左子句集合
        cnf_parser::clause clause_left;
        clause_left.literals.push_back(select_literal);
        clauses_tmp.push_front(clause_left);
        
        if (BasicSolveRecursively(clauses_tmp, assignment, BoolCount))
            return true;

        // 构建右子句集合
        cnf_parser::clause clause_right;
        clause_right.literals.push_back(-select_literal);
        clauses.push_front(clause_right);
        
        return BasicSolveRecursively(clauses, assignment, BoolCount);
    }

    // 优化后的DPLL求解器1
    bool OptimizedSolveRecursively1(DeLinkList<cnf_parser::clause> &clauses, LiteralStatus assignment[],const int BoolCount)
    {
        // 单子句传播
        UnitClausePropagation(clauses, assignment);

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

        // 所有子句被满足，返回true
        if (clauses.empty())
        {
            this->assignment_result = std::vector<LiteralStatus>(assignment, assignment + BoolCount + 1);
            left_nums.push_back(0);
            return true;
        }

        // 记录最小的非单子句数量
        int min_size = 1e9;

        // 含有空子句，返回false
        for (auto it_clause = clauses.begin(); it_clause != clauses.end(); ++it_clause)
        {
            if ((*it_clause).literals.empty())
            {
                left_nums.push_back(clauses.size());
                return false;
            }

            if ((*it_clause).literals.size() < min_size && (*it_clause).literals.size() > 1) min_size = (*it_clause).literals.size();
        }

        // 分裂策略
        int select_literal = 0;
  
        select_literal = Branching(min_size, clauses, BoolCount);

        // 构建搜索副本
        DeLinkList<cnf_parser::clause> clauses_tmp = clauses;

        // 构建左子句集合
        cnf_parser::clause clause_left;
        clause_left.literals.push_back(select_literal);
        clauses_tmp.push_front(clause_left);
        
        if (OptimizedSolveRecursively1(clauses_tmp, assignment, BoolCount))
            return true;

        // 构建右子句集合
        cnf_parser::clause clause_right;
        clause_right.literals.push_back(-select_literal);
        clauses.push_front(clause_right);
        
        return OptimizedSolveRecursively1(clauses, assignment, BoolCount);
    }

    // 优化后的DPLL求解器2
    bool OptimizedSolveRecursively2(DeLinkList<cnf_parser::clause> &clauses, LiteralStatus assignment[],const int BoolCount)
    {
        // 单子句传播
        UnitClausePropagation(clauses, assignment);

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

        // 所有子句被满足，返回true
        if (clauses.empty())
        {
            this->assignment_result = std::vector<LiteralStatus>(assignment, assignment + BoolCount + 1);
            left_nums.push_back(0);
            return true;
        }

        // 记录最小的非单子句数量
        int min_size = 1e9;

        // 含有空子句，返回false
        for (auto it_clause = clauses.begin(); it_clause != clauses.end(); ++it_clause)
        {
            if ((*it_clause).literals.empty())
            {
                left_nums.push_back(clauses.size());
                return false;
            }

            if ((*it_clause).literals.size() < min_size && (*it_clause).literals.size() > 1) min_size = (*it_clause).literals.size();
        }

        // 分裂策略
        int select_literal = 0;
        if (clauses.size() > threshold)
        {
            select_literal = SelectLiteral(counter, BoolCount);
        }
        else
        {
            select_literal = Branching(min_size, clauses, BoolCount);
        }

        // 构建搜索副本
        DeLinkList<cnf_parser::clause> clauses_tmp = clauses;

        // 构建左子句集合
        cnf_parser::clause clause_left;
        clause_left.literals.push_back(select_literal);
        clauses_tmp.push_front(clause_left);
        
        if (OptimizedSolveRecursively2(clauses_tmp, assignment, BoolCount))
            return true;

        // 构建右子句集合
        cnf_parser::clause clause_right;
        clause_right.literals.push_back(-select_literal);
        clauses.push_front(clause_right);
        
        return OptimizedSolveRecursively2(clauses, assignment, BoolCount);
    }
};

#endif