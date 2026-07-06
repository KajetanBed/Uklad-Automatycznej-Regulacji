#ifndef OKNOARX_H
#define OKNOARX_H

#include <QDialog>
#include <QList>
#include <QDoubleSpinBox>
#include <QScrollArea>
#include <vector>
namespace Ui
{
    class oknoARX;
}

class oknoARX : public QDialog
{
    Q_OBJECT

public:
    explicit oknoARX(QWidget *parent = nullptr);
    ~oknoARX();

    QVector<double> getWielomianA() const;
    QVector<double> getWielomianB() const;
    double getOdchylenie() const;
    int getOpoznienie() const;
    double getUMin() const;
    double getUMax() const;
    double getYMin() const;
    double getYMax() const;
    bool getLimitsEnabled() const;
    void ustawDane(double odch, int opoz, QVector<double> A, QVector<double> B, double uMin, double uMax, double yMin, double yMax, bool enabled);

private slots:
    void on_buttonZapiszARX_clicked();
    void on_buttonAnulujARX_clicked();

private:
    Ui::oknoARX *ui;
    QList<QDoubleSpinBox *> polaWspolczynnikowA;
    QList<QDoubleSpinBox *> polaWspolczynnikowB;
    void wyczyscDynamicznePola();
    void zbudujListePola(const QVector<double> &wektor, QList<QDoubleSpinBox *> *lista, QScrollArea *cel);
};

#endif
