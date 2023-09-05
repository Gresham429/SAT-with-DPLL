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

// 输出res文件
void cnf_parser::WriteRes(std::vector<LiteralStatus> assignemnt, std::string filename, bool flag, long long runtime, std::vector<size_t> left_nums)
{
    extern int BoolCount;
    std::ofstream outFile(filename);

    if (!outFile)
    {
        // 处理打开失败的文件
        std::cerr << "文件打开失败,请重新读取" << std::endl;
        return;
    }

    outFile.clear();

    if (!flag)
    {
        outFile << "s 0" << std::endl;
        outFile << "v" << std::endl;
        outFile << "t " << runtime << std::endl;
        outFile << "剩余未满足的子句数量: ";

        for (auto left_num : left_nums)
        {
            outFile << left_num << " ";
        }

        outFile.close();
        return;
    }

    outFile << "s 1" << std::endl;
    outFile << "v";

    for (int i = 1; i <= BoolCount; ++i)
    {
        if (assignemnt[i] == LiteralStatus::True)
        {
            outFile << " " << i;
        }
        else if (assignemnt[i] == LiteralStatus::False)
        {
            outFile << " " << -i;
        }
        else if (assignemnt[i] == LiteralStatus::Unassigned)
        {
            outFile << " " << i << "(Unassigned)";
        }
    }

    outFile << std::endl;

    outFile << "t " << runtime << std::endl;
    outFile << "剩余未满足的子句数量: ";

    for (auto left_num : left_nums)
    {
        outFile << left_num << " ";
    }

    outFile.close();
}
