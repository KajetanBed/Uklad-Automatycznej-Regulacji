#pragma once
#include <cmath>

class SignalGenerator
{
public:
    enum class Type
    {
        SIN,
        KROK
    };

    SignalGenerator();
    Type getType() const { return type; }
    int getPeriod() const { return T; }
    double getAmplitude() const { return A; }
    double getOffset() const { return S; }
    double getDuty() const { return p; }
    void setType(Type t) { type = t; }
    void setPeriod(int T_new)
    {
        if (T_new < 1)
            T = 1;
        else
            T = T_new;
    }
    void setAmplitude(double A_new)
    {
        if (A_new < 0.0)
            A = 0.0;
        else
            A = A_new;
    }
    void setOffset(double S_new) { S = S_new; }
    void setDuty(double p_new)
    {
        if (p_new < 0.0)
            p = 0.0;
        else if (p_new > 1.0)
            p = 1.0;
        else
            p = p_new;
    }
    void setTempType(Type t_new);
    void setTempPeriod(int T_new);
    void setTempAmplitude(double A_new);
    void setTempOffset(double TempS_new);
    void setTempDuty(double p_new);

    void resetMemory();
    void commitChanges();
    void discardChanges();

    double valueAtStep(int k) const;

private:
    Type type;

    int T;
    double A;
    double S;
    double p;

    int TempT_new;
    double TempA_new;
    double Tempp_new;
    double TempS;
    Type Temptype;
};
