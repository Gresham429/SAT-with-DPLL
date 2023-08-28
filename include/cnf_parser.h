#ifndef CNF_PARSER_H
#define CNF_PARSER_H

#include "list.h"
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
    // 定义子句
    struct clause
    {
        // 子句中的文字数组
        DeLinkList<int> literals;
    };
    
    // 读取cnf文件
    bool ReadCNFFile(const std::string filename, DeLinkList<clause> &clauses);

    // 判断是否为单子句（只有一个文字）
    bool IsUnitClause(clause clause);

    // 判断子句中是否含有某个文字
    bool HaveLiteral(clause clause, int literal);

    void WriteRes(std::vector<LiteralStatus> assignemnt, std::string filename, bool flag, long long runtime);
};

#endif