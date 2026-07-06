#pragma once

#include <QObject>
#include <QTimer>
#include <vector>

#include "SignalGenerator.h"
#include "ProstyUAR.h"

class WarstwaU : public QObject
{
    Q_OBJECT

public:
    WarstwaU(QVector<double> startA, QVector<double> startB,
             double Kp, double Ti, double Td,
             QObject *parent = nullptr)
        : QObject(parent)
    {
        uarSystem = new ProstyUAR(startA, startB, Kp, Ti, Td);

        timerWewnetrzny = new QTimer(this);
        connect(timerWewnetrzny, &QTimer::timeout, this, &WarstwaU::przeliczKrok);

        zresetujStanSymulacji();
    }

    ~WarstwaU()
    {
        delete uarSystem;
    }
    void startSymulacji(int interwalMs, double dtSekundy)
    {
        if (timerWewnetrzny->isActive())
            return;
        ustawKrokCzasu(dtSekundy);
        ustawInterwalTimera(interwalMs);
        timerWewnetrzny->start();
    }

    void stopSymulacji()
    {
        timerWewnetrzny->stop();
    }

    void zresetujStanSymulacji()
    {
        timerWewnetrzny->stop();

        uarSystem->resetPIDMemory();
        uarSystem->resetARXMemory();
        uarSystem->reset();

        generator.resetMemory();

        licznikKrokow = 0;
        czasGlobalny = 0.0;
        krokDt = 0.01;
        ostatnieY = 0.0;
    }

    void ustawInterwalTimera(int ms)
    {
        if (ms < 10)
            ms = 10;
        timerWewnetrzny->setInterval(ms);
    }

    void ustawKrokCzasu(double dt)
    {
        if (dt < 0.001)
            dt = 0.001;
        krokDt = dt;
    }

    bool czySymulacjaTrwa() const
    {
        return timerWewnetrzny->isActive();
    }
    void ustawTrybSieciowy(bool sieciowy, bool regulator)
    {
        trybSieciowy = sieciowy;
        jestemRegulatorem = regulator;
    }

    void ustawOstatnieY(double y)
    {
        ostatnieY = y;
    }
    double krokObiektu(double u_zewnetrzne)
    {
        return uarSystem->stepSamoARX(u_zewnetrzne);
    }
    void synchronizujCzasObiektu(quint32 idProbki)
    {
        if (licznikKrokow == 0 && idProbki == 0)
        {
            uarSystem->resetPIDMemory();
            uarSystem->resetARXMemory();
            uarSystem->reset();
        }
        licznikKrokow = idProbki;
        czasGlobalny = licznikKrokow * krokDt;
    }
    const QVector<double> &getA() const { return uarSystem->getARX_A(); }
    const QVector<double> &getB() const { return uarSystem->getARX_B(); }
    int getDelay() const { return uarSystem->getARX_Delay(); }
    double getSigma() const { return uarSystem->getARX_Sigma(); }
    double getUMin() const { return uarSystem->getARX_UMin(); }
    double getUMax() const { return uarSystem->getARX_UMax(); }
    double getYMin() const { return uarSystem->getARX_YMin(); }
    double getYMax() const { return uarSystem->getARX_YMax(); }
    bool getLimitsEnabled() const { return uarSystem->getARX_LimitsEnabled(); }

    void setTempA(const QVector<double> &A) { uarSystem->setARX_TempA(A); }
    void setTempB(const QVector<double> &B) { uarSystem->setARX_TempB(B); }
    void setTempDelay(int d) { uarSystem->setARX_TempDelay(d); }
    void setTempSigma(double s) { uarSystem->setARX_TempSigma(s); }
    void setTempControlLimits(double umin, double umax) { uarSystem->setARX_TempControlLimits(umin, umax); }
    void setTempOutputLimits(double ymin, double ymax) { uarSystem->setARX_TempOutputLimits(ymin, ymax); }
    void setTempLimitsEnabled(bool en) { uarSystem->setARX_TempLimitsEnabled(en); }

    void commitARX() { uarSystem->commitARX(); }
    void discardARX() { uarSystem->discardARX(); }
    void resetARX() { uarSystem->resetARXMemory(); }
    double getKp() const { return uarSystem->getPID_Kp(); }
    double getTi() const { return uarSystem->getPID_Ti(); }
    double getTd() const { return uarSystem->getPID_Td(); }
    LiczCalk getLiczCalk() const { return uarSystem->getPID_LiczCalk(); }

    void setTempKp(double v) { uarSystem->setPID_TempKp(v); }
    void setTempTi(double v) { uarSystem->setPID_TempTi(v); }
    void setTempTd(double v) { uarSystem->setPID_TempTd(v); }
    void setTempLiczCalk(LiczCalk m) { uarSystem->setPID_TempLiczCalk(m); }

    void commitPID() { uarSystem->commitPID(); }
    void discardPID() { uarSystem->discardPID(); }
    void resetPID() { uarSystem->resetPIDMemory(); }
    SignalGenerator::Type getGenType() const { return generator.getType(); }
    int getGenPeriod() const { return generator.getPeriod(); }
    double getGenAmplitude() const { return generator.getAmplitude(); }
    double getGenOffset() const { return generator.getOffset(); }
    double getGenDuty() const { return generator.getDuty(); }

    void setTempGenType(SignalGenerator::Type t) { generator.setTempType(t); }
    void setTempGenPeriod(int T) { generator.setTempPeriod(T); }
    void setTempGenAmplitude(double A) { generator.setTempAmplitude(A); }
    void setTempGenOffset(double S) { generator.setTempOffset(S); }
    void setTempGenDuty(double p) { generator.setTempDuty(p); }

    void commitGenerator() { generator.commitChanges(); }
    void discardGenerator() { generator.discardChanges(); }
    void resetGenerator() { generator.resetMemory(); }
    double getKrokDt() const { return krokDt; }

signals:
    void noweDaneSymulacji(double czas, const DaneSymulacji &dane);
    void wyslijSterowanie(quint32 idProbki, double u, double w);

private slots:
    void przeliczKrok()
    {
        if (licznikKrokow == 0)
        {
            uarSystem->resetPIDMemory();
            uarSystem->resetARXMemory();
            uarSystem->reset();
        }

        double w = generator.valueAtStep(licznikKrokow);
        DaneSymulacji wynik;

        if (trybSieciowy && jestemRegulatorem)
        {
            wynik = uarSystem->stepSamoPID(w, ostatnieY, krokDt);
            emit wyslijSterowanie(licznikKrokow, wynik.u, w);
        }
        else if (trybSieciowy && !jestemRegulatorem)
        {
            return;
        }
        else
        {
            wynik = uarSystem->step(w, krokDt);
        }

        emit noweDaneSymulacji(czasGlobalny, wynik);
        czasGlobalny += krokDt;
        licznikKrokow++;
    }

private:
    SignalGenerator generator;
    ProstyUAR *uarSystem;
    QTimer *timerWewnetrzny;
    int licznikKrokow = 0;
    double czasGlobalny = 0.0;
    double krokDt = 0.01;
    bool trybSieciowy = false;
    bool jestemRegulatorem = false;
    double ostatnieY = 0.0;
};
