#pragma once
#include <QDataStream>
struct PIDResult
{
    double u;
    double P;
    double I;
    double D;
};

enum class LiczCalk
{
    Zew,
    Wew
};

class RegulatorPID
{
    double lastP = 0.0;
    double lastI = 0.0;
    double lastD = 0.0;
    double lastU = 0.0;
    bool firstRun = true;

public:
    RegulatorPID(double Kp = 1.0, double Ti = 0.0, double Td = 0.0);

    PIDResult symuluj(double uchyb, double dt);
    void resetMemory();

    void setTempKp(double v_new);
    void setTempTi(double x_new);
    void setTempTd(double z_new);
    void setParametryPID(double v_new, double x_new, double z_new);

    void setLiczCalk(LiczCalk mode);
    void setStalaCalk(double Ti_new);
    void setTempLiczCalk(LiczCalk Tempmode);

    void commitChanges();
    void discardChanges();

    double getKp() const { return Kp; }
    double getTi() const { return Ti; }
    double getTd() const { return Td; }
    LiczCalk getLiczCalk() const { return liczCalk; }
    double getLastP() const { return lastP; }
    double getLastI() const { return lastI; }
    double getLastD() const { return lastD; }
    double getLastU() const { return lastU; }

    void resetIntegral() { integral = 0.0; }
    void resetDiff()
    {
        lastError = 0.0;
        lastDerivative = 0.0;
    }

private:
    double Kp;
    double Ti;
    double Td;
    double TempKp;
    double TempTi;
    double TempTd;
    double integral;
    double integralWew;
    double lastError;
    double lastDerivative;
    LiczCalk liczCalk;
    LiczCalk TempliczCalk;
};

inline QDataStream &operator<<(QDataStream &out, const RegulatorPID &pid)
{
    out << pid.getKp() << pid.getTi() << pid.getTd() << static_cast<qint32>(pid.getLiczCalk());
    return out;
}

inline QDataStream &operator>>(QDataStream &in, RegulatorPID &pid)
{
    double kp, ti, td;
    int tryb_int;
    in >> kp >> ti >> td >> tryb_int;
    pid.setParametryPID(kp, ti, td);
    pid.setLiczCalk(static_cast<LiczCalk>(tryb_int));
    return in;
}