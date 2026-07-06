#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <QFile>
#include <QFileDialog>
#include <QMessageBox>
#include <QDateTime>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    QVector<double> startA = {-0.5};
    QVector<double> startB = {0.5};
    double Kp = 1.0, Ti = 0.0, Td = 0.0;

    uslugi = new WarstwaU(startA, startB, Kp, Ti, Td, this);
    serwer = new Serwer(this);
    klient = new Klient(this);

    watchdogTimer = new QTimer(this);
    connect(watchdogTimer, &QTimer::timeout, this, &MainWindow::slot_WatchdogTick);

    uslugi->setTempDelay(1);
    uslugi->commitARX();

    krokZatwierdzonyWInterfejsie = 10;
    ui->wartoscZadanaSpinBox->setValue(0.01);
    ui->wzmocnienieSpinBox->setValue(0.0);
    ui->czasCalkowaniaSpinBox->setValue(0.0);

    ui->label_10->setVisible(false);
    ui->wypelnienieSpinBox->setVisible(false);
    ui->zakresOknaSpinBox->setKeyboardTracking(false);

    inicjalizujWykresy();

    connect(uslugi, &WarstwaU::noweDaneSymulacji, this, &MainWindow::odbierzDaneSymulacji);

    auto podepnij = [&](QDoubleSpinBox *box)
    {
        connect(box, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
                this, &MainWindow::przeslijParametryDoWarstwyU);
    };
    podepnij(ui->wzmocnienieSpinBox);
    podepnij(ui->czasCalkowaniaSpinBox);
    podepnij(ui->czasRozniczkaSpinBox);
    connect(ui->zakresOknaSpinBox, &QDoubleSpinBox::editingFinished, this, &MainWindow::odswiezOX);
    podepnij(ui->amplitudaSpinBox);
    podepnij(ui->okresSpinBox);
    podepnij(ui->offsetSpinBox);
    podepnij(ui->wartoscZadanaSpinBox);

    auto obslugaPingu = [this](qint64 ping)
    {
        if (!trybSieciowy)
            return;

        double interwalMs = uslugi->getKrokDt() * 1000.0;

        ui->buttonSiec->setText(QString("Połączono (Ping: %1 ms)").arg(ping));

        if (ping > interwalMs)
        {

            ui->buttonSiec->setStyleSheet("background-color: lightcoral; color: black; font-weight: bold;");
        }
        else
        {

            ui->buttonSiec->setStyleSheet("background-color: lightgreen; color: black; font-weight: normal;");
        }
    };

    connect(ui->radioButton_Sinusoidalny, &QRadioButton::toggled, this, &MainWindow::przeslijParametryDoWarstwyU);
    connect(ui->radioButton_Prostokatny, &QRadioButton::toggled, this, &MainWindow::przeslijParametryDoWarstwyU);
    connect(ui->sposobLiczeniacomboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::przeslijParametryDoWarstwyU);

    connect(serwer, &Serwer::newClientConnected, this, [this](QString adr)
            {
        ui->buttonSiec->setText("Połączono (Klient: " + adr + ")");
        ui->buttonSiec->setStyleSheet("background-color: lightgreen; color: black;");


        QByteArray payload;
        QDataStream s(&payload, QIODevice::WriteOnly);
        s << QString("Obiekt") << adr;
        serwer->sendMessage(zbudujRamke(HANDSHAKE, 0, payload));
        wyslijARX(); });

    connect(klient, &Klient::connected, this, [this](QString adr, int port)
            {
                ui->buttonSiec->setText("Połączono z Serwerem: " + adr);
                ui->buttonSiec->setStyleSheet("background-color: lightgreen; color: black;");

                QByteArray payload;
                QDataStream s(&payload, QIODevice::WriteOnly);
                s << QString("Regulator") << adr;
                klient->sendMessage(zbudujRamke(HANDSHAKE, 0, payload)); });

    connect(serwer, &Serwer::odebranoHandshake, this, [this](QString rola, QString ip)
            {
        QString info = "Połączono!\nDruga instancja: " + rola + "\nJej adres: " + ip;
        ui->buttonSiec->setToolTip(info);
        QMessageBox::information(this, "Handshake", info); });
    connect(klient, &Klient::odebranoHandshake, this, [this](QString rola, QString ip)
            {
        QString info = "Połączono!\nDruga instancja: " + rola + "\nJej adres: " + ip;
        ui->buttonSiec->setToolTip(info);
        QMessageBox::information(this, "Handshake", info); });

    connect(serwer, &Serwer::odebranoKonfiguracjeJSON, this, &MainWindow::aplikujKonfiguracjeJSON);
    connect(serwer, &Serwer::odebranoKomendeSterujaca, this, &MainWindow::wykonajKomendeSterujaca);
    connect(klient, &Klient::odebranoKonfiguracjeJSON, this, &MainWindow::aplikujKonfiguracjeJSON);
    connect(klient, &Klient::odebranoKomendeSterujaca, this, &MainWindow::wykonajKomendeSterujaca);

    connect(serwer, &Serwer::odebranoSterowanie, this, &MainWindow::odbierzSterowanieZSieci);
    connect(klient, &Klient::odebranoSterowanie, this, &MainWindow::odbierzSterowanieZSieci);
    connect(serwer, &Serwer::odebranoWyjscie, this, &MainWindow::odbierzWyjscieZSieci);
    connect(klient, &Klient::odebranoWyjscie, this, &MainWindow::odbierzWyjscieZSieci);

    connect(uslugi, &WarstwaU::wyslijSterowanie, this, [this](quint32 id, double u, double w)
            {
        if (!jestemRegulatorem || !trybSieciowy) return;

        czasyWyslania[id] = QDateTime::currentMSecsSinceEpoch();

        QByteArray payload;
        QDataStream s(&payload, QIODevice::WriteOnly);
        s << u << w;
        klient->sendMessage(zbudujRamke(SIM_DATA_CTRL, id, payload)); });

    connect(serwer, &Serwer::clientDisconnected, this, &MainWindow::obsluzRozlaczenie);
    connect(klient, &Klient::disconnected, this, &MainWindow::obsluzRozlaczenie);

    connect(klient, &Klient::odebranoARX, this, &MainWindow::aplikujOdebranyARXJson);
    connect(serwer, &Serwer::odebranoARX, this, &MainWindow::aplikujOdebranyARXJson);

    przeslijParametryDoWarstwyU();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::przeslijParametryDoWarstwyU()
{

    double dt = ui->wartoscZadanaSpinBox->value();
    int ms = static_cast<int>(dt);
    if (ms < 10)
        ms = 10;
    double dt_s = ms / 1000.0;
    krokZatwierdzonyWInterfejsie = dt_s;

    uslugi->ustawKrokCzasu(krokZatwierdzonyWInterfejsie);
    uslugi->ustawInterwalTimera(ms);

    if (watchdogTimer->isActive())
    {
        watchdogTimer->start(ms + 15);
    }

    uslugi->setTempKp(ui->wzmocnienieSpinBox->value());
    uslugi->setTempTi(ui->czasCalkowaniaSpinBox->value());
    uslugi->setTempTd(ui->czasRozniczkaSpinBox->value());

    if (ui->sposobLiczeniacomboBox->currentIndex() == 0)
        uslugi->setTempLiczCalk(LiczCalk::Zew);
    else
        uslugi->setTempLiczCalk(LiczCalk::Wew);

    uslugi->commitPID();

    uslugi->setTempGenOffset(ui->offsetSpinBox->value());
    uslugi->setTempGenAmplitude(ui->amplitudaSpinBox->value());
    uslugi->setTempGenDuty(ui->wypelnienieSpinBox->value());

    double T_sek = ui->okresSpinBox->value();
    int T_probki = static_cast<int>(T_sek / krokZatwierdzonyWInterfejsie);
    if (T_probki < 1)
        T_probki = 1;
    uslugi->setTempGenPeriod(T_probki);

    if (ui->radioButton_Prostokatny->isChecked())
        uslugi->setTempGenType(SignalGenerator::Type::KROK);
    else
        uslugi->setTempGenType(SignalGenerator::Type::SIN);

    uslugi->commitGenerator();

    wyslijKonfiguracje();
}

void MainWindow::odbierzDaneSymulacji(double czas, const DaneSymulacji &dane)
{
    if (trybSieciowy && !jestemRegulatorem)
    {
        return;
    }

    aktualizujWykresy(czas, dane);
}

void MainWindow::inicjalizujWykresy()
{
    auto setup = [&](QFrame *f, QString title) -> QCustomPlot *
    {
        if (!f->layout())
        {
            QVBoxLayout *l = new QVBoxLayout(f);
            l->setContentsMargins(0, 0, 0, 0);
        }
        QCustomPlot *p = new QCustomPlot(f);
        f->layout()->addWidget(p);
        p->setBackground(Qt::black);
        p->yAxis->setLabel(title);
        p->xAxis->setLabel("Czas [s]");
        p->xAxis->setBasePen(QPen(Qt::white));
        p->yAxis->setBasePen(QPen(Qt::white));
        p->xAxis->setTickPen(QPen(Qt::white));
        p->yAxis->setTickPen(QPen(Qt::white));
        p->xAxis->setTickLabelColor(Qt::white);
        p->yAxis->setTickLabelColor(Qt::white);
        p->xAxis->setLabelColor(Qt::white);
        p->yAxis->setLabelColor(Qt::white);
        p->xAxis->setRange(0, ui->zakresOknaSpinBox->value());
        return p;
    };

    plotZadanaWyjscie = setup(ui->wykres1, "y(t), w(t)");
    plotZadanaWyjscie->addGraph();
    plotZadanaWyjscie->graph(0)->setPen(QPen(Qt::green, 1));
    plotZadanaWyjscie->graph(0)->setName("Wyjście y(t)");
    plotZadanaWyjscie->addGraph();
    plotZadanaWyjscie->graph(1)->setPen(QPen(Qt::red, 1));
    plotZadanaWyjscie->graph(1)->setName("Zadana w(t)");
    plotZadanaWyjscie->legend->setVisible(true);
    plotZadanaWyjscie->legend->setBrush(QBrush(QColor(255, 255, 255, 150)));
    plotZadanaWyjscie->axisRect()->insetLayout()->setInsetAlignment(0, Qt::AlignTop | Qt::AlignLeft);

    plotUchyb = setup(ui->wykres2, "Uchyb e(t)");
    plotUchyb->addGraph();
    plotUchyb->graph(0)->setPen(QPen(Qt::yellow, 1));

    plotSterowanie = setup(ui->wykres3, "Sterowanie u(t)");
    plotSterowanie->addGraph();
    plotSterowanie->graph(0)->setPen(QPen(Qt::cyan, 1));

    if (ui->wykres4)
    {
        plotSkladowePID = setup(ui->wykres4, "Składowe P, I, D");
        plotSkladowePID->addGraph();
        plotSkladowePID->graph(0)->setPen(QPen(Qt::green, 1));
        plotSkladowePID->graph(0)->setName("P");
        plotSkladowePID->addGraph();
        plotSkladowePID->graph(1)->setPen(QPen(Qt::yellow, 1));
        plotSkladowePID->graph(1)->setName("I");
        plotSkladowePID->addGraph();
        plotSkladowePID->graph(2)->setPen(QPen(Qt::red, 1));
        plotSkladowePID->graph(2)->setName("D");
        plotSkladowePID->legend->setVisible(true);
        plotSkladowePID->legend->setBrush(QBrush(QColor(255, 255, 255, 150)));
        plotSkladowePID->axisRect()->insetLayout()->setInsetAlignment(0, Qt::AlignTop | Qt::AlignLeft);
    }
}

void MainWindow::odswiezOX()
{
    double okno = ui->zakresOknaSpinBox->value();
    auto ustaw = [&](QCustomPlot *p)
    {
        if (!p || p->graphCount() == 0)
            return;
        double t = 0.0;
        auto dane = p->graph(0)->data();
        if (!dane->isEmpty())
            t = (dane->constEnd() - 1)->key;

        if (t < okno)
            p->xAxis->setRange(0, okno);
        else
            p->xAxis->setRange(t - okno, t);

        p->replot(QCustomPlot::rpQueuedReplot);
    };
    ustaw(plotZadanaWyjscie);
    ustaw(plotUchyb);
    ustaw(plotSterowanie);
    if (plotSkladowePID)
        ustaw(plotSkladowePID);
}

void MainWindow::aktualizujWykresy(double t, const DaneSymulacji &d)
{
    const double zapamietane = 51.0;
    auto add = [&](QCustomPlot *p, int g, double v)
    {
        if (!p)
            return;
        if (!std::isfinite(v))
            v = 0.0;
        v = qBound(-1000.0, v, 1000.0);
        p->graph(g)->addData(t, v);
        p->graph(g)->data()->removeBefore(t - zapamietane);
    };

    add(plotZadanaWyjscie, 0, d.y);
    add(plotZadanaWyjscie, 1, d.w);
    add(plotUchyb, 0, d.e);
    add(plotSterowanie, 0, d.u);

    if (plotSkladowePID)
    {
        add(plotSkladowePID, 0, d.P);
        add(plotSkladowePID, 1, d.I);
        add(plotSkladowePID, 2, d.D);
    }

    odswiezOX();

    auto bezpiecznyRescale = [](QCustomPlot *p, double minSpan = 10.0)
    {
        if (!p)
            return;
        QCPRange widocznyX = p->xAxis->range();
        bool found = false;
        QCPRange range = p->graph(0)->getValueRange(found, QCP::sdBoth, widocznyX);
        for (int i = 1; i < p->graphCount(); ++i)
        {
            bool f;
            QCPRange r = p->graph(i)->getValueRange(f, QCP::sdBoth, widocznyX);
            if (f)
            {
                if (!found)
                {
                    range = r;
                    found = true;
                }
                else
                    range.expand(r);
            }
        }
        if (!found || range.size() < 0.001)
            p->yAxis->setRange(-minSpan / 2.0, minSpan / 2.0);
        else
        {
            double margin = range.size() * 0.15;
            p->yAxis->setRange(range.lower - margin, range.upper + margin);
        }
        if (!std::isfinite(p->yAxis->range().lower))
            p->yAxis->setRange(-5, 5);
        p->replot(QCustomPlot::rpQueuedReplot);
    };

    bezpiecznyRescale(plotZadanaWyjscie);
    bezpiecznyRescale(plotUchyb);
    bezpiecznyRescale(plotSterowanie);
    if (plotSkladowePID)
        bezpiecznyRescale(plotSkladowePID);
}

void MainWindow::on_buttonStart_clicked()
{
    przeslijParametryDoWarstwyU();
    int ms = static_cast<int>(ui->wartoscZadanaSpinBox->value());
    if (ms < 10)
        ms = 10;
    krokZatwierdzonyWInterfejsie = ms / 1000.0;
    uslugi->startSymulacji(ms, krokZatwierdzonyWInterfejsie);

    if (trybSieciowy)
    {
        ostatnieIdSterowania = 0;
        ostatnieIdWyjscia = 0;
        licznikZgubionychKlatek = 0;
        watchdogTimer->start(ms + 15);

        if (jestemRegulatorem)
        {
            QByteArray payload;
            QDataStream stream(&payload, QIODevice::WriteOnly);
            stream << (quint8)AKCJA_START;
            stream << (quint32)ms;
            klient->sendMessage(zbudujRamke(SIM_CTRL, 0, payload));
        }
    }
}

void MainWindow::on_buttonStop_clicked()
{
    watchdogTimer->stop();
    uslugi->stopSymulacji();
    if (trybSieciowy && jestemRegulatorem)
    {
        QByteArray payload;
        QDataStream stream(&payload, QIODevice::WriteOnly);
        stream << (quint8)AKCJA_STOP;
        klient->sendMessage(zbudujRamke(SIM_CTRL, 0, payload));
    }
}

void MainWindow::on_buttonReset_clicked()
{
    watchdogTimer->stop();
    uslugi->zresetujStanSymulacji();
    double zakres = ui->zakresOknaSpinBox->value();

    plotZadanaWyjscie->graph(0)->data()->clear();
    plotZadanaWyjscie->graph(1)->data()->clear();
    plotZadanaWyjscie->xAxis->setRange(0, zakres);
    plotUchyb->graph(0)->data()->clear();
    plotUchyb->xAxis->setRange(0, zakres);
    plotSterowanie->graph(0)->data()->clear();
    plotSterowanie->xAxis->setRange(0, zakres);

    if (plotSkladowePID)
    {
        for (int i = 0; i < plotSkladowePID->graphCount(); ++i)
            plotSkladowePID->graph(i)->data()->clear();
        plotSkladowePID->xAxis->setRange(0, zakres);
        plotSkladowePID->replot();
    }
    plotZadanaWyjscie->replot();
    plotUchyb->replot();
    plotSterowanie->replot();

    przeslijParametryDoWarstwyU();

    if (trybSieciowy && jestemRegulatorem)
    {
        QByteArray payload;
        QDataStream stream(&payload, QIODevice::WriteOnly);
        stream << (quint8)AKCJA_RESET;
        klient->sendMessage(zbudujRamke(SIM_CTRL, 0, payload));
    }
}

void MainWindow::on_buttonCalkaReset_clicked()
{
    uslugi->resetPID();
}

void MainWindow::on_buttonARX_clicked()
{
    oknoARX okno(this);
    okno.ustawDane(
        uslugi->getSigma(),
        uslugi->getDelay(),
        uslugi->getA(),
        uslugi->getB(),
        uslugi->getUMin(), uslugi->getUMax(),
        uslugi->getYMin(), uslugi->getYMax(),
        uslugi->getLimitsEnabled());

    if (okno.exec() == QDialog::Accepted)
    {
        uslugi->setTempA(okno.getWielomianA());
        uslugi->setTempB(okno.getWielomianB());
        uslugi->setTempDelay(okno.getOpoznienie());
        uslugi->setTempSigma(okno.getOdchylenie());
        uslugi->setTempControlLimits(okno.getUMin(), okno.getUMax());
        uslugi->setTempOutputLimits(okno.getYMin(), okno.getYMax());
        uslugi->setTempLimitsEnabled(okno.getLimitsEnabled());
        uslugi->commitARX();
        wyslijARX();

        przeslijParametryDoWarstwyU();
    }
}

void MainWindow::on_buttonSiec_clicked()
{
    if (trybSieciowy)
    {

        auto odp = QMessageBox::question(this, "Rozłącz",
                                         "Czy na pewno chcesz rozłączyć się i wrócić do trybu stacjonarnego?");
        if (odp == QMessageBox::Yes)
        {
            if (jestemRegulatorem)
                klient->disconnectFrom();
            else
                serwer->stopListening();
            obsluzRozlaczenie();
        }
        return;
    }

    oknosiec okno(this);
    if (okno.exec() == QDialog::Accepted)
    {
        bool jakoRegulator = okno.jestRegulatorem();
        QString ip = okno.getIP();

        jestemRegulatorem = jakoRegulator;
        trybSieciowy = true;

        uslugi->ustawTrybSieciowy(true, jakoRegulator);

        if (jakoRegulator)
            klient->connectTo(ip, 12345);
        else
            serwer->startListening(12345);

        aktualizujBlokadyGUI();
    }
}

void MainWindow::aktualizujBlokadyGUI()
{
    if (!trybSieciowy)
    {
        ui->groupBox_PID->setEnabled(true);
        ui->groupBox_GENERATOR->setEnabled(true);
        ui->buttonStart->setEnabled(true);
        ui->buttonStop->setEnabled(true);
        ui->buttonARX->setEnabled(true);
    }
    else if (jestemRegulatorem)
    {

        ui->groupBox_PID->setEnabled(true);
        ui->groupBox_GENERATOR->setEnabled(true);
        ui->buttonStart->setEnabled(true);
        ui->buttonStop->setEnabled(true);
        ui->buttonARX->setEnabled(false);
    }
    else
    {

        ui->groupBox_PID->setEnabled(false);
        ui->groupBox_GENERATOR->setEnabled(false);
        ui->buttonStart->setEnabled(false);
        ui->buttonStop->setEnabled(false);
        ui->buttonARX->setEnabled(true);
    }
}

void MainWindow::odbierzSterowanieZSieci(quint32 id, double u, double w)
{

    if (!uslugi->czySymulacjaTrwa())
        return;

    if (!jestemRegulatorem && trybSieciowy)
    {
        if (id <= ostatnieIdSterowania && id != 0)
        {
            qDebug() << "Odrzucono spozniona klatke ID: " << id;
            return;
        }

        ostatnieIdSterowania = id;

        ostatnieU = u;
        ostatnieW = w;
        licznikZgubionychKlatek = 0;

        watchdogTimer->start((uslugi->getKrokDt() * 1000.0) + 15);

        double y = uslugi->krokObiektu(u);
        uslugi->synchronizujCzasObiektu(id);

        DaneSymulacji dane = {w, y, w - y, u, 0.0, 0.0, 0.0};
        aktualizujWykresy(czasObiekt, dane);
        czasObiekt += uslugi->getKrokDt();

        QByteArray payload;
        QDataStream s(&payload, QIODevice::WriteOnly);
        s << y;
        serwer->sendMessage(zbudujRamke(SIM_DATA_OBJ, id, payload));
    }
}

void MainWindow::odbierzWyjscieZSieci(quint32 id, double y)
{

    if (!uslugi->czySymulacjaTrwa())
        return;

    if (jestemRegulatorem && trybSieciowy)
    {

        if (id <= ostatnieIdWyjscia && id != 0)
        {
            qDebug() << "Odrzucono spozniona klatke ID: " << id;
            return;
        }

        ostatnieIdWyjscia = id;

        ostatnieY = y;
        licznikZgubionychKlatek = 0;

        watchdogTimer->start((uslugi->getKrokDt() * 1000.0) + 15);

        uslugi->ustawOstatnieY(y);
    }

    if (czasyWyslania.contains(id))
    {
        qint64 ping = QDateTime::currentMSecsSinceEpoch() - czasyWyslania.take(id);

        ui->buttonSiec->setText(QString("Połączono (Ping RTT: %1 ms)").arg(ping));

        double interwalMs = uslugi->getKrokDt() * 1000.0;
        if (ping > interwalMs)
        {
            ui->buttonSiec->setStyleSheet("background-color: lightcoral; color: black; font-weight: bold;");
        }
        else
        {
            ui->buttonSiec->setStyleSheet("background-color: lightgreen; color: black; font-weight: normal;");
        }
    }
}

void MainWindow::obsluzRozlaczenie()
{
    watchdogTimer->stop();
    trybSieciowy = false;
    jestemRegulatorem = false;

    uslugi->ustawTrybSieciowy(false, false);
    aktualizujBlokadyGUI();

    ui->buttonSiec->setText("Połącz przez sieć");
    ui->buttonSiec->setStyleSheet("");

    if (!uslugi->czySymulacjaTrwa())
    {
        int ms = static_cast<int>(ui->wartoscZadanaSpinBox->value());
        if (ms < 10)
            ms = 10;
        uslugi->startSymulacji(ms, ms / 1000.0);
    }

    QMessageBox::warning(this, "Sieć",
                         "Połączenie sieciowe zostało zerwane!\nAplikacja przeszła w tryb stacjonarny.");
}

void MainWindow::aplikujOdebranyARXJson(QByteArray json)
{
    QJsonObject a = QJsonDocument::fromJson(json).object();
    QVector<double> vecA, vecB;
    for (auto v : a["A"].toArray())
        vecA.push_back(v.toDouble());
    for (auto v : a["B"].toArray())
        vecB.push_back(v.toDouble());

    uslugi->setTempA(vecA);
    uslugi->setTempB(vecB);
    uslugi->setTempDelay(a["Delay"].toInt(1));
    uslugi->setTempSigma(a["Sigma"].toDouble(0.0));
    uslugi->setTempControlLimits(a["u_min"].toDouble(-10.0), a["u_max"].toDouble(10.0));
    uslugi->setTempOutputLimits(a["y_min"].toDouble(-10.0), a["y_max"].toDouble(10.0));
    uslugi->setTempLimitsEnabled(a["limits_on"].toBool(true));
    uslugi->commitARX();

    qInfo() << "Regulator: zaktualizowano ARX z Obiektu.";
}

void MainWindow::on_buttonZapisz_clicked()
{
    QString plik = QFileDialog::getSaveFileName(this, "Zapisz", "", "JSON (*.json)");
    if (!plik.isEmpty())
        zapiszDoPliku(plik);
}

void MainWindow::zapiszDoPliku(QString sciezka)
{
    QJsonObject root;

    QJsonObject pidObj;
    pidObj["Kp"] = ui->wzmocnienieSpinBox->value();
    pidObj["Ti"] = ui->czasCalkowaniaSpinBox->value();
    pidObj["Td"] = ui->czasRozniczkaSpinBox->value();
    pidObj["Metoda"] = ui->sposobLiczeniacomboBox->currentIndex();
    root["PID"] = pidObj;

    QJsonObject genObj;
    genObj["Amplituda"] = ui->amplitudaSpinBox->value();
    genObj["InterwalGen"] = ui->wartoscZadanaSpinBox->value();
    genObj["Offset"] = ui->offsetSpinBox->value();
    genObj["Okres"] = ui->okresSpinBox->value();
    genObj["Wypelnienie"] = ui->wypelnienieSpinBox->value();
    genObj["Okno"] = ui->zakresOknaSpinBox->value();
    genObj["Typ"] = ui->radioButton_Prostokatny->isChecked() ? "STEP" : "SINE";
    root["Generator"] = genObj;

    QJsonObject arxObj;
    QJsonArray arrA, arrB;
    for (double v : uslugi->getA())
        arrA.append(v);
    for (double v : uslugi->getB())
        arrB.append(v);
    arxObj["A"] = arrA;
    arxObj["B"] = arrB;
    arxObj["Delay"] = uslugi->getDelay();
    arxObj["Sigma"] = uslugi->getSigma();
    arxObj["u_min"] = uslugi->getUMin();
    arxObj["u_max"] = uslugi->getUMax();
    arxObj["y_min"] = uslugi->getYMin();
    arxObj["y_max"] = uslugi->getYMax();
    arxObj["limits_on"] = uslugi->getLimitsEnabled();
    root["ARX"] = arxObj;

    QFile file(sciezka);
    if (file.open(QIODevice::WriteOnly))
    {
        file.write(QJsonDocument(root).toJson());
        file.close();
        QMessageBox::information(this, "Info", "Zapisano pomyślnie.");
    }
}

void MainWindow::on_buttonOdczytaj_clicked()
{
    QString plik = QFileDialog::getOpenFileName(this, "Wczytaj", "", "JSON (*.json)");
    if (!plik.isEmpty())
        wczytajZPliku(plik);
}

void MainWindow::wczytajZPliku(QString sciezka)
{
    QFile file(sciezka);
    if (!file.open(QIODevice::ReadOnly))
        return;

    QJsonObject root = QJsonDocument::fromJson(file.readAll()).object();
    file.close();

    if (root.contains("PID"))
    {
        QJsonObject p = root["PID"].toObject();
        ui->wzmocnienieSpinBox->setValue(p["Kp"].toDouble());
        ui->czasCalkowaniaSpinBox->setValue(p["Ti"].toDouble());
        ui->czasRozniczkaSpinBox->setValue(p["Td"].toDouble());
        ui->sposobLiczeniacomboBox->setCurrentIndex(p["Metoda"].toInt());
    }

    if (root.contains("Generator"))
    {
        QJsonObject g = root["Generator"].toObject();
        ui->amplitudaSpinBox->setValue(g["Amplituda"].toDouble());
        if (g.contains("Offset"))
            ui->offsetSpinBox->setValue(g["Offset"].toDouble());
        ui->okresSpinBox->setValue(g["Okres"].toDouble());
        if (g.contains("Wypelnienie"))
            ui->wypelnienieSpinBox->setValue(g["Wypelnienie"].toDouble());
        if (g.contains("InterwalGen"))
            ui->wartoscZadanaSpinBox->setValue(g["InterwalGen"].toDouble());
        if (g.contains("Okno"))
            ui->zakresOknaSpinBox->setValue(g["Okno"].toDouble());

        if (g["Typ"].toString() == "STEP")
        {
            ui->radioButton_Prostokatny->setChecked(true);
            ui->label_10->setVisible(true);
            ui->wypelnienieSpinBox->setVisible(true);
        }
        else
        {
            ui->radioButton_Sinusoidalny->setChecked(true);
            ui->label_10->setVisible(false);
            ui->wypelnienieSpinBox->setVisible(false);
        }
    }

    if (root.contains("ARX"))
    {
        QJsonObject a = root["ARX"].toObject();
        QVector<double> vecA, vecB;
        for (auto v : a["A"].toArray())
            vecA.push_back(v.toDouble());
        for (auto v : a["B"].toArray())
            vecB.push_back(v.toDouble());

        uslugi->setTempA(vecA);
        uslugi->setTempB(vecB);
        uslugi->setTempDelay(a["Delay"].toInt(1));
        uslugi->setTempSigma(a["Sigma"].toDouble(0.0));
        uslugi->setTempControlLimits(a["u_min"].toDouble(-10.0), a["u_max"].toDouble(10.0));
        uslugi->setTempOutputLimits(a["y_min"].toDouble(-10.0), a["y_max"].toDouble(10.0));
        uslugi->setTempLimitsEnabled(a["limits_on"].toBool(true));
        uslugi->commitARX();
    }

    przeslijParametryDoWarstwyU();
    QMessageBox::information(this, "Info", "Wczytano konfigurację.");
}

void MainWindow::on_radioButton_Prostokatny_toggled(bool checked)
{
    ui->label_10->setVisible(checked);
    ui->wypelnienieSpinBox->setVisible(checked);
}

QByteArray MainWindow::zbudujRamke(TypWiadomosci typ, quint32 idProbki, const QByteArray &payload)
{
    QByteArray ramka;
    QDataStream stream(&ramka, QIODevice::WriteOnly);
    stream << (quint16)0xAAAA
           << (quint8)typ
           << (quint32)idProbki
           << (qint64)QDateTime::currentMSecsSinceEpoch()
           << (quint32)payload.size();
    ramka.append(payload);
    return ramka;
}

void MainWindow::wyslijKonfiguracje()
{
    if (!trybSieciowy || odbieramZSieci || !jestemRegulatorem)
        return;

    QJsonObject root;

    QJsonObject pidObj;
    pidObj["Kp"] = ui->wzmocnienieSpinBox->value();
    pidObj["Ti"] = ui->czasCalkowaniaSpinBox->value();
    pidObj["Td"] = ui->czasRozniczkaSpinBox->value();
    pidObj["Metoda"] = ui->sposobLiczeniacomboBox->currentIndex();
    root["PID"] = pidObj;

    QJsonObject genObj;
    genObj["Amplituda"] = ui->amplitudaSpinBox->value();
    genObj["InterwalGen"] = ui->wartoscZadanaSpinBox->value();
    genObj["Offset"] = ui->offsetSpinBox->value();
    genObj["Okres"] = ui->okresSpinBox->value();
    genObj["Wypelnienie"] = ui->wypelnienieSpinBox->value();
    genObj["Typ"] = ui->radioButton_Prostokatny->isChecked() ? "STEP" : "SINE";
    root["Generator"] = genObj;

    QByteArray payload = QJsonDocument(root).toJson(QJsonDocument::Compact);
    klient->sendMessage(zbudujRamke(CONFIG_SYNC, 0, payload));
}

void MainWindow::aplikujKonfiguracjeJSON(QByteArray json)
{
    QJsonObject root = QJsonDocument::fromJson(json).object();
    odbieramZSieci = true;

    if (root.contains("PID"))
    {
        QJsonObject p = root["PID"].toObject();
        ui->wzmocnienieSpinBox->blockSignals(true);
        ui->czasCalkowaniaSpinBox->blockSignals(true);
        ui->czasRozniczkaSpinBox->blockSignals(true);
        ui->sposobLiczeniacomboBox->blockSignals(true);

        ui->wzmocnienieSpinBox->setValue(p["Kp"].toDouble());
        ui->czasCalkowaniaSpinBox->setValue(p["Ti"].toDouble());
        ui->czasRozniczkaSpinBox->setValue(p["Td"].toDouble());
        ui->sposobLiczeniacomboBox->setCurrentIndex(p["Metoda"].toInt());

        ui->wzmocnienieSpinBox->blockSignals(false);
        ui->czasCalkowaniaSpinBox->blockSignals(false);
        ui->czasRozniczkaSpinBox->blockSignals(false);
        ui->sposobLiczeniacomboBox->blockSignals(false);
    }

    if (root.contains("Generator"))
    {
        QJsonObject g = root["Generator"].toObject();

        ui->amplitudaSpinBox->blockSignals(true);
        ui->offsetSpinBox->blockSignals(true);
        ui->okresSpinBox->blockSignals(true);
        ui->wypelnienieSpinBox->blockSignals(true);
        ui->wartoscZadanaSpinBox->blockSignals(true);
        ui->radioButton_Prostokatny->blockSignals(true);
        ui->radioButton_Sinusoidalny->blockSignals(true);

        ui->amplitudaSpinBox->setValue(g["Amplituda"].toDouble());
        ui->offsetSpinBox->setValue(g["Offset"].toDouble());
        ui->okresSpinBox->setValue(g["Okres"].toDouble());
        ui->wypelnienieSpinBox->setValue(g["Wypelnienie"].toDouble());
        ui->wartoscZadanaSpinBox->setValue(g["InterwalGen"].toDouble());

        if (g["Typ"].toString() == "STEP")
        {
            ui->radioButton_Prostokatny->setChecked(true);
            ui->label_10->setVisible(true);
            ui->wypelnienieSpinBox->setVisible(true);
        }
        else
        {
            ui->radioButton_Sinusoidalny->setChecked(true);
            ui->label_10->setVisible(false);
            ui->wypelnienieSpinBox->setVisible(false);
        }

        ui->amplitudaSpinBox->blockSignals(false);
        ui->offsetSpinBox->blockSignals(false);
        ui->okresSpinBox->blockSignals(false);
        ui->wypelnienieSpinBox->blockSignals(false);
        ui->wartoscZadanaSpinBox->blockSignals(false);
        ui->radioButton_Prostokatny->blockSignals(false);
        ui->radioButton_Sinusoidalny->blockSignals(false);
    }

    if (root.contains("ARX") && jestemRegulatorem)
    {
        QJsonObject a = root["ARX"].toObject();
        QVector<double> vecA, vecB;
        for (auto v : a["A"].toArray())
            vecA.push_back(v.toDouble());
        for (auto v : a["B"].toArray())
            vecB.push_back(v.toDouble());

        uslugi->setTempA(vecA);
        uslugi->setTempB(vecB);
        uslugi->setTempDelay(a["Delay"].toInt(1));
        uslugi->setTempSigma(a["Sigma"].toDouble(0.0));
        uslugi->setTempControlLimits(a["u_min"].toDouble(-10.0), a["u_max"].toDouble(10.0));
        uslugi->setTempOutputLimits(a["y_min"].toDouble(-10.0), a["y_max"].toDouble(10.0));
        uslugi->setTempLimitsEnabled(a["limits_on"].toBool(true));
        uslugi->commitARX();
    }

    przeslijParametryDoWarstwyU();
    qInfo() << "Zaktualizowano konfigurację z sieci [JSON]!";
    odbieramZSieci = false;
}

void MainWindow::wykonajKomendeSterujaca(quint8 akcja, quint32 interwal)
{
    if (akcja == AKCJA_START)
    {
        qInfo() << "Odebrano komendę START z interwałem:" << interwal;
        ostatnieIdSterowania = 0;
        ostatnieIdWyjscia = 0;
        licznikZgubionychKlatek = 0;
        watchdogTimer->start(interwal + 15);
        uslugi->startSymulacji(interwal, interwal / 1000.0);
    }
    else if (akcja == AKCJA_STOP)
    {
        qInfo() << "Odebrano komendę STOP";
        watchdogTimer->stop();
        uslugi->stopSymulacji();
    }
    else if (akcja == AKCJA_RESET)
    {
        qInfo() << "Odebrano komendę RESET";
        on_buttonReset_clicked();

        if (!jestemRegulatorem)
        {
            czasObiekt = 0.0;
        }
    }
}
void MainWindow::wyslijARX()
{

    if (!trybSieciowy || jestemRegulatorem)
        return;

    QJsonObject arxObj;
    QJsonArray arrA, arrB;
    for (double v : uslugi->getA())
        arrA.append(v);
    for (double v : uslugi->getB())
        arrB.append(v);
    arxObj["A"] = arrA;
    arxObj["B"] = arrB;
    arxObj["Delay"] = uslugi->getDelay();
    arxObj["Sigma"] = uslugi->getSigma();
    arxObj["u_min"] = uslugi->getUMin();
    arxObj["u_max"] = uslugi->getUMax();
    arxObj["y_min"] = uslugi->getYMin();
    arxObj["y_max"] = uslugi->getYMax();
    arxObj["limits_on"] = uslugi->getLimitsEnabled();

    QByteArray payload = QJsonDocument(arxObj).toJson(QJsonDocument::Compact);
    serwer->sendMessage(zbudujRamke(ARX_SYNC, 0, payload));
}

void MainWindow::slot_WatchdogTick()
{
    if (!trybSieciowy)
        return;

    licznikZgubionychKlatek++;
    qWarning() << "Brak klatki na czas! ZOH. Zgubione pod rząd:" << licznikZgubionychKlatek;

    if (licznikZgubionychKlatek >= 100)
    {
        watchdogTimer->stop();
        QMessageBox::critical(this, "Błąd Krytyczny", "Zbyt dużo zgubionych klatek (Timeout). Aplikacja została rozłączona.");

        if (jestemRegulatorem)
            klient->disconnectFrom();
        else
            serwer->stopListening();

        obsluzRozlaczenie();
        return;
    }

    if (!jestemRegulatorem)
    {
        ostatnieIdSterowania++;

        double y = uslugi->krokObiektu(ostatnieU);
        uslugi->synchronizujCzasObiektu(ostatnieIdSterowania);

        DaneSymulacji dane = {ostatnieW, y, ostatnieW - y, ostatnieU, 0.0, 0.0, 0.0};
        aktualizujWykresy(czasObiekt, dane);
        czasObiekt += uslugi->getKrokDt();

        QByteArray payload;
        QDataStream s(&payload, QIODevice::WriteOnly);
        s << y;
        serwer->sendMessage(zbudujRamke(SIM_DATA_OBJ, ostatnieIdSterowania, payload));
    }
    else
    {
        ostatnieIdWyjscia++;
    }
}

void MainWindow::aplikujOdebranyPID(RegulatorPID pid)
{
    ui->wzmocnienieSpinBox->blockSignals(true);
    ui->czasCalkowaniaSpinBox->blockSignals(true);
    ui->czasRozniczkaSpinBox->blockSignals(true);
    ui->sposobLiczeniacomboBox->blockSignals(true);

    ui->wzmocnienieSpinBox->setValue(pid.getKp());
    ui->czasCalkowaniaSpinBox->setValue(pid.getTi());
    ui->czasRozniczkaSpinBox->setValue(pid.getTd());
    if (pid.getLiczCalk() == LiczCalk::Zew)
        ui->sposobLiczeniacomboBox->setCurrentIndex(0);
    else
        ui->sposobLiczeniacomboBox->setCurrentIndex(1);

    ui->wzmocnienieSpinBox->blockSignals(false);
    ui->czasCalkowaniaSpinBox->blockSignals(false);
    ui->czasRozniczkaSpinBox->blockSignals(false);
    ui->sposobLiczeniacomboBox->blockSignals(false);
}

void MainWindow::aplikujOdebranyARX(ModelARX arx)
{
    uslugi->setTempA(arx.getA());
    uslugi->setTempB(arx.getB());
    uslugi->setTempDelay(arx.getDelay());
    uslugi->setTempSigma(arx.getSigma());
    uslugi->setTempControlLimits(arx.getUMin(), arx.getUMax());
    uslugi->setTempOutputLimits(arx.getYMin(), arx.getYMax());
    uslugi->setTempLimitsEnabled(arx.getLimitsEnabled());
    uslugi->commitARX();
}
