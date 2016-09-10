#include <QDebug>
#include <QFile>
#include "dsmr_p1_parser.h"
#include "dsmr_p1_test.h"
#include "dsmr_p1_acquisitor.h"
#include "gass_meter.h"
#include "grid_meter.h"

void DsmrP1Test::run()
{
    QFile file("dsmr_p1_test_1.txt");
    file.open(QFile::ReadOnly);
    QByteArray t = file.readAll();

    DsmrP1Parser p;
    GridMeter m;
    GassMeter gm;
    DsmrP1Acquisitor a(&m, &gm);
    for (char c: t)
    {
        DsmrP1Message msg;
        DsmrP1State s = p.process(c, msg);
        if (s == DsmrP1State::Ok)
        {
            a.handleMessage(msg);
            qDebug() << m.power();
        }
    }
}
