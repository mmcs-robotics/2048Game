#include "threads.h"

void Controller::handleResults(moves move) {
    //  Тут одноразовый вызов метода, повтор он сам вызовет при необходимости
    workInProgress = false;
    if(dsk->AIRuns()) dsk->slideTiles(move);
        //emitOperate(dsk->desk);
}
