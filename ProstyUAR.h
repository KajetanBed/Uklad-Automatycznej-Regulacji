#pragma once
#include "ModelARX.h"
#include "RegulatorPID.h"
#include <vector>

struct DaneSymulacji
{
    double w, y, e, u;
    double P, I, D;
};

class ProstyUAR
{
public:
    DaneSymulacji stepSamoPID(double setpoint, double y_zewnetrzne, double dt);
    double stepSamoARX(double u_zewnetrzne);
    ProstyUAR(QVector<double> startA, QVector<double> startB,
              double startKp, double startTi, double startTd);
    ~ProstyUAR();

    DaneSymulacji step(double setpoint, double dt);
    void reset();
    double lastY() const { return last_y; }
    const QVector<double> &getARX_A() const { return model->getA(); }
    const QVector<double> &getARX_B() const { return model->getB(); }
    int getARX_Delay() const { return model->getDelay(); }
    double getARX_Sigma() const { return model->getSigma(); }
    double getARX_UMin() const { return model->getUMin(); }
    double getARX_UMax() const { return model->getUMax(); }
    double getARX_YMin() const { return model->getYMin(); }
    double getARX_YMax() const { return model->getYMax(); }
    bool getARX_LimitsEnabled() const { return model->getLimitsEnabled(); }

    void setARX_TempA(const QVector<double> &A) { model->setTempA(A); }
    void setARX_TempB(const QVector<double> &B) { model->setTempB(B); }
    void setARX_TempDelay(int d) { model->setTempDelay(d); }
    void setARX_TempSigma(double s) { model->setTempSigma(s); }
    void setARX_TempControlLimits(double min, double max) { model->setTempControlLimits(min, max); }
    void setARX_TempOutputLimits(double min, double max) { model->setTempOutputLimits(min, max); }
    void setARX_TempLimitsEnabled(bool en) { model->setTempLimitsEnabled(en); }

    void commitARX() { model->commitChanges(); }
    void discardARX() { model->discardChanges(); }
    void resetARXMemory() { model->resetMemory(); }
    double getPID_Kp() const { return pid->getKp(); }
    double getPID_Ti() const { return pid->getTi(); }
    double getPID_Td() const { return pid->getTd(); }
    LiczCalk getPID_LiczCalk() const { return pid->getLiczCalk(); }

    void setPID_TempKp(double v) { pid->setTempKp(v); }
    void setPID_TempTi(double v) { pid->setTempTi(v); }
    void setPID_TempTd(double v) { pid->setTempTd(v); }
    void setPID_TempLiczCalk(LiczCalk m) { pid->setTempLiczCalk(m); }

    void commitPID() { pid->commitChanges(); }
    void discardPID() { pid->discardChanges(); }
    void resetPIDMemory() { pid->resetMemory(); }

private:
    ModelARX *model;
    RegulatorPID *pid;
    double last_y;
};
