#include "../include/dpll_solver.h"
#include "../include/cnf_parser.h"
#include "../include/utils.h"
#include "../include/list.h"
#include "../include/hanidoku_solver.h"
#include <iostream>
#include <functional>

using namespace std;

DeLinkList<cnf_parser::clause> clauses;
DPLLSolver dpll_solver;
pair <bool, long long> result_pair;
long long runtime;
bool result;
std::vector<LiteralStatus> assignment;
std::vector<size_t> left_nums;

void printMenu()
{
    cout << "1、读取cnf文件" << endl;
    cout << "2、基础SAT求解" << endl;
    cout << "3、优化SAT求解1" << endl;
    cout << "4、优化SAT求解2" << endl;
    cout << "5、优化SAT求解3" << endl;
    cout << "6、输出res文件" << endl;
    cout << "7、生成蜂窝数独" << endl;
    cout << "8、求解已有的蜂窝数独" << endl;
    cout << "9、蜂窝数独游戏" << endl;
    cout << "0、结束" << endl;
    cout << "请输入你的选择: ";
}

void display(int op)
{
    extern int BoolCount;
    int count, blank_num, choice;
    string filename;
    cnf_parser parser;

    switch (op)
    {
    case 1:
        count = 0;
        do
        {
            count++;
            cout << "请输入文件路径: ";
            cin >> filename;
        } while (!parser.ReadCNFFile(filename, clauses) && count < 5);

        if(count == 5) cout << "文件读取失败次数过多，请重新选择操作" << endl;

        // 读取cnf文件成功
        std::cout << "成功读取cnf文件" << std::endl;
        
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

        result_pair = MeasureTime(std::bind(&DPLLSolver::OptimizedSolve1, &dpll_solver, std::placeholders::_1, std::placeholders::_2), clauses, BoolCount);
        result = result_pair.first;
        runtime = result_pair.second;

        if (result) cout << "有解" << endl;
        else  cout << "无解" << endl;

        clauses.clear();

        break;

    case 4:
        if (clauses.empty())
        {
            cout << "请先读入cnf文件" << endl;
            break;
        }

        result_pair = MeasureTime(std::bind(&DPLLSolver::OptimizedSolve2, &dpll_solver, std::placeholders::_1, std::placeholders::_2), clauses, BoolCount);
        result = result_pair.first;
        runtime = result_pair.second;

        if (result) cout << "有解" << endl;
        else  cout << "无解" << endl;

        clauses.clear();

        break;

    case 5:
        // if (clauses.empty())
        // {
        //     cout << "请先读入cnf文件" << endl;
        //     break;
        // }

        // result_pair = MeasureTime(std::bind(&DPLLSolver::OptimizedSolve3, &dpll_solver, std::placeholders::_1, std::placeholders::_2), clauses, BoolCount);
        // result = result_pair.first;
        // runtime = result_pair.second;

        // if (result) cout << "有解" << endl;
        // else  cout << "无解" << endl;

        // clauses.clear();

        break;

    case 6:
        cout << "请输入输出文件路径: ";
        cin >> filename; 
        assignment = dpll_solver.GetAssignment();
        left_nums = dpll_solver.GetLeftNums();
        parser.WriteRes(assignment, filename, result, runtime, left_nums);
        
        break;

    case 7:
        {
            cout << "请输入空格数量：";
            cin >> blank_num;

            Hanidoku hanidoku;
            hanidoku.generate_Hanidoku(blank_num);

            cout << endl;

            cout << "1、求解该数独  2、返回:"; 
            cin >> choice;

            if (choice == 1) hanidoku.solve();
        }

        cout << endl;

        break;

    case 8:
        {
            cout << "请输入数独文件路径：";
            cin >> filename;

            Hanidoku hanidoku;
            hanidoku.solve(filename);

            cout << endl;
        }

        cout << endl;

        break;

    case 9:
        {
            HexagonalSudokuGame game(1300, 1300);
            game.run();
        }

        break;

    default:
        break;
    }
}