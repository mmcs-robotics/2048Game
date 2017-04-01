/*
 * Здесь можно изменять скорость анимации - меняя интервал таймера и число шагов
 *
 *
 *
 */
#include <QBrush>
#include <QPen>
#include <QGraphicsScene>
#include <ctime>
#include "tiles.h"
//#include <QDebug>

//  Размер плитки
 int settings::tileSide = 134;
//  Зазор между плитками
 int settings::margin = 10;
//  Число шагов при слайде
 int settings::slideSteps = 5;
//  Число шагов при появлении новой плитки
 int settings::arrangeSteps = 2;
//  Интервал обновления таймера
 int settings::timerInterval = 5;


visualTile::visualTile(QGraphicsScene * scene) : tilePos(0), stepsLeft(0), tileNumber(0), state(tileState::hidden)
{
    srand(time(NULL));

    tileCoords = calcPosition(tilePos);
    tileDelta = QPointF(0,0);
    QBrush blueBrush(Qt::lightGray);
    QPen outlinePen(Qt::black);
    outlinePen.setWidth(1);

    body = scene->addRect(0, 0, settings::tileSide, settings::tileSide, outlinePen, blueBrush);
    grList.push_back(body);

    QFont q("Arial", 42);
    q.setBold(true);
    label = scene->addSimpleText("", q );
    setTileNumber(0);
    grList.push_back(label);

    grp = scene->createItemGroup(grList);
    grp->setFlag(QGraphicsItem::ItemIsMovable);
}

void visualTile::setTileNumber(const int & num)
{

    tileNumber = num>1 ? num : 0;

    QFont q = label->font();

    if(tileNumber<=8192)
        q.setPointSize(44);
    else
        if(tileNumber<=32768)
            q.setPointSize(36);
        else
            q.setPointSize(36);

    label->setFont(q);
    label->setText(QString::number(tileNumber));

    qreal x = settings::tileSide/2 - label->boundingRect().width()/2;
    qreal y = settings::tileSide/2 - label->boundingRect().height()/2;

    //  Цвета, к сожалению, пришлось вручную подбирать
    switch(tileNumber) {
        case 0 : body->setBrush(QBrush(QColor(240,240,240)));break;
        case 2 : body->setBrush(QBrush(QColor(236,228,217)));break;
        case 4 : body->setBrush(QBrush(QColor(236,224,198)));break;
        case 8 : body->setBrush(QBrush(QColor(240,178,121)));break;
        case 16 : body->setBrush(QBrush(QColor(245,149,98)));break;
        case 32 : body->setBrush(QBrush(QColor(245,124,98)));break;
        case 64 : body->setBrush(QBrush(QColor(245,94,56)));break;
        case 128 : body->setBrush(QBrush(QColor(240,216,100)));break;
        case 256 : body->setBrush(QBrush(QColor(236,204,95)));break;
        case 512 : body->setBrush(QBrush(QColor(236,204,78)));break;
        case 1024 : body->setBrush(QBrush(QColor(236,197,62)));break;
        case 2048 : body->setBrush(QBrush(QColor(216,167,42)));break;
        case 4096 : body->setBrush(QBrush(QColor(216,127,42)));break;
        case 8192 : body->setBrush(QBrush(QColor(150,150,255)));break;
        case 16384 : body->setBrush(QBrush(QColor(180,180,255)));break;
        default : body->setBrush(QBrush(QColor(255,100,100)));
    }
    setVisible(tileNumber>0);

    label->setPos(x,y);
}

QPointF visualTile::calcPosition(int position) const {
    int base = settings::tileSide*2+(settings::margin*3)/2;
    return QPointF((position % 4)*settings::tileSide + (position % 4)*settings::margin - base,
                             (position / 4)*settings::tileSide + (position / 4)*settings::margin - base);
}

//  Моментальный переход в указанную позицию
void visualTile::setTilePos(int position)
{
    tilePos = position;
    tileCoords = calcPosition(tilePos);
    tileDelta = QPointF(0,0);
    stepsLeft = 0;
    grp->setPos(tileCoords);
}

//  Моментально включить/выключить плитку
void visualTile::setVisible(bool visibility)
{
    if(visibility) state = tileState::shown; else state = tileState::hidden;
    body->setVisible(visibility);
    label->setVisible(visibility);
}

//  Просто изменение прозрачности, без изменения состояния
void visualTile::setOpacity(qreal opacity) {
    if(opacity>0) {
        body->setVisible(true);
        body->setOpacity(opacity);
        label->setVisible(true);
        label->setOpacity(opacity);
    }
    else {
        body->setVisible(false);
        body->setOpacity(opacity);
        label->setVisible(false);
        label->setOpacity(opacity);
    }
}

//  Перемещение в указанную позицию
//  Результат - остались ли еще шаги?
bool visualTile::slideTo(int position)
{
    stepsLeft = settings::slideSteps;

    //  Заранее указываем позицию назначения
    tilePos = position;

    QPointF dest(calcPosition(position));
    QPointF from(grp->pos());
    tileDelta = dest-from;
    if(stepsLeft!=0)
        tileDelta /= stepsLeft;
    state=tileState::moving;
    return true;
}

//  Потушить плитку
bool visualTile::fade() {
    if(state==tileState::hidden) return false;
    state = tileState::fading;
    stepsLeft = settings::arrangeSteps;
    return true;
}

//  Проявить новую плитку через заданное число шагов
bool visualTile::arrange(int num) {
    if(state!=tileState::hidden) return false;
    setTileNumber(num);
    state = tileState::arriving;
    stepsLeft = settings::arrangeSteps;
    return true;
}

//  Результат - изменилось ли что-то в плитке, или нет
bool visualTile::update() {

    if(state==tileState::hidden || state==tileState::shown)
        return false;

    --stepsLeft;
    if(stepsLeft<0) stepsLeft=0;

    if(state==tileState::moving) {
        //  Перемещение плиточки
        tileCoords += tileDelta;
        grp->setPos(tileCoords);
        if(stepsLeft<=0) {
            stepsLeft = 0;
            tileDelta = QPointF(0,0);
            //  Перемещение закончено, обновляем номер (если было слияние)
            setTileNumber(tileNumber);
            state = tileState::shown;
            return false;
        }
        return true;
    }

    if(state==tileState::arriving) {
        float tmp = settings::arrangeSteps;
        if(tmp==0) tmp=1;
        tmp = (tmp-stepsLeft)/tmp;
        setOpacity(tmp);
        if(stepsLeft==0)
            state=tileState::shown;
        return true;
    }

    if(state==tileState::fading) {
        float tmp = settings::arrangeSteps;
        if(tmp==0) tmp=1;
        tmp = stepsLeft/tmp;
        setOpacity(tmp);
        if(stepsLeft==0) {
            state=tileState::hidden;
            setTileNumber(0);
            setVisible(false);
            return false;
        }
        return true;
    }
    return false;
}

//---------------------------------------------------------------------------------------------------------------

visualDesk::visualDesk(QGraphicsScene * scene, QLabel * lbl, QLabel * movesLabel, QPushButton * AIBtn) : AIPlaying(false), phase (gamePhase::noGame), resultPanel(lbl),
    movesCount(0), movesLbl(movesLabel), AIButton(AIBtn)
{
    for(int index = 0; index<16; ++index) {
        visualTile *tile = new visualTile(scene);
        tile->setTilePos(index);
        tile->setTileNumber(0);
        //  if(index % 2 == 0) tile->setVisible(false);
        tile->setVisible(false);
        tiles.push_back(tile);
    }
    Master = new Controller(this);
}

visualDesk::~visualDesk() {
    for(int index = 0; index<16; ++index)
        delete tiles[index];
    delete Master;
}

visualTile *visualDesk::getFreeTile()
{
    for(int i=0;i<tiles.size();++i)
        if(tiles[i]->visible()) return tiles[i];
    return nullptr;
}

void visualDesk::resetDesk()
{
    phase = gamePhase::noGame;
    AIPlaying = false;
    score = 0;
    movesCount = 0;
    for(int i=0; i<tiles.size(); ++i) {
        tiles[i]->setTileNumber(0);
        desk.field[i]=0;
    }
}

void visualDesk::startGame()
{
    resetDesk();
    AIPlaying = false;
    computerMove();
    resultPanel->setText(QString::number(score));
    movesLbl ->setText(QString::number(movesCount));
}

void visualDesk::startAI() {

    //  Защита от повторного срабатывания
    if(phase==gamePhase::noGame) return;
    AIPlaying = true;
    //  Если ничего не происходит - обновить, там и ход AI будет
    if(phase==gamePhase::gamePaused) phase = gamePhase::waitMove;
    if(phase==gamePhase::waitMove) update();
}

bool visualDesk::slideTiles(moves dir)
{
    if(phase!=gamePhase::waitMove) return false;

    std::array<int,cellCount> movesMap,valueChanges;
    int moveScore(0);
    //qDebug() << "Performing slide " << int(dir);
    //debugDeskPrint();
    if(!desk.performSlide(dir,movesMap,valueChanges,moveScore)) {
        //  Попытка хода закончилась неудачей

        //  Если тупит компьтер - отключить этого поганца
        AIPlaying = false;
        AIButton->setText("Включить ИИ");

        //  Ну а если тупит игрок, то простительно
        phase = gamePhase::waitMove;
        return false;
    }

    /*/qDebug() << "After slide :";
    //debugDeskPrint();

    QString vc("Value changes map map : ");
    for(int i=0;i<valueChanges.size();++i)
        vc += "[" + QString::number(i)+"]="+QString::number(valueChanges[i])+" ";
    qDebug() << vc;*/

    score+=moveScore;
    ++movesCount;
    resultPanel->setText(QString::number(score));
    movesLbl ->setText(QString::number(movesCount));

    //  Назначить плитки, чтобы ехали куда надо, и всё такое
    //  Да, ещё и порядок на сцене менять надо как-то

    //  Технически перед этим все плитки на своих местах
    /*debugDeskPrint();
    QString mm("Moves map : ");
    for(int i=0;i<movesMap.size();++i)
        mm += "[" + QString::number(i)+"]="+QString::number(movesMap[i])+" ";
    qDebug() << mm;*/

    for(int i=0;i<movesMap.size();++i)
       {
            //  Эта плитка переезжает
            tiles[i]->slideTo(movesMap[i]);
        }
    phase = gamePhase::tilesMoves;

    update();
    return true;
}

//  Отработка перемещения плиток, и включение/выключение таймера
void visualDesk::update() {
    //  С таймером и прочей мутью
    //  Подвинуть плиточки, если еще двигаются - то ничего не делать, иначе если играет компьютер, то пнуть AI
    //  Таймер, кстати - фиг с ним, пусть всё время работает. Или нет?

    if(phase==gamePhase::noGame || phase==gamePhase::gamePaused) return;

    if(phase==gamePhase::waitMove) {
        if(AIPlaying) Master->emitOperate(desk);
        return;
    }

    //  Остались варианты при которых что-то происходит - плитки двигаются или
    //  появляются, или еще что

    bool tileMoved(false);
    for(int i=0;i<tiles.size();++i)
        if(tiles[i]->update())
            tileMoved = true;

    //  Если что-то произошло, то надо себя же повторно вызвать и выйти
    if(tileMoved) {
       // qDebug() << "Something happens";
        //  повторный запуск самой себя
        QTimer::singleShot(settings::timerInterval, this, SLOT(update()));
        return;
    }

    //  А теперь остались варианты - либо был ход компьютера, либо человек походил

    if(phase==gamePhase::tilesMoves) {
        //  если плиточка не двинулась - ход компьютера
        //fullRepaint();
        computerMove();
        return;
    }

    if(phase==gamePhase::tilesArranging) {
        //  Ну тут вроде бы как плитки появляются
        //  пока что просто затычка
        phase=gamePhase::waitMove;
        //qDebug() << "It's OKAY!!!!";
        if(AIPlaying)
            Master->emitOperate(desk);
    }
    return;
}

bool visualDesk::computerMove() {
    fullRepaint();
    //qDebug() << "Adding tile";debugDeskPrint();
    std::pair<int, int> newTile = desk.addRandomTile();
    //qDebug() << "After adding tile";debugDeskPrint();
    if(newTile.first == -1) {
        phase=gamePhase::gamePaused;
        return false;
    }

    if(tiles[newTile.first]->visible()==true) {
        //qDebug() << "This cannot be!!!";
    }

    //qDebug() << "Spawning tile at " << newTile.first << " with value " << newTile.second;

    tiles[newTile.first]->arrange(newTile.second);
    //qDebug() << "Spawn at " << newTile.first << " with value " << newTile.second;

    phase = gamePhase::tilesArranging;
    update();

    return true;
}

void visualDesk::fullRepaint()
{
    for(int i=0; i<tiles.size(); ++i) {
        tiles[i]->setTilePos(i);
        tiles[i]->setTileNumber(1 << desk.field[i]);
    }
}
