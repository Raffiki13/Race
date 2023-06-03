#include "server.h"
#include <QDataStream>
#include <QWidget>
#include <QTableWidget>
#include <QFile>
#include <QSslKey>
#include <QSslConfiguration>
#include <QMessageBox>
#include <QtNetwork>
#include <QAbstractSocket>
#include <string>
#include <sstream>

Server::Server(QObject *parent): QTcpServer(parent) {
    xml();
    win = new QWidget();
    tableWidget = new QTableWidget(win);
    tableWidget->setColumnCount(2);
    tableWidget->setHorizontalHeaderLabels(QString("Client_Status").split("_"));
    tableWidget->setFixedHeight(500);
    tableWidget->setFixedWidth(900);

    server = new QTcpServer;

    if (!server->listen(QHostAddress::Any, 2323)) {
        qDebug() << "Server could not start";
    } else {
        qDebug() << "Server started!";
    }

    connect(server, SIGNAL(newConnection()), this, SLOT(newConnectionRecognized()));

    QTimer* timer = new QTimer;
    connect(timer, &QTimer::timeout, this, &Server::sendDataAboutPlayers);
    timer->start(50);

    window();
}

void Server::showErrors() {
    this->err = socket->sslErrors();
    for(int i = 0; i < err.size(); i++)
        qDebug() << err[i];
}

void Server::newConnectionRecognized() {
    incomingConnection(server->nextPendingConnection()->socketDescriptor());
}

QByteArray type_cast2(qint32 number) {
    QByteArray tmp;
    QDataStream data(&tmp, QIODevice::ReadWrite);
    data << number;
    return tmp;
}

qint32 type_cast(QByteArray buf){
    qint32 tmp;
    QDataStream data(&buf, QIODevice::ReadWrite);
    data >> tmp;
    return tmp;
}

void Server::sendDataAboutPlayers(){
    int command = 210;
    QByteArray dataPlayers;
    dataPlayers.clear();

    int countPlayers = sockets.size();
    dataPlayers.append(reinterpret_cast<char*> (&command), 2);
    dataPlayers.append(reinterpret_cast<char*> (&countPlayers), 2);
    collision();
    for (int i = 0; i < sockets.size(); i++){
        short len = users[sockets[i]].first.size();
        dataPlayers.append(reinterpret_cast<char*> (&len), 2);
        dataPlayers.append(users[sockets[i]].first.toStdString().c_str(), len);
        users[sockets[i]].second[0] += users[sockets[i]].second[2];
        users[sockets[i]].second[1] += users[sockets[i]].second[3];

        float x = users[sockets[i]].second[0];
        float y = users[sockets[i]].second[1];
        float vX = users[sockets[i]].second[2];
        float vY = users[sockets[i]].second[3];

        dataPlayers.append(reinterpret_cast<char*> (&x), 4);
        dataPlayers.append(reinterpret_cast<char*> (&y), 4);
        dataPlayers.append(reinterpret_cast<char*> (&vX), 4);
        dataPlayers.append(reinterpret_cast<char*> (&vY), 4);

        users[sockets[i]].second[0] += users[sockets[i]].second[2];
        users[sockets[i]].second[1] += users[sockets[i]].second[3];

    }
    for (int i = 0; i < sockets.size(); i++){
        sockets[i]->write(type_cast2(dataPlayers.size()));
        sockets[i]->write(dataPlayers);
    }
}

void Server::changeJournal() {
    tableWidget->setRowCount(sockets.size());
    for (int i = 0; i < sockets.size(); i++){
        tableWidget->setItem(i, 0, new QTableWidgetItem(users[sockets[i]].first));
        tableWidget->setItem(i, 1, new QTableWidgetItem("connected"));
    }
}

void Server::incomingConnection(qintptr socketDescriptor) {
    socket = new QSslSocket(this);
    sockets.push_back(socket);
    connections.push_back(false);
    QByteArray *buffer = new QByteArray;
    qint32 *s = new qint32(0);
    buffers.insert(socket, buffer);
    sizes.insert(socket, s);
    
    socket->setProtocol(QSsl::TlsV1SslV3);

    connect(socket, SIGNAL(sslErrors(QList<QSslError>)), this, SLOT(showErrors()));
    connect(socket, SIGNAL(encrypted()), this, SLOT(ready()));
    connect(socket, SIGNAL(readyRead()), this, SLOT(read()));

    QByteArray key;
    QFile KeyFile("/etc/ssl/private/ssl-cert-snakeoil.key");
    if(KeyFile.open(QIODevice::ReadOnly)) {
        key = KeyFile.readAll();
        KeyFile.close();
    } else {
        qDebug() << KeyFile.errorString();
    }

    QSslKey sslKey(key, QSsl::Rsa, QSsl::Pem);
    socket->setPrivateKey(sslKey);

    QByteArray cert;
    QFile CertFile("/etc/ssl/certs/nginx-selfsigned.crt");
    if(CertFile.open(QIODevice::ReadOnly)) {
        cert = CertFile.readAll();
        CertFile.close();
    } else {
        qDebug() << CertFile.errorString();
    }

    QSslCertificate sslCert(cert);
    socket->setLocalCertificate(sslCert);

    QSslConfiguration cfg = socket->sslConfiguration();
    cfg.caCertificates();

    if (!socket->setSocketDescriptor(socketDescriptor)) {
        qDebug() << ("! Couldn't set socket descriptor");
        delete socket;
        return;
    }

    socket->startServerEncryption();

    if (socket->isEncrypted()) {
        emit socket->encrypted();
    } else {
        qDebug() << "No chance";
    }

    if(!socket->waitForEncrypted(3000)) {
        qDebug("Wait for encrypted!!!!");
        return;
    }

    while (true) {
        socket->waitForReadyRead();
    }
}

void Server::ready() {
    qDebug() << "Encrypted";
}

void Server::read() {
    data.clear();
    int user_id = -1;
    QSslSocket *tmp_socket = static_cast<QSslSocket*>(sender());
    for (int i = 0; i < sockets.size(); i++) {
         if (sockets[i] == tmp_socket)
             user_id = i;
    }
    QByteArray *buffer = buffers.value(sockets[user_id]);
    qint32 *s = sizes.value(sockets[user_id]);
    qint32 size = *s;
    while (sockets[user_id]->bytesAvailable() > 0){
        buffer->append(sockets[user_id]->readAll());
        while((size == 0 && buffer->size() >= 4) || (size > 0 && buffer->size() >= size)) {
            if (size == 0 && buffer->size() >= 4) {
                size = type_cast(buffer->mid(0, 4));
                *s = size;
                buffer->remove(0, 4);
            }
            if (size > 0 && buffer->size() >= size) {
                data = buffer->mid(0, size);
                buffer->remove(0, size);
                size = 0;
                *s = size;
            }
        }
    }
    getRequest(user_id);
}

void Server::getRequest(int user_id) {
    QString hexStr = data.toHex();
    bool ok;

    QString id = QString(hexStr[2]) + QString(hexStr[3]) + QString(hexStr[0]) + QString(hexStr[1]);
    int command = QString::number(id.toInt(&ok, 16)).toInt();


    if (command == 100){
        QString byte_len = QString(hexStr[6]) + QString(hexStr[7]) + QString(hexStr[4]) + QString(hexStr[5]);
        int len_nickname = QString::number(byte_len.toInt(&ok, 16)).toInt();

        QByteArray dataNick;
        for (int i = 4; i < len_nickname*4; i++)
            dataNick.append(QChar(data[i]));

        QString nickname = QString(dataNick);
        users[sockets[user_id]] = {nickname, {(float)(45.0*sockets.size()), 30.0, 0.0, 0.0}};
        changeJournal();
    } else if (command == 150) {
        QString byte_len = QString(hexStr[6]) + QString(hexStr[7]) + QString(hexStr[4]) + QString(hexStr[5]);
        int len_nickname = QString::number(byte_len.toInt(&ok, 16)).toInt();
        QByteArray dataNick;
        for (int i = 4; i < len_nickname*4; i++){
            dataNick.append(QChar(data[i]));
        }
        QString nickname = QString(dataNick);

        sockets.erase(sockets.begin() + user_id);
        buffers.erase(buffers.begin() + user_id);
        sizes.erase(sizes.begin() + user_id);
        users.remove(sockets[user_id]);

        changeJournal();
    } else if (command == 300) {
        QString res = data.toHex();
        QString x;
        QString y;
        QString speed_x;
        QString speed_y;

        int tmp = 11;
        while (tmp > 3) {
            x.append(res[tmp-1]);
            x.append(res[tmp]);
            tmp -= 2;
        }
        tmp = 19;
        while (tmp > 11) {
            y.append(res[tmp-1]);
            y.append(res[tmp]);
            tmp -= 2;
        }
        tmp = 27;
        while (tmp > 19) {
            speed_x.append(res[tmp-1]);
            speed_x.append(res[tmp]);
            tmp -= 2;
        }
        tmp = 35;
        while (tmp > 27) {
            speed_y.append(res[tmp-1]);
            speed_y.append(res[tmp]);
            tmp -= 2;
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
        users[sockets[user_id]].second = {coordX, coordY, speedX, speedY};

        SendToClient(user_id, 301);
    } else if (command == 200){
        qDebug () << 200;
        SendToClient(user_id, 220);
    }
    else if (command == 350){
        QByteArray dataPlayers;
        dataPlayers.clear();
        int cmd = 351;
        dataPlayers.append(reinterpret_cast<char*>(&cmd), 2);
        int countPlayers = sockets.size();
        dataPlayers.append(reinterpret_cast<char*> (&countPlayers), 2);
        for (int i = 0; i < sockets.size(); i++){
            QString pngName = "car"+QString::number(i%5)+".png";
            QString nickname = users[sockets[i]].first;
            short lenFileName = pngName.size();
            short nicknameLen = nickname.size();
            dataPlayers.append(reinterpret_cast<char*>(&nicknameLen), 2);
            dataPlayers.append(nickname.toStdString().c_str(),nicknameLen);
            dataPlayers.append(reinterpret_cast<char*>(&lenFileName), 2);
            dataPlayers.append(pngName.toStdString().c_str(),lenFileName);
        }
        qDebug () << dataPlayers;
        qDebug () << user_id << sockets.size();
        for (int i = 0; i < sockets.size(); i++){
            sockets[i]->write(type_cast2(dataPlayers.size()));
            sockets[i]->write(dataPlayers);
            sockets[i]->waitForBytesWritten(30000);
        }
    }
    qDebug () << connections;
}

void Server::SendToClient(int user_id, int command) {
    data.clear();
    if (command == 101){
        data.clear();
        data.append(reinterpret_cast<char*> (&command), 2);
        sockets[user_id]->write(type_cast2(data.size()));
        sockets[user_id]->write(data);
        sockets[user_id]->waitForBytesWritten(1000);
    } else if (command == 151){
        data.clear();
        data.append(reinterpret_cast<char*> (&command), 2);
        sockets[user_id]->write(type_cast2(data.size()));
        sockets[user_id]->write(data);
        sockets[user_id]->waitForBytesWritten(1000);
    } else if (command == 301){
           data.clear();
           data.append(reinterpret_cast<char*> (&command), 2);
           sockets[user_id]->write(type_cast2(data.size()));
           sockets[user_id]->write(data);
           sockets[user_id]->waitForBytesWritten(1000);
    } else if (command == 220){
        qDebug () << 220;
        sockets[user_id]->write(type_cast2(map.size()));
        sockets[user_id]->write(map);
        sockets[user_id]->waitForBytesWritten(3000);
    }
}

void Server::window() {
    win->setFixedHeight(900);
    win->setFixedWidth(1200);
    win->setWindowTitle("127.0.0.1 2323 0"); //ip, port, num
}

void Server::collision(){
    for(int i = 0; i < sockets.size(); i++) {
        for(int j = 0; j < sockets.size(); j++) {
            int dx = 0, dy = 0;
            while (dx*dx + dy*dy < 4* 22*22) {
               users[sockets[i]].second[0] += dx/10.0;
               users[sockets[i]].second[1] += dy/10.0;
               users[sockets[j]].second[0] -= dx/10.0;
               users[sockets[j]].second[1] -= dy/10.0;
               dx = users[sockets[i]].second[0] - users[sockets[j]].second[0];
               dy = users[sockets[i]].second[1] - users[sockets[j]].second[1];
               if (!dx && !dy) break;
            }
        }
    }
}

void Server::xml() {
    int command = 220;
    map.append(reinterpret_cast<char*> (&command), 2);
    QString fileName = "/home/mikaraf/LAB/1race/laba/server/map/map.xml";
    QFile file(fileName);
    if (!file.open(QFile::ReadOnly | QFile::Text)){
        qDebug () << "Not opened xml";
    } else {
        QXmlStreamReader xmlReader;
        xmlReader.setDevice(&file);
        xmlReader.readNext();
        while (!xmlReader.atEnd()){
            if (xmlReader.isStartElement()){
                if (xmlReader.name() == "Length"){
                    foreach(const QXmlStreamAttribute &attr, xmlReader.attributes()){
                        if (attr.name().toString() == "n") {
                            int num = attr.value().toString().toInt();
                            qDebug () << num;
                            map.append(reinterpret_cast<char*> (&num), 2);
                        }
                    }
                }
                if (xmlReader.name() == "Path"){
                    foreach(const QXmlStreamAttribute &attr, xmlReader.attributes()) {
                        if (attr.name().toString() == "num") {
                            int num = attr.value().toString().toInt();
                            map.append(reinterpret_cast<char*> (&num), 2);
                        }
                        if (attr.name().toString() == "type") {
                            int type = attr.value().toString().toInt();
                            map.append(reinterpret_cast<char*> (&type), 2);

                        }
                        if (attr.name().toString() == "x"){
                            float x = attr.value().toString().toFloat();
                            map.append(reinterpret_cast<char*> (&x), 4);

                        }
                        if (attr.name().toString() == "y"){
                            float y = attr.value().toString().toFloat();
                            map.append(reinterpret_cast<char*> (&y), 4);
                        }
                        if (attr.name().toString() == "d1"){
                            float d1 = attr.value().toString().toFloat();
                            map.append(reinterpret_cast<char*> (&d1), 4);

                        }
                        if (attr.name().toString() == "d2"){
                            float d2 = attr.value().toString().toFloat();
                            map.append(reinterpret_cast<char*> (&d2), 4);
                        }
                    }
                }
            }
            xmlReader.readNext();
        }
    }
    file.close();
}
