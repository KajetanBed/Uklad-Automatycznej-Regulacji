#include "serwer.h"
#include <QDateTime>

Serwer::Serwer(QObject *parent) : QObject{parent}, _server(this)
{
    connect(&_server, SIGNAL(newConnection()), this, SLOT(slot_new_client()));
}

bool Serwer::startListening(int port)
{
    _port = port;
    _isListening = _server.listen(QHostAddress::AnyIPv4, port);
    return _isListening;
}

void Serwer::stopListening()
{
    if (_client)
    {
        _client->disconnectFromHost();
        _client = nullptr;
    }
    _server.close();
    _isListening = false;
    _bufor.clear();
}

void Serwer::sendMessage(QByteArray msg)
{
    if (_client != nullptr)
        _client->write(msg);
}

void Serwer::slot_new_client()
{
    _client = _server.nextPendingConnection();
    _client->setSocketOption(QAbstractSocket::LowDelayOption, 1);
    _bufor.clear();

    connect(_client, SIGNAL(disconnected()), this, SLOT(slot_client_disconnected()));
    connect(_client, SIGNAL(readyRead()), this, SLOT(slot_newMsg()));

    emit newClientConnected(_client->peerAddress().toString());
}

void Serwer::slot_client_disconnected()
{
    _client = nullptr;
    _bufor.clear();
    emit clientDisconnected();
}

void Serwer::slot_newMsg()
{
    _bufor.append(_client->readAll());

    while (true)
    {
        if (_bufor.size() < 19)
            return;

        QDataStream s(_bufor);
        quint16 znacznik;
        s >> znacznik;

        if (znacznik != 0xAAAA)
        {
            qWarning() << "Serwer: zły znacznik — czyszczę bufor!";
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
void Serwer::przetworzRamke(quint8 typ, quint32 id, QByteArray payload)
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
        qWarning() << "Serwer: nieznany typ wiadomości:" << typ;
    }
}

bool Serwer::czekajNaKlienta(int timeout)
{
    return _server.waitForNewConnection(timeout);
}

QTcpSocket *Serwer::pobierzPolaczenie()
{
    return _client;
}
