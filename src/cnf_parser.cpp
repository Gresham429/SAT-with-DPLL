#include "../include/cnf_parser.h"
#include <fstream>
#include <sstream>
#include <vector>

int BoolCount, ClauseCount;

// 读取cnf文件
bool cnf_parser::ReadCNFFile(const std::string filename, DeLinkList<clause> &clauses)
{
    std::ifstream inFile(filename);

    if (!inFile)
    {
        // 处理打开失败的文件
        std::cerr << "文件打开失败,请重新读取" << std::endl;
        return false;
    }

    // 读取cnf文件成功
    std::cout << "成功读取cnf文件" << std::endl;

    std::string line;

    while (std::getline(inFile, line))
    {
        if (line.empty() || line[0] == 'c')
            continue;

        std::istringstream iss(line);

        if (line[0] == 'p')
        {
            // 匿名变量忽略cnf字符串
            std::string _;
            iss >> _ >> _ >> BoolCount >> ClauseCount;
            continue;
        }

        cnf_parser::clause clause;
        int literal;

        while (iss >> literal && literal != 0)
        {
            clause.literals.push_back(literal);
        }

        clauses.push_back(clause);
    }

    inFile.close();

    return true;
}

// 判断是否为单子句（只有一个文字）
bool cnf_parser::IsUnitClause(clause clause)
{
    return (clause.literals.size() == 1);
}

// 判断子句中是否含有某个文字
bool cnf_parser::HaveLiteral(clause clause, int literal)
{
    for (auto it_literal = clause.literals.begin(); it_literal != clause.literals.end(); ++it_literal)
    {
        if (*it_literal == literal)
            return true;
    }

    return false;
}
