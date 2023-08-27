#include "../include/dpll_solver.h"
#include "../include/cnf_parser.h"
#include "../include/utils.h"
#include "../include/list.h"
#include <iostream>
#include <functional>

using namespace std;

DeLinkList<cnf_parser::clause> clauses;
DPLLSolver dpll_solver(8, 8);

void display(int op)
{
    extern int BoolCount;
    int count;
    string filename;
    cnf_parser parser;
    
    switch (op)
    {
    case 1:
        count = 0;
        do
        {
            count++;
            cout << "请输入文件路径:";
            cin >> filename;
        } while (!parser.ReadCNFFile(filename, clauses) && count < 5);

        if(count == 5) cout << "文件读取失败次数过多，请重新选择操作" << endl;
        break;
    
    case 2:
        if (clauses.empty())
        {
            cout << "请先读入cnf文件" << endl;
            break;
        }

        if (MeasureTime(std::bind(&DPLLSolver::BasicSolve, &dpll_solver, std::placeholders::_1, std::placeholders::_2), clauses, BoolCount)) cout << "有解" << endl;
        else cout << "无解" << endl;

        clauses.clear();

        break;

    case 3:
        if (clauses.empty())
        {
            cout << "请先读入cnf文件" << endl;
            break;
        }

        if (MeasureTime(std::bind(&DPLLSolver::OptimizedSolve, &dpll_solver, std::placeholders::_1, std::placeholders::_2), clauses, BoolCount)) cout << "有解" << endl;
        else cout << "无解" << endl;

        clauses.clear();

        break;

    default:
        break;
    }
}