#include "ProstyUAR.h"

ProstyUAR::ProstyUAR(QVector<double> startA, QVector<double> startB,
                     double startKp, double startTi, double startTd)
    : last_y(0.0)
{
    model = new ModelARX(startA, startB, 1, 0.0);
    pid = new RegulatorPID(startKp, startTi, startTd);
}
ProstyUAR::~ProstyUAR()
{
    delete model;
    delete pid;
}

void ProstyUAR::reset()
{
    last_y = 0.0;
    model->resetMemory();
    pid->resetMemory();
}

DaneSymulacji ProstyUAR::step(double setpoint, double dt)
{
    double e = setpoint - last_y;
    PIDResult wynikPID = pid->symuluj(e, dt);
    double u = wynikPID.u;
    double y = model->symuluj(u);
    last_y = y;
    return {setpoint, y, e, u, wynikPID.P, wynikPID.I, wynikPID.D};
}
DaneSymulacji ProstyUAR::stepSamoPID(double setpoint, double y_zewnetrzne, double dt)
{
    double e = setpoint - y_zewnetrzne;
    PIDResult wynikPID = pid->symuluj(e, dt);
    last_y = y_zewnetrzne;
    return {setpoint, y_zewnetrzne, e, wynikPID.u, wynikPID.P, wynikPID.I, wynikPID.D};
}

double ProstyUAR::stepSamoARX(double u_zewnetrzne)
{
    double y = model->symuluj(u_zewnetrzne);
    last_y = y;
    return y;
}
