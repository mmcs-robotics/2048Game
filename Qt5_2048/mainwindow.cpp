#include <QGraphicsItem>
#include <QKeyEvent>
#include "mainwindow.h"
#include "ui_mainwindow.h"
//#include <QDebug>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    //  Инициализация - создаём и настраиваем графическую сцену
    QGraphicsScene *scene = new QGraphicsScene(this);
    QGraphicsView *view = ui->graphicsView;

    scene->setItemIndexMethod(QGraphicsScene::NoIndex);
    scene->setSceneRect(-200, -200, 400, 400);
    view->setScene(scene);

    view->setCacheMode(QGraphicsView::CacheBackground);
    view->setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
    view->setRenderHint(QPainter::Antialiasing);
    view->setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    view->scale(qreal(1), qreal(1));
    view->setMinimumSize(400, 400);

    dsk = new visualDesk(scene,ui->label,ui->label_2,ui->pushButton);
}

MainWindow::~MainWindow()
{
    delete dsk;
    delete ui;
}

void MainWindow::on_pushButton_clicked()
{
    if(dsk->AIRuns()) {
        dsk->stopAI();
        ui->pushButton->setText("Включить ИИ");
    }
    else {
        ui->pushButton->setText("Остановить ИИ");
        dsk->startAI();
    }
}

void MainWindow::on_pushButton_2_clicked()
{
    close();
}

void MainWindow::on_pushButton_3_clicked()
{
    ui->pushButton->setEnabled(true);
    ui->pushButton_3->setText("Перезапуск");
    dsk->startGame();
    ui->pushButton->setText("Включить ИИ");
    setFocus();
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    //  С кнопками всё сложно, для управления клавишами надо постоянно фокус сбрасывать на форму
    dsk->stopAI();
    switch(event->key()) {
       case Qt::Key_Up:
       case 87 :
       case '5' : dsk->slideTiles(moves::up);break;
       case Qt::Key_Left:
       case 65 :
       case '1' : dsk->slideTiles(moves::left);break;
       case 83 :
       case Qt::Key_Down:
       case '2' : dsk->slideTiles(moves::down);break;
       case 68 :
       case Qt::Key_Right:
       case '3' : dsk->slideTiles(moves::right);break;
    }
}
