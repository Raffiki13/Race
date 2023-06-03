#ifndef CLIENT_H
#define CLIENT_H

#include <QWidget>
#include <QPainterPath>
#include <QDebug>
#include <QTcpSocket>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QDialog>
#include <QLineEdit>
#include <QComboBox>
#include <QLabel>
#include <QNetworkSession>
#include <QMainWindow>
#include <game.h>
#include <network.h>

class ServerAttributes;
class PlayerAttributes;
class Game;

class MainWindow: public QWidget {
    Q_OBJECT
    friend class ServerAttributes;
    friend class PlayerAttributes;
    friend class Game;

public:
    MainWindow();

private slots:
    void openPlayerAttributes();
    void openServerAttributes();
    void connectToServer();
    void serverDisconnect();
    void sendCoordsToServer();
    void sendToPainter();
    void sendMapData();
    void sendPicturesToGame();
protected:
    void closeEvent(QCloseEvent *event) override;

private:
    Network *network;
    ServerAttributes *attrWindow;
    PlayerAttributes *playerWindow;
    Game *gameWindow;
};

class ServerAttributes: public  QWidget {
    Q_OBJECT
    friend class MainWindow;

protected:
    void closeEvent(QCloseEvent *event) override;

public:
    explicit ServerAttributes(QWidget *parent = 0);

private:
    QString serverIp;
    QString serverPort;
    QLineEdit *serverLineEdit = nullptr;
    QLineEdit *portLineEdit = nullptr;
    QPushButton *confirmButton = nullptr;
    QString getServetIp() {return serverIp;}
    QString getPort() {return serverPort;}

private slots:
    void dataSaved();
};

class PlayerAttributes: public QWidget {
    Q_OBJECT
    friend class MainWindow;

protected:
    void closeEvent(QCloseEvent *event) override;

public:
    explicit PlayerAttributes(QWidget *parent = 0);

private:
    QString nickname;
    QLineEdit *nicknameEdit = nullptr;
    QPushButton *confirmButton = nullptr;
    QString getNickname() {return nickname;}

private slots:
    void saveNickname();
};

#endif // CLIENT_H
