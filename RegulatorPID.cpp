#include "RegulatorPID.h"
#include <cmath>

RegulatorPID::RegulatorPID(double Kp_, double Ti_, double Td_)
    : Kp(Kp_), Ti(Ti_), Td(Td_), integral(0.0), lastError(0.0),
      lastDerivative(0.0), integralWew(0.0), liczCalk(LiczCalk::Wew),
      TempKp(Kp_), TempTi(Ti_), TempTd(Td_), TempliczCalk(LiczCalk::Wew)
{
}

void RegulatorPID::setLiczCalk(LiczCalk mode)
{
    if (liczCalk != mode)
    {
        if (mode == LiczCalk::Wew)
        {
            integralWew = integral / Ti;
        }
        else
        {
            integral = integralWew * Ti;
        }
        liczCalk = mode;
    }
}

void RegulatorPID::setStalaCalk(double Ti_new)
{
    Ti = Ti_new;
}
PIDResult RegulatorPID::symuluj(double uchyb, double dt)
{
    if (firstRun)
    {
        integral = 0.0;
        integralWew = 0.0;
        lastError = 0.0;
        firstRun = false;
    }
    if (dt <= 0.000001)
        dt = 0.1;
    double P = Kp * uchyb;
    double I = 0.0;
    double I_temp = 0.0;
    if (std::abs(Ti) < 0.00001)
    {
        I = 0.0;
        integral = 0.0;
        integralWew = 0.0;
    }
    else
    {
        if (liczCalk == LiczCalk::Zew)
        {
            integral += uchyb;
            I_temp = integral / Ti;
        }
        else
        {
            integralWew += (uchyb / Ti);
            I_temp = integralWew;
        }
    }
    I = Kp * I_temp;
    double D = 0.0;
    if (Td > 0.00001)
    {
        double derivative = (uchyb - lastError);
        D = Kp * Td * derivative;
    }
    lastError = uchyb;
    double output = P + I + D;
    if (std::isnan(output) || std::isinf(output))
    {
        output = 0.0;
        P = 0.0;
        I = 0.0;
        D = 0.0;
        integral = 0.0;
        integralWew = 0.0;
        lastError = 0.0;
    }
    return {output, P, I, D};
}
void RegulatorPID::resetMemory()
{
    integral = 0.0;
    lastError = 0.0;
    lastDerivative = 0.0;
    integralWew = 0.0;
    firstRun = true;
}

void RegulatorPID::setTempKp(double v_new) { TempKp = v_new; }
void RegulatorPID::setTempTi(double x_new) { TempTi = x_new; }
void RegulatorPID::setTempTd(double z_new) { TempTd = z_new; }
void RegulatorPID::setParametryPID(double v_new, double x_new, double z_new)
{
    Kp = v_new;
    Ti = x_new;
    Td = z_new;
}

void RegulatorPID::setTempLiczCalk(LiczCalk Tempmode)
{
    if (TempliczCalk != Tempmode)
    {
        if (Tempmode == LiczCalk::Wew)
        {
            integralWew = integral / Ti;
        }
        else
        {
            integral = integralWew * Ti;
        }
        TempliczCalk = Tempmode;
    }
}
void RegulatorPID::commitChanges()
{
    Kp = TempKp;
    Ti = TempTi;
    Td = TempTd;
    liczCalk = TempliczCalk;
}
void RegulatorPID::discardChanges()
{
    TempKp = Kp;
    TempTi = Ti;
    TempTd = Td;
    TempliczCalk = liczCalk;
}
