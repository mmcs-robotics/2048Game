/*
 * Описываются классы для поиска решения в игре:
 *
 *      state - класс для описания состояния игры (поле). Хранит только значения
 *          в клетках, причём не сами значения, а степени двойки.
 *
 *      heuristic - класс для работы с эвристикой, не завершён, теоретически может
 *          быть доработан для автоматической модификации и поиска оптимальной эвристики.
 *          Оставлены методы сохранения в файл, чтения, модификации, однако в текущей
 *          версии это не реализовано, эвристика фиксированная.
 *
 *
 *      node - обёртка над состоянием игры, добавляет поле эвристики и указание на родительское
 *          состояние
 *
 *      solver - класс-решатель, инкапсулирует методы поиска решения
 */

#pragma once

#include <iostream>
#include <array>
#include <vector>
#include <string>

//  Размер поля и количество ячеек
const int fieldSize = 4;
const int cellCount = fieldSize*fieldSize;

//  Направление слайда плиточек
enum class moves { right=0, down, left, up, none };

//  Состояние игры
class state {
public:
    typedef unsigned char cellValue;
    typedef cellValue dskType[cellCount];

    //  Копирование поля целиком
    static void copyField(dskType & dst, const dskType & src) {
        memcpy(dst, src, cellCount * sizeof(cellValue));
    }

public:
    dskType field;
    static size_t totalObjects;

    //  Конструктор пустого поля - только начальное состояние так создаётся
    state();

    //  Копирующий конструктор - необходимости в нём особой нет, объект POD
    state(const state & other);

    //  Оператор присваивания - аналогично,
    const state & operator=(const state & other);

    //  Слайд (ход) для текущей позиции. При неудаче возвращает false
    bool move(moves dir);

    //  Случайный ход - для игры
    bool randomMove();

    //  Проверка наличия свободной ячейки
    bool hasFreeCell() const;

    //  Проверка возможности хода
    bool hasMoves() const;

    //  Выполнить ход в указанном направлении
    bool performSlide(moves dir, std::array<int,cellCount> & movesMap, std::array<int, cellCount> & valueChanges, int & score);

    //  Добавить случайную плитку - вернёт куда добавили, и значение плитки. Если нет - то -1
    std::pair<int, int> addRandomTile();

    //  Очистка поля
    void reset();

    //  Вывод на экран либо в файл
    friend std::ostream & operator<<(std::ostream & out, const state & data);
};

std::ostream & operator<<(std::ostream & out, const state & data);

//  Класс эвристики
class heuristic
{
public:
    //  Веса для клеток для вычисления скалярного произведения на плитки
    std::array<double, cellCount> heur;
    //  Эти поля в данном проекте не используются
    std::vector<double> params;
    std::vector<double> deltas;
    std::vector<double> scores, oldScores;
    size_t current, iterations;
    double totalScoring;
    double prevScore;

    heuristic();
    bool loadFromFile(std::string filename);
    bool saveToFile(std::string filename);
    void addScore(double result);
    bool nextParam();

    //  Вычисление эвристической функции - используется для оценки поля
    double calcHeuristics(const state & field) const;
    double operator()(const state & field) const { return calcHeuristics(field); }
};

//  Состояние в пространстве поиска - добавляет к позиции эвристическую оценку.
//  Также тут запоминаются сделанный ход и предпочтительный следующий
class node : public state {
public:
    double heuristic;
    moves fromParentMove;
    moves prefferedMove;
    node() : heuristic(0), fromParentMove(moves::none) {}
    explicit node(const state & src) : state(src), heuristic(0), fromParentMove(moves::none) {}
};

//  Решатель - инкапсулирует методы решения
class solver {

private:
    //  Максимальная глубина просмотра
    std::size_t max_depth;
    //  Вектор для разворачивания потенциальных ходов
    std::vector<std::vector<node>> desk;
    //  Исходное состояния
    state source;
    //  Выбранный ход
    moves bestMove;
    //  Породить дочерние вершины для хода игрока
    void spawnPlayerMoves(node & parent, size_t childsIndex);
    //  Породить дочерние вершины для случайного хода
    void spawnRandomMoves(node & parent, size_t childsIndex);
    //  Обработать дочерние и выбрать значение
    void processPlayerMoves(node & parent, size_t childsIndex, int depth);
    //  Обработка ходов компьютера
    void processCompMoves(node & parent, size_t childsIndex, int depth);
    //  Вспомогательная функция для решения
    double solveAB(node & origin, int depth, double alpha, double beta, bool maximizingPlayer);

public:
    //  Поле эвристики
    heuristic h;
    //  Глубина по умолчанию 4, однако можно и больше
    solver(std::size_t max_depth = 4);
    //  Переопределение глубины просмотра
    void redefineDepth(std::size_t new_depth);
    //  Решение с указанием начального состояния
    bool solve(const state & origin);
    //  Решение с аналогом альфа-бета
    bool solveAlphaBeta(const state & origin);
    //  Предлагаемый ход
    moves proposedMove() const;
};
