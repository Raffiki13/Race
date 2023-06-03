#include "network.h"
#include <windows.h>
#include <game.h>
#include <iostream>
#include <QFile>
#include <string>
#include <sstream>
#include <QSslConfiguration>
#include <QSslKey>

Network::Network() {
    client_socket.setProtocol(QSsl::TlsV1SslV3);

    connect(&client_socket, SIGNAL(error(QAbstractSocket::SocketError)),
            this, SLOT(TCPError(QAbstractSocket::SocketError)));

    connect(&client_socket, SIGNAL(sslErrors(QList<QSslError>)), this, SLOT(sslError()));

    QByteArray cert;
    QFile CertFile("/etc/ssl/certs/ca-certificates.crt");
    if(CertFile.open(QIODevice::ReadOnly)) {
        cert = CertFile.readAll();
        CertFile.close();
    } else {
        qDebug() << CertFile.errorString();
    }

    QSslCertificate sslCert(cert);
    client_socket.setLocalCertificate(sslCert);

    QSslConfiguration cfg = client_socket.sslConfiguration();
    cfg.caCertificates();

    connect(&client_socket, SIGNAL(readyRead()), this, SLOT(tcpReady()));
}

qint32 type_cast(QByteArray buf) {
    qint32 tmp;
    QDataStream data(&buf, QIODevice::ReadWrite);
    data >> tmp;
    return tmp;
}

void Network::getAnswer() {
    if (client_socket.state() != QAbstractSocket::UnconnectedState) {
        QString hexStr = data.toHex();
        bool ok;

        QString id = QString(hexStr[2]) + QString(hexStr[3]) + QString(hexStr[0]) + QString(hexStr[1]);
        int command = QString::number(id.toInt(&ok, 16)).toInt();
        if (command == 301) {

        } else if (command == 210){
            int sumLen = 0;
            QString countPlayersStr = QString(hexStr[6]) + QString(hexStr[7]) + QString(hexStr[4]) + QString(hexStr[5]);
            int countPlayers = QString::number(countPlayersStr.toInt(&ok, 16)).toInt();
            for (int i = 0; i < countPlayers; i++){
                QString byte_len = QString(hexStr[10+4*i+sumLen*2+32*i]) + QString(hexStr[11+4*i+sumLen*2+32*i]) + QString(hexStr[8+4*i+sumLen*2+32*i]) + QString(hexStr[9+4*i+sumLen*2+32*i]);
                int len_nickname = QString::number(byte_len.toInt(&ok, 16)).toInt();
                QByteArray strHexNick;
                for (int j = 8+32*i+sumLen*2+4*(i+1); j < 8+4*(i+1)+(sumLen+len_nickname)*2+32*i; j++)
                    strHexNick.append(QChar(hexStr[j]));

                sumLen += len_nickname;
                QByteArray hexNick = QByteArray::fromHex(strHexNick);
                QString nick = QString(hexNick.data());

                users[i].first = nick;
                QString res = data.toHex();
                QString x;
                QString y;
                QString speed_x;
                QString speed_y;
                int tmp_end = 8+sumLen*2+4*(i+1)+32*i-1;
                int tmp_start = 8+sumLen*2+4*(i+1)+32*i+7;
                while (tmp_start > tmp_end){
                    x.append(res[tmp_start-1]);
                    x.append(res[tmp_start]);
                    tmp_start -= 2;
                }
                tmp_end = 8+sumLen*2+4*(i+1)+32*i+7;
                tmp_start = 8+sumLen*2+4*(i+1)+32*i+15;
                while (tmp_start > tmp_end){
                    y.append(res[tmp_start-1]);
                    y.append(res[tmp_start]);
                    tmp_start -= 2;
                }
                tmp_end = 8+sumLen*2+4*(i+1)+32*i+15;
                tmp_start = 8+sumLen*2+4*(i+1)+32*i+23;
                while (tmp_start > tmp_end){
                    speed_x.append(res[tmp_start-1]);
                    speed_x.append(res[tmp_start]);
                    tmp_start -= 2;
                }
                tmp_end = 8+sumLen*2+4*(i+1)+32*i+23;
                tmp_start = 8+sumLen*2+4*(i+1)+32*i+31;
                while (tmp_start > tmp_end){
                    speed_y.append(res[tmp_start-1]);
                    speed_y.append(res[tmp_start]);
                    tmp_start -= 2;
                }
                union ulf{
                    unsigned long ul;
                    float f;
                };

                ulf u;
                std::string x_std = x.toStdString();
                std::string y_std = y.toStdString();
                std::string speed_x_std = speed_x.toStdString();
                std::string speed_y_std = speed_y.toStdString();
                std::stringstream ss1(x_std);
                ss1 >> std::hex >> u.ul;
                float coordX = u.f;

                std::stringstream ss2(y_std);
                ss2 >> std::hex >> u.ul;
                float coordY = u.f;

                std::stringstream ss3(speed_x_std);
                ss3 >> std::hex >> u.ul;
                float speedX = u.f;
                std::stringstream ss4(speed_y_std);
                ss4 >> std::hex >> u.ul;
                float speedY = u.f;
                users[i].second = {coordX, coordY, speedX, speedY};

            }
            emit needToPaint();
        } else if (command == 220) {
            QString countPathsStr = QString(hexStr[6]) + QString(hexStr[7]) + QString(hexStr[4]) + QString(hexStr[5]);
            int countPaths = QString::number(countPathsStr.toInt(&ok, 16)).toInt();
            int sumLen = 0;
            for (int i = 0; i < countPaths; i++){
                QString numStr = QString(hexStr[10+sumLen]) + QString(hexStr[11+sumLen]) + QString(hexStr[8+sumLen]) + QString(hexStr[9+sumLen]);
                int num = QString::number(numStr.toInt(&ok, 16)).toInt();
                QString typeStr = QString(hexStr[14+sumLen]) + QString(hexStr[15+sumLen]) + QString(hexStr[12+sumLen]) + QString(hexStr[13+sumLen]);
                int type = QString::number(typeStr.toInt(&ok, 16)).toInt();
                if (type == 1){
                    QString res = data.toHex();
                    QString x;
                    QString y;
                    int tmp_end = 15+sumLen;
                    int tmp_start = 23+sumLen;
                    while (tmp_start > tmp_end){
                        x.append(res[tmp_start-1]);
                        x.append(res[tmp_start]);
                        tmp_start -= 2;
                    }
                    tmp_end = 23+sumLen;
                    tmp_start = 31+sumLen;
                    while (tmp_start > tmp_end){
                        y.append(res[tmp_start-1]);
                        y.append(res[tmp_start]);
                        tmp_start -= 2;
                    }
                    union ulf{
                        unsigned long ul;
                        float f;
                    };

                    ulf u;
                    std::string x_std = x.toStdString();
                    std::string y_std = y.toStdString();
                    std::stringstream ss1(x_std);
                    ss1 >> std::hex >> u.ul;
                    float coordX = u.f;

                    std::stringstream ss2(y_std);
                    ss2 >> std::hex >> u.ul;
                    float coordY = u.f;
                    sumLen += 24;
                    mapCoords.push_back({{num, type}, {coordX, coordY}});

                }
                if (type == 2){
                    QString res = data.toHex();
                    QString x;
                    QString y;
                    int tmp_end = 15+sumLen;
                    int tmp_start = 23+sumLen;
                    while (tmp_start > tmp_end){
                        x.append(res[tmp_start-1]);
                        x.append(res[tmp_start]);
                        tmp_start -= 2;
                    }
                    tmp_end = 23+sumLen;
                    tmp_start = 31+sumLen;
                    while (tmp_start > tmp_end){
                        y.append(res[tmp_start-1]);
                        y.append(res[tmp_start]);
                        tmp_start -= 2;
                    }
                    union ulf{
                        unsigned long ul;
                        float f;
                    };

                    ulf u;
                    std::string x_std = x.toStdString();
                    std::string y_std = y.toStdString();
                    std::stringstream ss1(x_std);
                    ss1 >> std::hex >> u.ul;
                    float coordX = u.f;

                    std::stringstream ss2(y_std);
                    ss2 >> std::hex >> u.ul;
                    float coordY = u.f;
                    sumLen += 24;
                    mapCoords.push_back({{num, type}, {coordX, coordY}});
                }
                if (type == 3){
                    QString res = data.toHex();
                    QString x;
                    QString y;
                    QString ang1;
                    QString ang2;
                    int tmp_end = 15+sumLen;
                    int tmp_start = 23+sumLen;
                    while (tmp_start > tmp_end){
                        x.append(res[tmp_start-1]);
                        x.append(res[tmp_start]);
                        tmp_start -= 2;
                    }
                    tmp_end = 23+sumLen;
                    tmp_start = 31+sumLen;
                    while (tmp_start > tmp_end){
                        y.append(res[tmp_start-1]);
                        y.append(res[tmp_start]);
                        tmp_start -= 2;
                    }

                    tmp_end = 31+sumLen;
                    tmp_start = 39+sumLen;
                    while (tmp_start > tmp_end){
                        ang1.append(res[tmp_start-1]);
                        ang1.append(res[tmp_start]);
                        tmp_start -= 2;
                    }
                    tmp_end = 39+sumLen;
                    tmp_start = 47+sumLen;
                    while (tmp_start > tmp_end){
                        ang2.append(res[tmp_start-1]);
                        ang2.append(res[tmp_start]);
                        tmp_start -= 2;
                    }

                    union ulf{
                        unsigned long ul;
                        float f;
                    };

                    ulf u;
                    std::string x_std = x.toStdString();
                    std::string y_std = y.toStdString();
                    std::string ang1_std = ang1.toStdString();
                    std::string ang2_std = ang2.toStdString();
                    std::stringstream ss1(x_std);
                    ss1 >> std::hex >> u.ul;
                    float coordX = u.f;

                    std::stringstream ss2(y_std);
                    ss2 >> std::hex >> u.ul;
                    float coordY = u.f;

                    std::stringstream ss3(ang1_std);
                    ss3 >> std::hex >> u.ul;
                    float angle1 = u.f;

                    std::stringstream ss4(ang2_std);
                    ss4 >> std::hex >> u.ul;
                    float angle2 = u.f;
                    sumLen += 40;
                    mapCoords.push_back({{num, type}, {coordX, coordY, angle1, angle2}});
                }
            }
            emit sendMapCoords();
            sendToServer(350);
        } else if(command == 351){
            sendToServer(333);
            int sumLen = 0;
            QString countPlayersStr = QString(hexStr[6]) + QString(hexStr[7]) + QString(hexStr[4]) + QString(hexStr[5]);
            int countPlayers = QString::number(countPlayersStr.toInt(&ok, 16)).toInt();
            for (int i = 0; i < countPlayers; i++){


                QString byte_len = QString(hexStr[10+sumLen]) + QString(hexStr[11+sumLen]) + QString(hexStr[8+sumLen]) + QString(hexStr[9+sumLen]);
                int len_nickname = QString::number(byte_len.toInt(&ok, 16)).toInt();
                QByteArray strHexNick;
                for (int j = 12+sumLen; j < 12+sumLen+(len_nickname)*2; j++)
                    strHexNick.append(QChar(hexStr[j]));

                QByteArray hexNick = QByteArray::fromHex(strHexNick);
                QString nick = QString(hexNick.data());
                QString pnglenStr = QString(hexStr[12+sumLen+(len_nickname)*2+2]) + QString(hexStr[12+sumLen+(len_nickname)*2+3]) + QString(hexStr[12+sumLen+(len_nickname)*2]) + QString(hexStr[12+sumLen+(len_nickname)*2+1]);
                int pnglen = QString::number(pnglenStr.toInt(&ok, 16)).toInt();

                QByteArray strHexPng;
                for (int j = 12+sumLen+(len_nickname)*2+4; j < 12+sumLen+(len_nickname+pnglen)*2+4; j++)
                    strHexPng.append(QChar(hexStr[j]));

                QByteArray hexPng = QByteArray::fromHex(strHexPng);
                QString png = QString(hexPng.data());
                sumLen += len_nickname*2+pnglen*2+8;
                pictures[nick] = png;
            }
            emit sendPictures();
        }
    }
}

void Network::tcpReady() {
    data.clear();
    QByteArray *buffer = new QByteArray;
    qint32 *s = new qint32(0);
    buffers.insert(&client_socket, buffer);
    sizes.insert(&client_socket, s);

    buffer = buffers.value(&client_socket);
    s = sizes.value(&client_socket);
    qint32 size = *s;

    while (client_socket.bytesAvailable() > 0){
        buffer->append(client_socket.readAll());
        while((size == 0 && buffer->size() >= 4) || (size > 0 && buffer->size() >= size)){
            if (size == 0 && buffer->size() >= 4){
                size = type_cast(buffer->mid(0, 4));
                *s = size;
                buffer->remove(0, 4);
            }
            if (size > 0 && buffer->size() >= size){
                data = buffer->mid(0, size);
                buffer->remove(0, size);
                size = 0;
                *s = size;
            }
        }
    }
    getAnswer();
}

void Network::connectTo(QString tmp_ip, int prt, QString nick) {
    client_socket.abort();
    nickname = nick;
    port = prt;
    ip = tmp_ip;
    client_socket.connectToHostEncrypted(ip, port);

    sendToServer(100);
}

QByteArray type_cast(qint32 number) {
    QByteArray tmp;
    QDataStream data(&tmp, QIODevice::ReadWrite);
    data << number;
    return tmp;
}

void Network::sendToServer(int command) {
    if (client_socket.state() != QAbstractSocket::UnconnectedState) {
        if (command == 100) {
            data.clear();
            data.append(reinterpret_cast<char*> (&command), 2);
            short len = nickname.size();
            data.append(reinterpret_cast<char*> (&len), 2);
            data.append(nickname.toStdString().c_str(), len);
            client_socket.write(type_cast(data.size()));
            client_socket.write(data);
            client_socket.waitForBytesWritten(1000);
        }
        else if (command == 150){
            data.clear();
            data.append(reinterpret_cast<char*> (&command), 2);
            short len = nickname.size();
            data.append(reinterpret_cast<char*> (&len), 2);
            data.append(nickname.toStdString().c_str(), len);
            client_socket.write(type_cast(data.size()));
            client_socket.write(data);
            client_socket.waitForBytesWritten(1000);
        } else if (command == 300){
            data.clear();
            data.append(reinterpret_cast<char*> (&command), 2);
            data.append(reinterpret_cast<char*> (&x), 4);
            data.append(reinterpret_cast<char*> (&y), 4);
            data.append(reinterpret_cast<char*> (&speed_x), 4);
            data.append(reinterpret_cast<char*> (&speed_y), 4);
            client_socket.write(type_cast(data.size()));
            client_socket.write(data);
            client_socket.waitForBytesWritten(1000);
        } else if (command == 200){
            data.clear();
            data.append(reinterpret_cast<char*> (&command), 2);
            client_socket.write(type_cast(data.size()));
            client_socket.write(data);
            client_socket.waitForBytesWritten(500);
        } else if (command == 350){
            data.clear();
            data.append(reinterpret_cast<char*> (&command), 2);
            client_socket.write(type_cast(data.size()));
            client_socket.write(data);
            client_socket.waitForBytesWritten(3000);
        } else if (command == 333){
            data.clear();
            data.append(reinterpret_cast<char*> (&command), 2);
            client_socket.write(type_cast(data.size()));
            client_socket.write(data);
            client_socket.waitForBytesWritten(1000);
        }
    }
}

void Network::disconnect() {
    sendToServer(150);
    client_socket.disconnectFromHost();
}

void Network::sslError() {
    this->err = client_socket.sslErrors();
    for(int i = 0; i < err.size(); i++)
        qDebug() << err[i];
}

void Network::TCPError(QAbstractSocket::SocketError) {
    qDebug("Error: %s", qPrintable(client_socket.errorString()));
}

void Network::move(float x, float y, float speed_x, float speed_y) {
    this->x = x;
    this->y = y;
    this->speed_x = speed_x;
    this->speed_y = speed_y;

    sendToServer(300);
}
