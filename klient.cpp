#include "klient.h"
#include <QDateTime>

Klient::Klient(QObject *parent) : QObject{parent}, _socket(this)
{
    connect(&_socket, SIGNAL(connected()), this, SLOT(slot_connected()));
    connect(&_socket, SIGNAL(disconnected()), this, SIGNAL(disconnected()));
    connect(&_socket, SIGNAL(readyRead()), this, SLOT(slot_readyRead()));
}

void Klient::connectTo(QString address, int port)
{
    _ipAddress = address;
    _port = port;
    _bufor.clear();
    _socket.connectToHost(_ipAddress, port);
}

void Klient::disconnectFrom()
{
    _bufor.clear();
    _socket.close();
}

void Klient::sendMessage(QByteArray msg)
{
    _socket.write(msg);
}

void Klient::slot_connected()
{
    _socket.setSocketOption(QAbstractSocket::LowDelayOption, 1);
    emit connected(_ipAddress, _port);
}

void Klient::slot_readyRead()
{
    _bufor.append(_socket.readAll());

    while (true)
    {
        if (_bufor.size() < 19)
            return;

        QDataStream s(_bufor);
        quint16 znacznik;
        s >> znacznik;

        if (znacznik != 0xAAAA)
        {
            qWarning() << "Klient: zły znacznik — czyszczę bufor!";
            _bufor.clear();
            return;
        }

        quint8 typ;
        quint32 id, dlugosc;
        qint64 timestamp;

        s >> typ >> id >> timestamp >> dlugosc;

        if (_bufor.size() < 19 + static_cast<int>(dlugosc))
            return;

        QByteArray payload = _bufor.mid(19, dlugosc);
        _bufor.remove(0, 19 + static_cast<int>(dlugosc));

        przetworzRamke(typ, id, payload);
    }
}
void Klient::przetworzRamke(quint8 typ, quint32 id, QByteArray payload)
{
    if (typ == 0x01)
    {
        QDataStream pStream(&payload, QIODevice::ReadOnly);
        QString rola, adresIP;
        pStream >> rola >> adresIP;
        emit odebranoHandshake(rola, adresIP);
    }
    else if (typ == 0x02)
    {
        QDataStream pStream(&payload, QIODevice::ReadOnly);
        double u, w;
        pStream >> u >> w;
        emit odebranoSterowanie(id, u, w);
    }
    else if (typ == 0x03)
    {
        QDataStream pStream(&payload, QIODevice::ReadOnly);
        double y;
        pStream >> y;
        emit odebranoWyjscie(id, y);
    }
    else if (typ == 0x04)
    {
        QDataStream pStream(&payload, QIODevice::ReadOnly);
        quint8 akcja;
        quint32 interwal = 0;
        pStream >> akcja;
        if (akcja == 1)
            pStream >> interwal;
        emit odebranoKomendeSterujaca(akcja, interwal);
    }
    else if (typ == 0x05)
    {
        emit odebranoKonfiguracjeJSON(payload);
    }
    else if (typ == 0x06)
    {
        emit odebranoARX(payload);
    }
    else
    {
        qWarning() << "Klient: nieznany typ wiadomości:" << typ;
    }
}
