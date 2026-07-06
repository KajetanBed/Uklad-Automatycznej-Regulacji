#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFileDialog>
#include <QMessageBox>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <QVBoxLayout>
#include <QHash>

#include "WarstwaU.h"
#include "oknoarx.h"
#include "qcustomplot.h"
#include "oknosiec.h"

#include "serwer.h"
#include "klient.h"

QT_BEGIN_NAMESPACE
namespace Ui
{
    class MainWindow;
}
QT_END_NAMESPACE
enum TypWiadomosci
{
    HANDSHAKE = 0x01,
    SIM_DATA_CTRL = 0x02,
    SIM_DATA_OBJ = 0x03,
    SIM_CTRL = 0x04,
    CONFIG_SYNC = 0x05,
    ARX_SYNC = 0x06
};
enum Akcje
{
    AKCJA_START = 1,
    AKCJA_STOP = 2,
    AKCJA_RESET = 3
};
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_buttonStart_clicked();
    void on_buttonStop_clicked();
    void on_buttonReset_clicked();
    void on_buttonCalkaReset_clicked();
    void on_buttonARX_clicked();
    void on_buttonSiec_clicked();
    void odswiezOX();
    void on_buttonZapisz_clicked();
    void on_buttonOdczytaj_clicked();
    void odbierzDaneSymulacji(double czas, const DaneSymulacji &dane);
    void przeslijParametryDoWarstwyU();
    void on_radioButton_Prostokatny_toggled(bool checked);
    void aktualizujBlokadyGUI();
    void odbierzSterowanieZSieci(quint32 id, double u, double w);
    void odbierzWyjscieZSieci(quint32 id, double y);
    void obsluzRozlaczenie();
    void aplikujOdebranyPID(RegulatorPID pid);
    void aplikujOdebranyARX(ModelARX arx);
    void aplikujKonfiguracjeJSON(QByteArray json);
    void wykonajKomendeSterujaca(quint8 akcja, quint32 interwal);

    void aplikujOdebranyARXJson(QByteArray json);

    void slot_WatchdogTick();

private:
    Ui::MainWindow *ui;
    WarstwaU *uslugi;

    QTimer *watchdogTimer = nullptr;

    quint32 ostatnieIdSterowania = 0;
    quint32 ostatnieIdWyjscia = 0;

    int licznikZgubionychKlatek = 0;

    double ostatnieU = 0.0;
    double ostatnieW = 0.0;
    double ostatnieY = 0.0;
    QCustomPlot *plotZadanaWyjscie = nullptr;
    QCustomPlot *plotUchyb = nullptr;
    QCustomPlot *plotSterowanie = nullptr;
    QCustomPlot *plotSkladowePID = nullptr;
    double krokZatwierdzonyWInterfejsie = 0.01;
    void inicjalizujWykresy();
    void aktualizujWykresy(double t, const DaneSymulacji &d);
    void zapiszDoPliku(QString sciezka);
    void wczytajZPliku(QString sciezka);
    Serwer *serwer = nullptr;
    Klient *klient = nullptr;
    bool trybSieciowy = false;
    bool jestemRegulatorem = false;
    bool odbieramZSieci = false;
    double czasObiekt = 0.0;
    QByteArray zbudujRamke(TypWiadomosci typ, quint32 idProbki, const QByteArray &payload);

    QHash<quint32, quint64> czasyWyslania;
    void wyslijKonfiguracje();

    void wyslijARX();
};

#endif
