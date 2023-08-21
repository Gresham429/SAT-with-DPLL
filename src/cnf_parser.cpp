#include "../include/cnf_parser.h"
#include <fstream>
#include <sstream>
#include <vector>

int BoolCount, ClauseCount;

//读取cnf文件
bool cnf_parser::ReadCNFFile(const std::string filename, std::vector<clause> &clauses)
{
    std::cout << filename << std::endl;
    std::ifstream inFile(filename);

    if (!inFile)
    {
        //处理打开失败的文件
        std::cerr << "文件打开失败,请重新读取" << std::endl;
        return false;
    }

    //读取cnf文件成功
    std::cout << "成功读取cnf文件" << std::endl;

    std::string line;

    while (std::getline(inFile, line))
    {
        if (line.empty() || line[0] == 'c')
            continue;

        std::istringstream iss(line);

        if (line[0] == 'p')
        {
            //匿名变量忽略cnf字符串
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

//判断是否为单子句（只有一个文字）
bool cnf_parser::IsUnitClause(clause clause)
{
    if (clause.literals.size() == 1) return true;
    
    return false;
}

//判断子句的真假
bool cnf_parser::EvaluateClause(clause clause, std::vector<LiteralStatus> assignment)
{
    for (auto literal : clause.literals)
    {
        if (literal > 0 && assignment[literal - 1] == LiteralStatus::True)
        {
            clause.satisfied_flag = true;
            break;
        }
        else if (literal < 0 && assignment[-literal - 1] == LiteralStatus::False)
        {
            clause.satisfied_flag = true;
            break;
        }
    }

    return clause.satisfied_flag;
}