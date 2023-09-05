#ifndef HANIDOKU_SOLVER_H
#define HANIDOKU_SOLVER_H

#include <iostream>
#include <vector>
#include <random>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <ctime>
#include <cstdlib>
#include "list.h"
#include "cnf_parser.h"
#include "dpll_solver.h"
#include <SFML/Graphics.hpp>
#include <vector>

class Hanidoku
{
public:
    Hanidoku()
    {
        InitializeClauses();

        cnf_parser parser;

        parser.ReadCNFFile(this->filename, this->clauses);
    }

    // 生成数独
    void generate_Hanidoku(int blank_num)
    {
        // 随机化种子
        srand((int)time(0));

        // 获取游戏终盘
        int game_index = rand() % 20;
        std::string game_data = ending_game_data[game_index];

        // 挖洞生成唯一解的蜂窝数独
        for (int blank_count = 1; blank_count <= blank_num; ++blank_count)
        {
            // 随机一个挖洞的位置
            int pos = -1;
            do
            {
                pos = rand() % 61;
            } while (game_data[pos] == '0');

            // 拷贝游戏终盘副本
            std::string game_data_temp = game_data;

            // 挖去该洞以后解的数量
            int sum_answer = 0;

            for (int k = 1; k <= 9; ++k)
            {
                game_data_temp[pos] = '0' + k;
                if (Justify_solve(game_data_temp))
                    ++sum_answer;
                if (sum_answer > 1)
                    break;
            }

            if (sum_answer == 1)
            {
                game_data[pos] = '0';
                std::cout << "挖好第 " << blank_count << " 个洞" << std::endl;
            }
            else
            {
                std::cout << "回退一次" << std::endl;
                blank_count--;
            }
        }

        // 记录该数独并打印
        int game[62] = {0};

        for (int pos = 0; pos < 61; ++pos)
        {
            game[pos + 1] = game_data[pos] - '0';

            if (game_data[pos] == '0')
                continue;

            cnf_parser::clause clause_temp;
            clause_temp.literals.push_back(game_data[pos] - '0' + pos * 9);
            this->clauses.push_front(clause_temp);
        }

        WriteCnfFile(this->filename, this->clauses);

        show(game);
    }

    // 求解数独
    void solve()
    {
        extern DPLLSolver dpll_solver;

        dpll_solver.BasicSolve(this->clauses, 549);

        std::vector<LiteralStatus> temp = dpll_solver.GetAssignment();

        int solution[62] = {0};

        for (int pos = 0; pos < 61; ++pos)
        {
            for (int k = 1; k <= 9; ++k)
            {
                if (temp[pos * 9 + k] == LiteralStatus::True)
                    solution[pos + 1] = k;
            }
        }

        show(solution);
    }

    void solve(const std::string filename)
    {
        std::ifstream inFile(filename);

        if (!inFile)
        {
            // 处理打开失败的文件
            std::cerr << "文件打开失败，请重新读取" << std::endl;
            return;
        }

        std::string line;

        while (std::getline(inFile, line))
        {
            line.erase(0, 5);

            // 记录读到的数独并打印
            int game[62] = {0};
            DeLinkList<cnf_parser::clause> clauses_temp = this->clauses;

            for (int pos = 0; pos < 61; ++pos)
            {
                game[pos + 1] = line[pos] - '0';

                if (line[pos] == '0')
                    continue;

                cnf_parser::clause clause_temp;
                clause_temp.literals.push_back(line[pos] - '0' + pos * 9);
                clauses_temp.push_front(clause_temp);
            }

            std::cout << "数独：" << std::endl;

            show(game);

            extern DPLLSolver dpll_solver;

            if (!dpll_solver.OptimizedSolve1(clauses_temp, 549))
            {
                std::cout << "无解" << std::endl;
                continue;
            }

            std::vector<LiteralStatus> temp = dpll_solver.GetAssignment();

            int solution[62] = {0};

            for (int pos = 0; pos < 61; ++pos)
            {
                for (int k = 1; k <= 9; ++k)
                {
                    if (temp[pos * 9 + k] == LiteralStatus::True)
                        solution[pos + 1] = k;
                }
            }

            std::cout << "解：" << std::endl;

            show(solution);
        }
    }

    bool solve(std::vector<std::vector<int>> &hanidoku)
    {
        int pos = -1;
        for (auto row : hanidoku)
        {
            for (auto literal : row)
            {
                ++pos;
                if (literal == 0)
                    continue;
                cnf_parser::clause temp;
                temp.literals.push_back(pos * 9 + literal);
                this->clauses.push_front(temp);
            }
        }

        WriteCnfFile(this->filename, this->clauses);

        extern DPLLSolver dpll_solver;

        bool flag = dpll_solver.BasicSolve(this->clauses, 549);

        std::vector<LiteralStatus> temp = dpll_solver.GetAssignment();

        int solution[62] = {0};

        for (pos = 0; pos < 61; ++pos)
        {
            for (int k = 1; k <= 9; ++k)
            {
                if (temp[pos * 9 + k] == LiteralStatus::True)
                    solution[pos + 1] = k;
            }
        }

        pos = 1;

        for (int row = 0; row < 9; ++row)
        {
            for (int col = 0; col < rowSizes_[row]; ++col)
            {
                hanidoku[row][col] = solution[pos++];
            }
        }

        return flag;
    }

private:
    DeLinkList<cnf_parser::clause> clauses;
    std::string filename;
    int index[9] = {0, 5, 11, 18, 26, 35, 43, 50, 56};
    const std::vector<int> rowSizes_ = {5, 6, 7, 8, 9, 8, 7, 6, 5}; // 每行的格子数
    std::string ending_game_data[20] = {"4563263847554721632638975437915428625789634463217554876363452",
                                        "4536232457667984532367984556813792445281673745283631456263745",
                                        "7856963748554123768635124795276413874128365634125787354656789",
                                        "5364246723575984636237985434598271678361452451726367483553624",
                                        "3647567853474521634536928759218374687321465684725346859775684",
                                        "5436263724575986343247985646598271378431652651723437684554326",
                                        "3574676458364132754352186758217639476839245375248669485745768",
                                        "5634248765335124762769853464273918535892476436875251426376534",
                                        "3675461543275621435428937643192768524379856376854268547356234",
                                        "3675454231661572434361852772489163551237486367285445876363542",
                                        "4632558476337152466529783424683951737492865531672468745345236",
                                        "5426342531626317546317842531659284754821763724863545738663574",
                                        "7436563584747126535829347636758129454312678672348549685778546",
                                        "2435653641262785433589276446978123574396582524317667845335764",
                                        "5236464573247986533542987626831794537986452657983442357654623",
                                        "6845754679887935644582137676398214537461528582437663725445637",
                                        "5426376354263721544251873637842961556398742487695375483663524",
                                        "5637461542374631524389726535928174627594863486732563857456432",
                                        "5632438465747251366387241524519876357861324637241548653735246",
                                        "2436557624363451274581273636278951427396845587493643865765423"};

    // 求解数独
    bool Justify_solve(std::string game_data)
    {
        DeLinkList<cnf_parser::clause> clauses_temp = this->clauses;

        extern DPLLSolver dpll_solver;

        for (int pos = 0; pos < 61; ++pos)
        {
            if (game_data[pos] == '0')
                continue;

            cnf_parser::clause clause_temp;
            clause_temp.literals.push_back(game_data[pos] - '0' + pos * 9);
            clauses_temp.push_front(clause_temp);
        }

        return dpll_solver.BasicSolve(clauses_temp, 549);
    }

    // 生成行、对角线约束
    void generate_clause(std::string filename, std::vector<int> positions, int grid_size)
    {
        std::ofstream outFile(filename, std::ios::app); // 打开文件并且追加内容

        if (!outFile)
        {
            // 处理打开失败的文件
            std::cerr << "文件打开失败,请重新读取" << std::endl;
            return;
        }

        switch (grid_size)
        {
        case 5:
            // 必填数字
            for (int i = 5; i <= 5; ++i)
            {
                for (const auto position : positions)
                {
                    int literal = position * 10 + i;
                    literal = transmission(literal);
                    outFile << literal << " ";
                }
                outFile << "0" << std::endl;
            }

            break;

        case 6:
            // 必填数字
            for (int i = 4; i <= 6; ++i)
            {
                for (const auto position : positions)
                {
                    int literal = position * 10 + i;
                    literal = transmission(literal);
                    outFile << literal << " ";
                }
                outFile << "0" << std::endl;
            }

            break;

        case 7:
            // 必填数字
            for (int i = 3; i <= 7; ++i)
            {
                for (const auto position : positions)
                {
                    int literal = position * 10 + i;
                    literal = transmission(literal);
                    outFile << literal << " ";
                }
                outFile << "0" << std::endl;
            }

            break;

        case 8:
            // 必填数字
            for (int i = 2; i <= 8; ++i)
            {
                for (const auto position : positions)
                {
                    int literal = position * 10 + i;
                    literal = transmission(literal);
                    outFile << literal << " ";
                }
                outFile << "0" << std::endl;
            }

            break;

        case 9:
            // 必填数字
            for (int i = 1; i <= 9; ++i)
            {
                for (const auto position : positions)
                {
                    int literal = position * 10 + i;
                    literal = transmission(literal);
                    outFile << literal << " ";
                }
                outFile << "0" << std::endl;
            }

            break;

        default:
            break;
        }

        // 选填数字
        for (int i = grid_size + 1; i <= 9; ++i)
        {
            for (int j = i - grid_size; j <= 9 - grid_size; ++j)
            {
                for (const auto position : positions)
                {
                    int literal = position * 10 + i;
                    literal = transmission(literal);
                    outFile << literal << " ";
                }

                for (const auto position : positions)
                {
                    int literal = position * 10 + j;
                    literal = transmission(literal);
                    outFile << literal << " ";
                }

                outFile << "0" << std::endl;
            }
        }

        // 任意两个格子不可以填写相同的数字
        for (int i = 1; i <= 9; ++i)
        {
            for (int index1 = 0; index1 < grid_size; ++index1)
            {
                for (int index2 = index1 + 1; index2 < grid_size; ++index2)
                {
                    int literal1 = positions[index1] * 10 + i;
                    int literal2 = positions[index2] * 10 + i;
                    literal1 = transmission(literal1);
                    literal2 = transmission(literal2);
                    outFile << -literal1 << " " << -literal2 << " 0" << std::endl;
                }
            }
        }

        outFile.close();
    }

    // 初始化数独规则
    void InitializeClauses()
    {
        std::cout << "请输入生成的cnf文件路径: ";
        std::cin >> this->filename;

        std::ofstream outFile(filename);

        if (!outFile)
        {
            // 处理打开失败的文件
            std::cerr << "文件打开失败,请重新读取" << std::endl;
            return;
        }

        outFile << "p cnf " << 549 << " " << 7468 << std::endl;

        // 格约束
        for (int i = 0; i < 61; ++i)
        {
            for (int k = 1; k <= 9; ++k)
            {
                outFile << i * 9 + k << " ";
            }

            outFile << "0" << std::endl;

            for (int k1 = 1; k1 <= 9; ++k1)
            {
                for (int k2 = k1 + 1; k2 <= 9; ++k2)
                {
                    outFile << -(i * 9 + k1) << " " << -(i * 9 + k2) << " 0" << std::endl;
                }
            }
        }

        outFile.close();

        // 5 格的规则
        generate_clause(filename, std::vector<int>({11, 12, 13, 14, 15}), 5);
        generate_clause(filename, std::vector<int>({11, 21, 31, 41, 51}), 5);
        generate_clause(filename, std::vector<int>({51, 61, 71, 81, 91}), 5);
        generate_clause(filename, std::vector<int>({91, 92, 93, 94, 95}), 5);
        generate_clause(filename, std::vector<int>({59, 68, 77, 86, 95}), 5);
        generate_clause(filename, std::vector<int>({15, 26, 37, 48, 59}), 5);

        // 6 格的规则
        generate_clause(filename, std::vector<int>({21, 22, 23, 24, 25, 26}), 6);
        generate_clause(filename, std::vector<int>({12, 22, 32, 42, 52, 61}), 6);
        generate_clause(filename, std::vector<int>({41, 52, 62, 72, 82, 92}), 6);
        generate_clause(filename, std::vector<int>({81, 82, 83, 84, 85, 86}), 6);
        generate_clause(filename, std::vector<int>({48, 58, 67, 76, 85, 94}), 6);
        generate_clause(filename, std::vector<int>({14, 25, 36, 47, 58, 68}), 6);

        // 7 格的规则
        generate_clause(filename, std::vector<int>({31, 32, 33, 34, 35, 36, 37}), 7);
        generate_clause(filename, std::vector<int>({13, 23, 33, 43, 53, 62, 71}), 7);
        generate_clause(filename, std::vector<int>({31, 42, 53, 63, 73, 83, 93}), 7);
        generate_clause(filename, std::vector<int>({71, 72, 73, 74, 75, 76, 77}), 7);
        generate_clause(filename, std::vector<int>({37, 47, 57, 66, 75, 84, 93}), 7);
        generate_clause(filename, std::vector<int>({13, 24, 35, 46, 57, 67, 77}), 7);

        // 8 格的规则
        generate_clause(filename, std::vector<int>({41, 42, 43, 44, 45, 46, 47, 48}), 8);
        generate_clause(filename, std::vector<int>({14, 24, 34, 44, 54, 63, 72, 81}), 8);
        generate_clause(filename, std::vector<int>({21, 32, 43, 54, 64, 74, 84, 94}), 8);
        generate_clause(filename, std::vector<int>({61, 62, 63, 64, 65, 66, 67, 68}), 8);
        generate_clause(filename, std::vector<int>({26, 36, 46, 56, 65, 74, 83, 92}), 8);
        generate_clause(filename, std::vector<int>({12, 23, 34, 45, 56, 66, 76, 86}), 8);

        // 9 格的规则
        generate_clause(filename, std::vector<int>({51, 52, 53, 54, 55, 56, 57, 58, 59}), 9);
        generate_clause(filename, std::vector<int>({15, 25, 35, 45, 55, 64, 73, 82, 91}), 9);
        generate_clause(filename, std::vector<int>({11, 22, 33, 44, 55, 65, 75, 85, 95}), 9);
    }

    // 变元映射
    int transmission(int literal)
    {
        int bool_variation = 0;
        int x = literal % 10;

        bool_variation += x;

        literal = literal / 10;
        x = literal % 10;

        bool_variation += (x - 1) * 9;

        x = literal / 10;

        bool_variation += index[x - 1] * 9;

        return bool_variation;
    }

    // 输出cnf文件
    void WriteCnfFile(std::string filename, DeLinkList<cnf_parser::clause> clauses)
    {
        std::ofstream outFile(filename);

        if (!outFile)
        {
            // 处理打开失败的文件
            std::cerr << "文件打开失败,请重新读取" << std::endl;
            return;
        }

        outFile << "p cnf " << 549 << " " << clauses.size() << std::endl;

        for (auto it_clause = clauses.begin(); it_clause != clauses.end(); ++it_clause)
        {
            for (auto it_literal = (*it_clause).literals.begin(); it_literal != (*it_clause).literals.end(); ++it_literal)
            {
                outFile << *it_literal << " ";
            }
            outFile << "0" << std::endl;
        }

        outFile.close();
    }

    // 打印数独
    void show(int solution[])
    {
        printf("         / \\ / \\ / \\ / \\ / \\  \n");
        printf("        | %d | %d | %d | %d | %d |  \n", solution[1], solution[2], solution[3], solution[4], solution[5]);
        printf("       / \\ / \\ / \\ / \\ / \\ / \\  \n");
        printf("      | %d | %d | %d | %d | %d | %d |  \n", solution[6], solution[7], solution[8], solution[9], solution[10], solution[11]);
        printf("     / \\ / \\ / \\ / \\ / \\ / \\ / \\  \n");
        printf("    | %d | %d | %d | %d | %d | %d | %d |  \n", solution[12], solution[13], solution[14], solution[15], solution[16], solution[17], solution[18]);
        printf("   / \\ / \\ / \\ / \\ / \\ / \\ / \\ / \\  \n");
        printf("  | %d | %d | %d | %d | %d | %d | %d | %d |  \n", solution[19], solution[20], solution[21], solution[22], solution[23], solution[24], solution[25], solution[26]);
        printf(" / \\ / \\ / \\ / \\ / \\ / \\ / \\ / \\ / \\  \n");
        printf("| %d | %d | %d | %d | %d | %d | %d | %d | %d |  \n", solution[27], solution[28], solution[29], solution[30], solution[31], solution[32], solution[33], solution[34], solution[35]);
        printf(" \\ / \\ / \\ / \\ / \\ / \\ / \\ / \\ / \\ /  \n");
        printf("  | %d | %d | %d | %d | %d | %d | %d | %d |  \n", solution[36], solution[37], solution[38], solution[39], solution[40], solution[41], solution[42], solution[43]);
        printf("   \\ / \\ / \\ / \\ / \\ / \\ / \\ / \\ /  \n");
        printf("    | %d | %d | %d | %d | %d | %d | %d |  \n", solution[44], solution[45], solution[46], solution[47], solution[48], solution[49], solution[50]);
        printf("     \\ / \\ / \\ / \\ / \\ / \\ / \\ /  \n");
        printf("      | %d | %d | %d | %d | %d | %d |  \n", solution[51], solution[52], solution[53], solution[54], solution[55], solution[56]);
        printf("       \\ / \\ / \\ / \\ / \\ / \\ /  \n");
        printf("        | %d | %d | %d | %d | %d |  \n", solution[57], solution[58], solution[59], solution[60], solution[61]);
        printf("         \\ / \\ / \\ / \\ / \\ /  \n");
    }
};

class HexagonalSudokuGame
{
public:
    HexagonalSudokuGame(int windowWidth, int windowHeight)
        : windowWidth_(windowWidth), windowHeight_(windowHeight), sudokuBoard_(9, std::vector<int>())
    {
        // 创建SFML窗口
        window_.create(sf::VideoMode(windowWidth_, windowHeight_), "Hexagonal Sudoku");

        // 初始化数独板
        for (int row = 0; row < numRows_; ++row)
        {
            for (int col = 0; col < rowSizes_[row]; ++col)
            {
                sudokuBoard_[row].push_back(0);
            }
        }

        // 加载字体
        if (!font_.loadFromFile("D:\\font\\NotoSansSC-VariableFont_wght.ttf"))
        {
            std::cerr << "Error loading font!" << std::endl;
            exit(1);
        }

        // 初始化行、列、值输入框
        rowInputBox_.setSize(sf::Vector2f(100, 50));
        rowInputBox_.setFillColor(sf::Color::White);
        rowInputBox_.setOutlineThickness(2);
        rowInputBox_.setOutlineColor(sf::Color::Black);
        rowInputBox_.setPosition({100.0, windowHeight_ - 140.0});

        colInputBox_.setSize(sf::Vector2f(100, 50));
        colInputBox_.setFillColor(sf::Color::White);
        colInputBox_.setOutlineThickness(2);
        colInputBox_.setOutlineColor(sf::Color::Black);
        colInputBox_.setPosition({220.0, windowHeight_ - 140.0});

        valueInputBox_.setSize(sf::Vector2f(100, 50));
        valueInputBox_.setFillColor(sf::Color::White);
        valueInputBox_.setOutlineThickness(2);
        valueInputBox_.setOutlineColor(sf::Color::Black);
        valueInputBox_.setPosition({340.0, windowHeight_ - 140.0});

        // 创建求解按钮
        solveButton_.setSize(sf::Vector2f(90, 40));
        solveButton_.setFillColor(sf::Color::Green);
        solveButton_.setPosition({500.0, windowHeight_ - 140.0});

        // 创建生成按钮
        generateButton_.setSize(sf::Vector2f(100, 40));
        generateButton_.setFillColor(sf::Color::Blue);
        generateButton_.setPosition({windowWidth_ - 200.0, 300.0});

        solving_ = false; // 初始状态下未求解数独
    }

    void run()
    {
        while (window_.isOpen())
        {
            drawBoard();
            handleEvents();
        }
    }

private:
    int windowWidth_;
    int windowHeight_;
    sf::RenderWindow window_;
    sf::Font font_;
    sf::RectangleShape rowInputBox_;
    sf::RectangleShape colInputBox_;
    sf::RectangleShape valueInputBox_;
    sf::RectangleShape solveButton_;
    sf::RectangleShape generateButton_;
    bool solving_ = false; // 标志位，用于指示是否正在求解数独
    std::string currentRowInput_;
    std::string currentColInput_;
    std::string currentValueInput_;

    const int numRows_ = 9;                                         // 9行
    const std::vector<int> rowSizes_ = {5, 6, 7, 8, 9, 8, 7, 6, 5}; // 每行的格子数
    std::vector<std::vector<int>> sudokuBoard_;
    std::string filename_;

    enum class InputField
    {
        Row,
        Col,
        Value
    };

    InputField currentInputField_ = InputField::Row;

    // 绘制 canva
    void drawBoard()
    {
        window_.clear(sf::Color::White);

        // 定义六边形的宽度和高度
        float hexagonWidth = 80.0f;
        float hexagonHeight = 100.0f;

        // 绘制蜂窝数独板
        for (int row = 0; row < numRows_; ++row)
        {
            int start_x = std::abs(4 - row) * hexagonWidth / 2 + 40.0f;
            int start_y = row * 3 * hexagonHeight / 4 + 150.0f;

            for (int col = 0; col < rowSizes_[row]; ++col)
            {
                sf::ConvexShape hexagon;
                hexagon.setPointCount(6);

                // 计算蜂窝的位置，以六边形的中心为旋转点
                float x = start_x + col * hexagonWidth;
                float y = start_y;

                // 设置蜂窝的顶点
                hexagon.setPoint(0, sf::Vector2f(x + hexagonWidth / 2, y));
                hexagon.setPoint(1, sf::Vector2f(x + hexagonWidth, y + hexagonHeight / 4));
                hexagon.setPoint(2, sf::Vector2f(x + hexagonWidth, y + 3 * hexagonHeight / 4));
                hexagon.setPoint(3, sf::Vector2f(x + hexagonWidth / 2, y + hexagonHeight));
                hexagon.setPoint(4, sf::Vector2f(x, y + 3 * hexagonHeight / 4));
                hexagon.setPoint(5, sf::Vector2f(x, y + hexagonHeight / 4));

                // 设置蜂窝的边框
                hexagon.setOutlineThickness(2.0f);
                hexagon.setOutlineColor(sf::Color::Black);

                window_.draw(hexagon);

                // 创建文本输入框
                sf::Text inputText(std::to_string(sudokuBoard_[row][col]), font_, 30.0f);
                inputText.setFillColor(sf::Color::Black);

                // 设置文本框位置，这里可以根据需要微调
                float textX = x + hexagonWidth / 2 - 10.0f;
                float textY = y + hexagonHeight / 2 - 10.0f;

                inputText.setPosition({textX, textY});

                window_.draw(inputText);
            }
        }

        // 绘制行、列、值输入框和标签
        window_.draw(rowInputBox_);
        window_.draw(colInputBox_);
        window_.draw(valueInputBox_);

        sf::Text rowLabel("Row:", font_, 30);
        rowLabel.setFillColor(sf::Color::Black);
        rowLabel.setPosition({100.0, windowHeight_ - 200.0});
        window_.draw(rowLabel);

        sf::Text colLabel("Col:", font_, 30);
        colLabel.setFillColor(sf::Color::Black);
        colLabel.setPosition({220.0, windowHeight_ - 200.0});
        window_.draw(colLabel);

        sf::Text valueLabel("Value:", font_, 30);
        valueLabel.setFillColor(sf::Color::Black);
        valueLabel.setPosition({340.0, windowHeight_ - 200.0});
        window_.draw(valueLabel);

        // 绘制输入框中的文本
        sf::Text rowInputText(currentRowInput_, font_, 30);
        rowInputText.setFillColor(sf::Color::Black);
        rowInputText.setPosition({120.0, windowHeight_ - 145});
        window_.draw(rowInputText);

        sf::Text colInputText(currentColInput_, font_, 30);
        colInputText.setFillColor(sf::Color::Black);
        colInputText.setPosition({230.0, windowHeight_ - 145});
        window_.draw(colInputText);

        sf::Text valueInputText(currentValueInput_, font_, 30);
        valueInputText.setFillColor(sf::Color::Black);
        valueInputText.setPosition({350.0, windowHeight_ - 145});
        window_.draw(valueInputText);

        // 绘制焦点框
        sf::RectangleShape focusRect;
        focusRect.setSize(sf::Vector2f(100, 50));
        focusRect.setFillColor(sf::Color::Transparent);
        focusRect.setOutlineThickness(2);
        focusRect.setOutlineColor(sf::Color::Red);

        if (currentInputField_ == InputField::Row)
        {
            focusRect.setPosition({100.0, windowHeight_ - 140.0});
        }
        else if (currentInputField_ == InputField::Col)
        {
            focusRect.setPosition({220.0, windowHeight_ - 140.0});
        }
        else if (currentInputField_ == InputField::Value)
        {
            focusRect.setPosition({340.0, windowHeight_ - 140.0});
        }

        window_.draw(focusRect);

        // 求解按钮
        if (solving_)
        {
            sf::Text solvingText("Solving...", font_, 25);
            solvingText.setFillColor(sf::Color::Black);
            solvingText.setPosition({510.0, windowHeight_ - 140});
            window_.draw(solvingText);
        }
        else
        {
            sf::Text solveText("Solve", font_, 25);
            solveText.setFillColor(sf::Color::Black);
            solveText.setPosition({510.0, windowHeight_ - 140});
            window_.draw(solveButton_);
            window_.draw(solveText);
        }

        // 生成按钮
        sf::Text generateText("Generate", font_, 20);
        generateText.setFillColor(sf::Color::White);
        generateText.setPosition({windowWidth_ - 200.0, 300.0});
        window_.draw(generateButton_);
        window_.draw(generateText);

        window_.display();
    }

    // 处理事件
    void handleEvents()
    {
        sf::Event event;
        while (window_.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window_.close();
            else if (event.type == sf::Event::TextEntered)
            {
                if (event.text.unicode < 128 && event.text.unicode != 8) // 8 is backspace
                {
                    if (currentInputField_ == InputField::Row)
                    {
                        currentRowInput_ += static_cast<char>(event.text.unicode);
                    }
                    else if (currentInputField_ == InputField::Col)
                    {
                        currentColInput_ += static_cast<char>(event.text.unicode);
                    }
                    else if (currentInputField_ == InputField::Value)
                    {
                        currentValueInput_ += static_cast<char>(event.text.unicode);
                    }
                }
                else if (event.text.unicode == 8)
                {
                    if (currentInputField_ == InputField::Row && !currentRowInput_.empty())
                    {
                        currentRowInput_.pop_back(); // Handle backspace for row input
                    }
                    else if (currentInputField_ == InputField::Col && !currentColInput_.empty())
                    {
                        currentColInput_.pop_back(); // Handle backspace for col input
                    }
                    else if (currentInputField_ == InputField::Value && !currentValueInput_.empty())
                    {
                        currentValueInput_.pop_back(); // Handle backspace for value input
                    }
                }
            }
            else if (event.type == sf::Event::KeyPressed)
            {
                if (event.key.code == sf::Keyboard::Tab)
                {
                    // Switch between input fields (Row -> Col -> Value)
                    switch (currentInputField_)
                    {
                    case InputField::Row:
                        currentInputField_ = InputField::Col;
                        break;
                    case InputField::Col:
                        currentInputField_ = InputField::Value;
                        break;
                    case InputField::Value:
                        currentInputField_ = InputField::Row;
                        break;
                    }
                }
                else if (event.key.code == sf::Keyboard::Enter)
                {
                    // Handle Enter key press to update the Sudoku board
                    if (!currentRowInput_.empty() && !currentColInput_.empty() && !currentValueInput_.empty())
                    {
                        int row = std::stoi(currentRowInput_);
                        int col = std::stoi(currentColInput_);
                        int value = std::stoi(currentValueInput_);

                        // Update the Sudoku board with the entered value
                        if (isValidInput(row, col, value))
                        {
                            sudokuBoard_[row - 1][col - 1] = value;
                        }
                        else
                            std::cerr << "请输入合法的行、列、值" << std::endl;

                        // Clear the input fields
                        currentRowInput_.clear();
                        currentColInput_.clear();
                        currentValueInput_.clear();
                    }
                }
            }
            else if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left)
            {
                sf::Vector2i mousePos = sf::Mouse::getPosition(window_);

                // 检查鼠标点击是否在按钮内
                if (solveButton_.getGlobalBounds().contains({static_cast<float>(mousePos.x), static_cast<float>(mousePos.y)}))
                {
                    // 触发求解操作
                    solving_ = true;

                    Hanidoku hanidoku;

                    // 调用 solve 函数来解决数独
                    if (hanidoku.solve(sudokuBoard_))
                        std::cout << "有解" << std::endl;
                    else
                        std::cout << "无解" << std::endl;

                    solving_ = false;
                }

                // 检查鼠标点击是否在生成按钮内
                if (generateButton_.getGlobalBounds().contains({static_cast<float>(mousePos.x), static_cast<float>(mousePos.y)}))
                {
                    // 触发生成数独操作
                    generateSudokuFromFile("D:\\code\\SAT_test\\easy_hanidoku.txt"); // 从文件中生成数独
                }
            }
        }
    }

    // 判断输入是否合法
    bool isValidInput(int row, int col, int value)
    {
        if (row < 1 || row > 9 || col < 1 || col > rowSizes_[row - 1])
            return false;
        if (value < 0 || value > 9)
            return false;

        return true;
    }

    // 随机选择文件中的一行
    std::string getRandomLine(const std::string &filename)
    {
        std::ifstream file(filename);
        if (!file.is_open())
        {
            std::cerr << "Failed to open file: " << filename << std::endl;
            return "";
        }

        // 存储文件中的每一行
        std::vector<std::string> lines;
        std::string line;
        while (std::getline(file, line))
        {
            lines.push_back(line);
        }

        // 关闭文件
        file.close();

        // 随机数生成
        std::srand(static_cast<unsigned int>(std::time(nullptr)));
        int randomIndex = std::rand() % lines.size();

        // 返回随机选择的行
        return lines[randomIndex];
    }

    void generateSudokuFromFile(const std::string &filename)
    {
        std::string line = getRandomLine(filename);

        // 删除前五个字符
        line.erase(0, 5);

        std::cout << line << std::endl;

        // 初始化数独板
        sudokuBoard_.clear();
        sudokuBoard_.resize(numRows_, std::vector<int>());
        int row = 0;
        int col = 0;

        // 将字符转换为数字并填充数独板
        for (char c : line)
        {
            if (std::isdigit(c))
            {
                sudokuBoard_[row].push_back(c - '0');
                col++;
                if (col == rowSizes_[row])
                {
                    row++;
                    col = 0;
                }
            }
        }

        if (row != numRows_)
        {
            std::cerr << "文件不满足初始化条件: " << filename << std::endl;
            return;
        }
    }
};

#endif