#include "../include/dpll_solver.h"
#include "../include/cnf_parser.h"
#include "../include/utils.h"
#include "../include/list.h"
#include <iostream>
#include <functional>

using namespace std;

DeLinkList<cnf_parser::clause> clauses;
DPLLSolver dpll_solver(8, 8);
pair <bool, long long> result_pair;
long long runtime;
bool result;
std::vector<LiteralStatus> assignment;

void printMenu()
{
    cout << "1、读取cnf文件" << endl;
    cout << "2、基础SAT求解" << endl;
    cout << "3、优化SAT求解" << endl;
    cout << "4、输出res文件" << endl;
    cout << "0、结束" << endl;
    cout << "请输入你的选择: ";
}

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

        result_pair = MeasureTime(std::bind(&DPLLSolver::BasicSolve, &dpll_solver, std::placeholders::_1, std::placeholders::_2), clauses, BoolCount);
        result = result_pair.first;
        runtime = result_pair.second;

        if (result) cout << "有解" << endl;
        else  cout << "无解" << endl;

        clauses.clear();

        break;

    case 3:
        if (clauses.empty())
        {
            cout << "请先读入cnf文件" << endl;
            break;
        }

        result_pair = MeasureTime(std::bind(&DPLLSolver::OptimizedSolve, &dpll_solver, std::placeholders::_1, std::placeholders::_2), clauses, BoolCount);
        result = result_pair.first;
        runtime = result_pair.second;

        clauses.clear();

        break;

    case 4:
        cout << "请输入输出文件路径" << endl;
        cin >> filename; 
        assignment = dpll_solver.GetResult();
        parser.WriteRes(assignment, filename, result, runtime);
        break;

    default:
        break;
    }
}