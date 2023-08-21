#ifndef CNF_PARSER_H
#define CNF_PARSER_H

#include <iostream>
#include <utility>
#include <map>
#include <vector>

enum class LiteralStatus
{
    Unassigned,
    True,
    False,
};

class cnf_parser
{
public:
    //定义子句
    struct clause
    {
        //子句中的文字数组
        std::vector<int> literals;
        //子句真假
        bool satisfied_flag = false;
    };
    
    //读取cnf文件
    bool ReadCNFFile(const std::string filename, std::vector<clause> &clauses);

    //判断是否为单子句（只有一个文字）
    bool IsUnitClause(clause clause);

    //判断子句的真假
    bool EvaluateClause(clause clause, std::vector<LiteralStatus> assignment);
};

#endif