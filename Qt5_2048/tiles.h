/*
 * Заголовок описывает классы для визуального представления плиток:
 *
 *  settings - настройки
 *
 *  visualTile - группа из двух графических примитивов, прямоугольника
 *      и текста. QGraphicsRectItem и QGraphicsSimpleTextItem объединяются
 *      в группу QGraphicsItemGroup, отрисовкой которой можно управлять.
 *
 *
 */
#ifndef TILES_H
#define TILES_H

#include <QGraphicsItemGroup>
#include <QLabel>
#include <QList>
#include <QTimer>
#include <QObject>
#include <QPushButton>
#include "state.h"
#include "threads.h"

class Controller;

class settings {
public:
    //  Размер плитки
    static int tileSide;
    //  Зазор между плитками
    static int margin;
    //  Число шагов при слайде
    static int slideSteps;
    //  Число шагов при появлении новой плитки
    static int arrangeSteps;
    //  Интервал обновления таймера
    static int timerInterval;
};

class visualTile {
private:
    enum class tileState {hidden, shown, moving, fading, arriving} state;

    int tilePos;
    //  Штукенции для выполнения движения - это пока не реализовано
    QPointF tileCoords;
    QPointF tileDelta;
    int stepsLeft;
    int tileNumber;

    QPointF calcPosition(int position) const;

public:

    //  Список графических объектов
    QList<QGraphicsItem *> grList;
    //  Указатель на группу объектов
    QGraphicsItemGroup *grp;

    //  Пара объектов для рисования
    QGraphicsRectItem *body;
    QGraphicsSimpleTextItem *label;

    visualTile(QGraphicsScene * scene);

    void setTileNumber(const int &num);
    void setTilePos(int position);
    void setVisible(bool visibility);
    void setOpacity(qreal opacity);

    bool visible() const {return state!=tileState::hidden;}
    int position() const {return tilePos;}

    //  Перемещение в указанную позицию
    bool slideTo(int position);
    //  Выключить плитку через заданное число шагов
    bool fade();
    //  Проявить новую плитку через заданное число шагов
    bool arrange(int num);
    //  Метод обновления
    bool update();
};

//  Доска для представления всего этого
class visualDesk  : public QObject
{
    Q_OBJECT
    //  Единственное, для чего тут QObject нужен - для однократного запуска таймера
    //  так как эта пакость требует наличия слота, а без QObject слоты не работают
private:

    //  Набор возможных состояний игры
    enum class gamePhase {noGame = 0, gamePaused, waitMove, tilesMoves, tilesArranging } phase;

    //  Ведёт ли игру компьютер
    bool AIPlaying;

    //  Диспетчер работы с потоком вычислений
    Controller *Master;

    //  Текущий результат
    int score;

    //  Указатель на визуальный компонент для отображения счёта
    QLabel * resultPanel;

    //  Указатель на метку для отображения количества ходов
    QLabel * movesLbl;

    //  Кнопка вкл/выкл ИИ. На ней надо текст изменить, если игра закончится
    QPushButton * AIButton;

    //  Счётчик ходов
    int movesCount;

    //  Ход компьютера - это просто добавление одной плиточки
    bool computerMove();

    //  Собственно текущее состояние
    state desk;

    //  Массив плиток - визуальных "бутербродов" из прямоугольников и надписей, создаём сразу все
    QList<visualTile *> tiles;

    //  Выбор свободной (скрытой на данный момент) плитки
    visualTile * getFreeTile();

public:
    //  Создание коллекции плиток требует графической сцены и указания элементов управления
    visualDesk(QGraphicsScene * scene, QLabel * lbl, QLabel * movesLabel, QPushButton * AIBtn);

    ~visualDesk();

    void resetDesk();
    void startGame();
    void startAI();
    void stopAI() {AIPlaying = false;update();}

    int getScore() const {return score;}

    bool slideTiles(moves dir);

    bool AIRuns() const {return AIPlaying;}

    void fullRepaint();

public slots:
    //  Самый тяжелый метод - должен отрабатывать перемещение плиток, и включать/выключать таймер
    void update();
};

#endif // TILES_H
