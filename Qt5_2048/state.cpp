#include <cstring>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <limits>
#include <cfloat>
#include "State.h"
//#include <QDebug>

using std::size_t;

size_t state::totalObjects(0);

//  Класс эвристики, реализующий аналог градиентного поиска
heuristic::heuristic()
{
    heur = { 1,2,3,4,5,6,7,8,9,10,11,22,60,80,110,160 };
    params = { 1.0,1.0,1.0 };
    deltas = { 1,1,1 };
    scores = { 0,0,0 };
    oldScores = { 0,0,0 };

    current = 0;
    iterations = 0;
    //loadFromFile("default.heu");
}

void heuristic::addScore(double result)
{
    scores[current] += result;
    iterations++;
}

bool heuristic::loadFromFile(std::string filename)
{
    std::ifstream inFile(filename);
    if (!inFile) return false;
    params.clear();
    double tmp(0);
    while (inFile >> tmp) params.push_back(tmp);
    inFile.close();
    return true;
}

bool heuristic::saveToFile(std::string filename)
{
    std::ofstream outFile(filename);
    if (!outFile) return false;
    for (int i = 0; i < params.size(); ++i)
        outFile << params[i] << std::endl;
    outFile.close();
    return true;
}

bool heuristic::nextParam()
{
    if (iterations>0)
        scores[current] /= iterations;
    else
        scores[current] = oldScores[current];
    iterations = 0;
    ++current;

    if (current < params.size()) return false;

    std::cout << "\nHeuristics modified : \n";
    std::cout << "Old scores : ";
    for (int i = 0; i < params.size(); ++i)
        std::cout << "  " << oldScores[i];

    //  Сначала обновляем дельты
    for (int i = 0; i < params.size(); ++i)
    {
        if (scores[i] > oldScores[i])
        {
            //  Если ОК - дельту увеличиваем
            deltas[i] *= 1.1;
            oldScores[i] = scores[i];
        }
        else
        {
            //  Если нет - откатываем, дельту уменьшаем
            params[i] /= deltas[i];  //  Откат
            deltas[i] *= 0.9;
        }
    }
    //  Сохраняем итерацию в файл
    saveToFile("default.heu");

    std::cout << "\nNew scores : ";
    for (int i = 0; i < params.size(); ++i)
        std::cout << "  " << scores[i];

    std::cout << "\nParams : ";
    for (int i = 0; i < params.size(); ++i)
        std::cout << "  " << params[i];
    std::cout << std::endl;

    for (int i = 0; i < params.size(); ++i) {
        params[i] *= deltas[i];
        scores[i] = 0;
    }

    current = 0;
    return true;
}

//  Вычисление эвристики - основной метод
double heuristic::calcHeuristics(const state & field) const
{
    state::dskType field2;
    state::copyField(field2, field.field);

    //  Меняем порядок клеток - для линейного произведения
    //  это можно оптимизировать, просто изменив вектор эвристики
    std::swap(field2[0], field2[3]);
    std::swap(field2[1], field2[2]);
    std::swap(field2[8], field2[11]);
    std::swap(field2[9], field2[10]);

    double rez = 1;
    for (size_t i = 0; i < cellCount; ++i) rez += (1 << field2[i]) * heur[i];

    //  Теперь находим максимум, и считаем количество плиток до max
    //  а также число свободных клеток
    int maxInd = 0, maxVal = field2[0];
    int freeCells = field2[0]==0 ? 1 : 0;
    int deltaCells = 0;
    for (int i = 1; i<cellCount; ++i) {
        if(field2[i-1] > field2[i] && field2[i - 1]>5)  deltaCells += (field2[i-1] - field2[i])*field2[i-1];
        if (field2[i] == 0) ++freeCells;
        else
            if (field2[i]>maxVal) {
                maxVal = field2[i];
                maxInd = i;
            }
    }
    //if (freeCells == 0) return 0;

    if (field2[maxInd] < 8) maxInd = cellCount - 1;

    //return rez - 200 * double(cellCount - maxInd - 1) + 11* freeCells - 0.1 * deltaCells;  Best at now!!!  58544
    return rez - 6000 * double(cellCount - maxInd - 1) + 11 * freeCells;// -1 * deltaCells;
    //return rez*double(1+params[0]*freeCells) / (1 + params[1]*deltaCells) / (1 + 100*params[2] * (double(cellCount - maxInd + 1)));
}

state::state() {
    for (size_t i = 0; i < cellCount; ++i) field[i] = 0;
    totalObjects++;
}

state::state(const state & other) {
    copyField(field, other.field);
    totalObjects++;
}

state::state(const char * data, int length)
{
	if (length > cellCount) length = cellCount;

	for (size_t i = 0; i < length; ++i)
		field[i] = data[i];

	totalObjects++;
}

//  Экспорт поля в буфер
void state::exportField(char * data, int length) const
{
	if (length<cellCount) throw std::invalid_argument("state::exportField receive buffer too small");
	for (size_t i = 0; i < cellCount; ++i)
		data[i] = field[i];
}

const state & state::operator=(const state & other) {
    copyField(field, other.field);
    return *this;
}

bool state::move(moves dir)
{
    // Ну тут же без извращения никак нельзя - это ж C++. А без комментариев - можно
    bool moved(false);
    int diff(0),base(0),step(0);
    switch (dir) {
        case moves::left : diff = +1; base = 0; step = fieldSize; break;
        case moves::up : diff = fieldSize; base = 0; step = 1; break;
        case moves::right : diff = -1; base = cellCount-1; step = -fieldSize; break;
        case moves::down : diff = -fieldSize; base = cellCount-1; step = -1; break;
        default : throw "Invalid move in state::move";
    }
    for (int k = 0; k < fieldSize; ++k) {
        int to(base), from(base + diff);
        for (int i = 0; i < fieldSize-1; ++i) {
            if (field[from] == 0) { from += diff; continue; }
            if (field[to] == 0) {
                if (field[from] != 0) {
                    field[to] = field[from];
                    field[from] = 0;
                    //to += diff;
                    moved = true;
                }
                from += diff;
            }
            else
                if (field[to] == field[from]) {
                    field[to]++; field[from] = 0; to += diff; from += diff; moved = true;
                }
                else {
                    to += diff;//   !!!!
                    if (from != to) {
                        field[to] = field[from]; field[from] = 0; moved = true;
                    }
                    from += diff;
                }
        }
        base += step;
    }
    return moved;
}

bool state::randomMove()
{
    int freeCells = 0;
    for (size_t i = 0; i < cellCount; ++i)
        if (field[i] == 0) ++freeCells;
    if (freeCells == 0) return false;

    int rand_pos = rand() % freeCells;
    int index = 0;

    for (; index < cellCount; ++index)
        if (field[index] == 0)
            if (rand_pos == 0) break;
            else --rand_pos;

    if (rand() % 100 < 10)
        field[index] = 2;
    else  field[index] = 1;

    return true;
}

bool state::hasFreeCell() const
{
    for (int i = 0; i < cellCount; ++i)
        if (field[i] == 0) return true;
    return false;
}

bool state::hasMoves() const {
    if (hasFreeCell()) return true;
    //  Свободных клеток нет, значит, ищем рядом стоящие плитки с одинаковым значением
    for (int r = 0; r<fieldSize; ++r)
        for (int c = 0; c<fieldSize - 1; ++c)
            if (field[r*fieldSize + c] == field[r*fieldSize + c + 1]) return true;

    for (int c = 0; c<fieldSize; ++c)
        for (int r = 0; r<fieldSize - 1; ++r)
            if (field[r*fieldSize + c] == field[(r + 1)*fieldSize + c]) return true;
    return false;
}

std::pair<int, int> state::addRandomTile()
{
    std::pair<int, int> rez(-1, -1);
    int freeCells = 0;
    for (size_t i = 0; i < cellCount; ++i)
        if (field[i] == 0) ++freeCells;
    if (freeCells == 0) return rez;

    int rand_pos = rand() % freeCells;
    int index = 0;

    for (; index < cellCount; ++index) {
        if (field[index] == 0)
            if (rand_pos == 0) break;
            else --rand_pos;
    }

    if (rand() % 100 < 10) {
        field[index] = 2; rez = std::pair<int, int>(index, 4);
    }
    else {field[index] = 1; rez = std::pair<int, int>(index, 2);}

    return rez;
}

bool state::performSlide(moves dir, std::array<int, cellCount>& movesMap, std::array<int, cellCount>& valueChanges, int & score)
{
    if(dir==moves::none) return false;
    // Это более тяжеловесная форма функции move - тут также определяются карты перемещений плиток
    for (int i = 0; i < cellCount; ++i) {
        movesMap[i] = i;  //  Все клетки на своём месте
        valueChanges[i] = field[i];  // Значения пока что остаются старые
    }
    score = 0;
    bool moved(false);
    int diff(0), base(0), step(0);
    switch (dir) {
        case moves::left: diff = +1; base = 0; step = fieldSize; break;
        case moves::up: diff = fieldSize; base = 0; step = 1; break;
        case moves::right: diff = -1; base = cellCount - 1; step = -fieldSize; break;
        case moves::down: diff = -fieldSize; base = cellCount - 1; step = -1; break;
    }
    for (int k = 0; k < fieldSize; ++k) {
        int to(base), from(base + diff);
        for (int i = 0; i < fieldSize - 1; ++i) {
            if (field[from] == 0) { from += diff; continue; }
            if (field[to] == 0) {
                if (field[from] != 0) {
                    field[to] = field[from];  movesMap[from] = to;
                    field[from] = 0;
                    //to += diff;
                    moved = true;
                }
                from += diff;
            }
            else
                if (field[to] == field[from]) {
                    field[to]++;
                    field[from] = 0;
                    movesMap[from] = to; valueChanges[from] = 0; valueChanges[to] = field[to]; score += (1 << field[to]);
                    to += diff;
                    from += diff; moved = true;
                }
                else {
                    to += diff;//   !!!!
                    if (from != to) {
                        field[to] = field[from]; field[from] = 0; moved = true;
                        movesMap[from] = to;
                    }
                    from += diff;
                }
        }
        base += step;
    }
    return moved;
}

void state::reset()
{
    for (size_t i = 0; i < cellCount; ++i) field[i] = 0;
}

std::ostream & operator<<(std::ostream & out, const state & data) {
    for (size_t i = 0; i < fieldSize; ++i) {
        for (size_t j = 0; j < fieldSize; ++j) {
            out << std::setw(6);
            int num(1);
            if (data.field[i*fieldSize + j] > 0) { num = num << data.field[i*fieldSize + j]; out << num; }
            else out << '.';
        }
        out << '\n';
    }

    out << "  ___________________________\n";
    return out;
}

void solver::redefineDepth(std::size_t new_depth)
{
    max_depth = new_depth;
    desk.resize(max_depth+1);
    for (size_t i = 0; i < max_depth+1; ++i)
        desk[i].reserve(30);
}

//  Породить дочерние ходы игрока для текущей вершины
void solver::spawnPlayerMoves(node & parent, size_t childsIndex)
{
    desk[childsIndex].clear();
    desk[childsIndex].push_back(parent);
    if (desk[childsIndex].back().move(moves::left)) { desk[childsIndex].back().fromParentMove = moves::left;  desk[childsIndex].push_back(parent); }
    if(desk[childsIndex].back().move(moves::right)) { desk[childsIndex].back().fromParentMove = moves::right;  desk[childsIndex].push_back(parent); }
    if (desk[childsIndex].back().move(moves::down)) { desk[childsIndex].back().fromParentMove = moves::down;  desk[childsIndex].push_back(parent); }
    if (!desk[childsIndex].back().move(moves::up)) desk[childsIndex].pop_back(); else desk[childsIndex].back().fromParentMove = moves::up;
    return;
}

//  Породить дочерние вершины для случайного хода
void solver::spawnRandomMoves(node & parent, size_t childsIndex)
{
    desk[childsIndex].clear();
    for(size_t i=0;i<cellCount;++i)
        if (parent.field[i] == 0) {
            //  Двоечку размещаем
            desk[childsIndex].push_back(parent);
            desk[childsIndex].back().field[i] = 1;
            //  Четвёрочку размещаем
            desk[childsIndex].push_back(parent);
            desk[childsIndex].back().field[i] = 2;
        }
    return;
}

void solver::processPlayerMoves(node & parent, size_t childsIndex, int depth)
{
    if (depth < max_depth)
        //  Если глубину не выбрали - идём вниз
        for (size_t i = 0; i < desk[childsIndex].size(); ++i)
        {
            spawnRandomMoves(desk[childsIndex][i], childsIndex + 1);
            processCompMoves(desk[childsIndex][i], childsIndex + 1, depth + 1);
        }
    else  // Иначе просто рассчитываем эвристику
        for (size_t i = 0; i < desk[childsIndex].size(); ++i)
            desk[childsIndex][i].heuristic = h(desk[childsIndex][i]);
    //  Выбраем наилучшее значение, ход не запоминаем, это только на первом слое
    parent.heuristic = 0;
    for (size_t i = 0; i < desk[childsIndex].size(); ++i)
        if (desk[childsIndex][i].heuristic > parent.heuristic)
            parent.heuristic = desk[childsIndex][i].heuristic;
}

void solver::processCompMoves(node & parent, size_t childsIndex, int depth)
{
    //  Если глубина позволяет, то идём вниз
    if (depth < max_depth)
        for (size_t i = 0; i < desk[childsIndex].size(); ++i)
        {
            spawnPlayerMoves(desk[childsIndex][i], childsIndex + 1);
            processPlayerMoves(desk[childsIndex][i], childsIndex + 1, depth + 1);
        }
    else  // Иначе просто рассчитываем эвристику
        for (size_t i = 0; i < desk[childsIndex].size(); ++i)
            desk[childsIndex][i].heuristic = h(desk[childsIndex][i]);

    //parent.heuristic = 0;
    parent.heuristic = desk[childsIndex][0].heuristic;
    for (size_t i = 1; i < desk[childsIndex].size(); ++i)
        if(desk[childsIndex][i].heuristic<parent.heuristic)
            parent.heuristic = desk[childsIndex][i].heuristic;
}


solver::solver(std::size_t max_depth)
{
    redefineDepth(max_depth);
}

///  α-β-отсечение
double solver::solveAB(node & origin, int depth, double alpha, double beta, bool maximizingPlayer)
{
    //  Тут как бы решаем. Всё довольно просто - это первый уровень, просто порождаем
    //  наши возможные ходы. Сначала пихаем в массивчик, а потом - снимаем показания эвристики

    if (depth >= max_depth)
        return h(origin);

    //node currentNode(origin);

    if (maximizingPlayer)
        spawnPlayerMoves(origin, depth);
    else
        spawnRandomMoves(origin, depth);

    if (desk[depth].size() == 0)
        return h(origin);

    if (maximizingPlayer) {
        double v = FLT_MIN;
        for (int i = 0; i < desk[depth].size(); ++i)
        {
            double tmp = solveAB(desk[depth][i], depth + 1, alpha, beta, false);
            if (tmp > v) {
                origin.prefferedMove = desk[depth][i].fromParentMove;
                v = tmp;
            }
            alpha = std::max(alpha, v);
            if (beta <= alpha) break;
        }
        return v;
    }
    else {
        double v = 0;
        for (int i = 0; i < desk[depth].size()/2; ++i)
        {
            v += 0.9*solveAB(desk[depth][2*i], depth + 1, alpha, beta, true);
            v += 0.1*solveAB(desk[depth][2*i+1], depth + 1, alpha, beta, true);
        }
        return 2*v/ desk[depth].size();
    }
}

bool solver::solveAlphaBeta(const state & origin)
{
    node startNode(origin);
    startNode.prefferedMove = moves::none;

    double alpha = FLT_MIN;
    double beta = FLT_MAX;

    solveAB(startNode, 0, alpha, beta, true);

    bestMove = startNode.prefferedMove;

    return bestMove != moves::none;
}


bool solver::solve(const state & origin)
{
    //  Тут как бы решаем. Всё довольно просто - это первый уровень, просто порождаем
    //  наши возможные ходы. Сначала пихаем в массивчик, а потом - тупо снимаем показания эвристики
    node startNode(origin);
    spawnPlayerMoves(startNode,0);
    if (desk[0].size() == 0) {
        bestMove = moves::none;
        return false;
    }

    processPlayerMoves(startNode, 0, 0);

    size_t index = desk[0].size();
    for (size_t i = 0; i < desk[0].size(); ++i)
        if (desk[0][i].heuristic == startNode.heuristic)
            index = i;
    if (index == desk[0].size()) throw ("WTF?!!!!");
    bestMove = desk[0][index].fromParentMove;
    return true;
}

moves solver::proposedMove() const
{
    return bestMove;
}
