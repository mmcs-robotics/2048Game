/*
 * Вычисления ведутся в отдельном потоке - чтобы не блокировали пользовательский интерфейс.
 * Здесь реализация этого потока, внутри которого на каждом шаге создаётся объект solver,
 * и решает представленную позицию.
 *
 *
 */

#ifndef THREADS_H
#define THREADS_H

#include <QApplication>
#include <QThread>
#include <QMetaType>

#include "tiles.h"

Q_DECLARE_METATYPE(state)
Q_DECLARE_METATYPE(moves)

class visualDesk;

class Worker : public QObject
{
    Q_OBJECT

public slots:
    void doWork(const state & source) {
        //  Собственно, основная работа тут. Создаём класс для каждого хода, решаем и
        //  возвращаем результат
        solver ai1011(7);
        ai1011.solveAlphaBeta(source);
        emit resultReady(ai1011.proposedMove());
    }

signals:
    void resultReady(moves);
};

class Controller : public QObject
{
    Q_OBJECT
    QThread workerThread;
    visualDesk * dsk;
    Worker *worker;

public:
    bool workInProgress;
    Controller(visualDesk * activeDesk) : dsk(activeDesk), workInProgress(false) {
        qRegisterMetaType<state>();
        qRegisterMetaType<moves>();
        worker = new Worker;
        worker->moveToThread(&workerThread);
        connect(&workerThread, SIGNAL(finished()), worker, SLOT(deleteLater()));
        connect(this, SIGNAL(operate(const state &)), worker, SLOT(doWork(const state &)));
        connect(worker, SIGNAL(resultReady(moves)), this, SLOT(handleResults(moves)));
        workerThread.start();
    }

    ~Controller() {
        workerThread.terminate();
        workerThread.wait();
    }
public slots:
    void handleResults(moves move);

public:
    void emitOperate(const state & source) {
        if(!workInProgress) {
            workInProgress = true;
            emit operate(source);
        }
    }

signals:
    void operate(const state & source);

};

#endif // THREADS_H
