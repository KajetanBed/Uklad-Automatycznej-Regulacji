#include "mainwindow.h"
#include "serwer.h"
#include "klient.h"
#include "RegulatorPID.h"
#include "ModelARX.h"

#include <QApplication>
#include <QCoreApplication>
#include <QDataStream>

int main(int argc, char *argv[])
{
    if (argc > 1)
    {
        QString tryb = argv[1];
        if (tryb == "-serwer")
        {
            QCoreApplication a(argc, argv);
            qInfo() << "--- START: Tryb konsolowy (SERWER) ---";
            Serwer serwer;



            if (serwer.startListening(12345))
            {
                qInfo() << "Serwer nasluchuje na porcie 12345...";
            }
            else
            {
                qInfo() << "Blad: Nie mozna uruchomic serwera!";
                return -1;
            }

            if (serwer.czekajNaKlienta(-1))
            {
                qInfo() << "Klient podlaczony! Tworze i wysylam konfig ARX...";

                ModelARX arx({0.5, 1.0, 2.4}, {0.6, 0.7, 0.67}, 2, 0.5);
                arx.setControlLimits(6.0, 7.0);
                arx.setOutputLimits(7.0, 6.0);
                arx.setLimitsEnabled(true);
                QByteArray ramka;
                QDataStream stream(&ramka, QIODevice::WriteOnly);
                stream << (quint8)2;
                stream << arx;

                serwer.sendMessage(ramka);
                serwer.pobierzPolaczenie()->waitForBytesWritten();
                qInfo() << "Konfiguracja ARX zostala wyslana pomyslnie!";
            }

            return a.exec();
        }
        else if (tryb == "-klient")
        {
            QCoreApplication a(argc, argv);
            qInfo() << "--- START: Tryb konsolowy (KLIENT) ---";

            Klient klient;
            QString ipAddress = "127.0.0.1";
            if (argc > 2)
            {
                ipAddress = argv[2];
            }



            klient.connectTo(ipAddress, 12345);

            if (klient.czekajNaPolaczenie(3000))
            {
                qInfo() << "Polaczono!";

                RegulatorPID testowyPID(2.5, 1.0, 0.5);
                testowyPID.setLiczCalk(LiczCalk::Wew);
                QByteArray ramka;
                QDataStream stream(&ramka, QIODevice::WriteOnly);
                stream << (quint8)1;
                stream << testowyPID;
                klient.sendMessage(ramka);
                qInfo() << "Wiadomosc wyslana!";
            }
            else
            {
                qInfo() << "Blad: Nie udalo sie polaczyc z serwerem w ciagu 3 sekund.";
                return -1;
            }
            return a.exec();
        }
    }

    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return a.exec();
}
