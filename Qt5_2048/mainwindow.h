#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QObject>
#include <QEvent>
#include "threads.h"
#include "tiles.h"
#include "state.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_pushButton_clicked();

    void on_pushButton_2_clicked();

    void on_pushButton_3_clicked();

    void keyPressEvent(QKeyEvent*);

private:
    Ui::MainWindow *ui;

    visualDesk *dsk;
};

#endif // MAINWINDOW_H
