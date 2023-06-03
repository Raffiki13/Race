#include "windows.h"
#include <iostream>
#include <QPainter>
#include <QKeyEvent>
#include <QTimer>
#include <QtMath>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMenu>
#include <QMenuBar>
#include <QPushButton>
#include <QHostInfo>
#include <QNetworkConfigurationManager>
#include <QDialogButtonBox>
#include <QSettings>
#include <QApplication>
#include <QStyleHints>
#include <QGroupBox>
#include <QRect>
#include <QDesktopWidget>
#include <network.h>


MainWindow::MainWindow(): QWidget() {
    attrWindow = new ServerAttributes;
    playerWindow = new PlayerAttributes;
    network = new Network;
    connect(network, SIGNAL(needToPaint()), this, SLOT(sendToPainter()));
    connect(network, SIGNAL(sendMapCoords()),this, SLOT(sendMapData()));
    connect(network, SIGNAL(sendPictures()), this, SLOT(sendPicturesToGame()));
    QVBoxLayout *box = new QVBoxLayout;
    QMenuBar *bar = new QMenuBar();
    QMenu *menu1 = new QMenu("&Game");
    menu1->addAction(tr("&Connect"), this, SLOT(connectToServer()));
    menu1->addAction(tr("&Disonnect"), this, SLOT(serverDisconnect()));
    QMenu *menu2 = new QMenu("&Settings");
    QMenu *menu2_1 = new QMenu("&Network");
    QMenu *menu2_2 = new QMenu("&Player");
    bar->addMenu(menu1);
    bar->addMenu(menu2);
    menu2->addMenu(menu2_1);
    menu2->addMenu(menu2_2);
    menu2_1->addAction(tr("&Server"), this, SLOT(openServerAttributes()));
    menu2_2->addAction(tr("&Profile"), this, SLOT(openPlayerAttributes()));
    menu1->addAction(tr("&Exit"), this, SLOT(serverDisconnect()));
    this->setLayout(box);
    this->layout()->setMenuBar(bar);
    this->setFixedHeight(300);
    this->setFixedWidth(500);
}

void MainWindow::closeEvent(QCloseEvent *event) {
    serverDisconnect();
}

void MainWindow::sendToPainter() {
    gameWindow->setUsers(network->getUsers());
    gameWindow->updateField();
}

void MainWindow::sendPicturesToGame() {
    gameWindow->pictures = network->pictures;
}

void MainWindow::sendMapData() {
    gameWindow->mapCoords = network->mapCoords;
}

void MainWindow::serverDisconnect() {
    network->disconnect();
    gameWindow->close();
}

void MainWindow::connectToServer() {
    gameWindow = new Game;
    network->connectTo(attrWindow->getServetIp(), attrWindow->getPort().toInt(), playerWindow->getNickname());
    connect(gameWindow, &Game::signal, this, &MainWindow::sendCoordsToServer);
    connect(gameWindow, &Game::disc, this, &MainWindow::serverDisconnect);
    gameWindow->setNickname(playerWindow->getNickname());
    gameWindow->setCar();
    gameWindow->show();
}

void MainWindow::sendCoordsToServer() {
    if (gameWindow->moveUp && gameWindow->moveLeft)
        network->move(gameWindow->getCoordX(), gameWindow->getCoordY(), gameWindow->getSpeedX(), -gameWindow->getSpeedY());
    else if (gameWindow->moveUp && gameWindow->moveRight)
        network->move(gameWindow->getCoordX(), gameWindow->getCoordY(), gameWindow->getSpeedX(), -gameWindow->getSpeedY());
    else if (gameWindow->moveDown && gameWindow->moveLeft)
        network->move(gameWindow->getCoordX(), gameWindow->getCoordY(), -gameWindow->getSpeedX(), gameWindow->getSpeedY());
    else if (gameWindow->moveDown && gameWindow->moveRight)
        network->move(gameWindow->getCoordX(), gameWindow->getCoordY(), -gameWindow->getSpeedX(), gameWindow->getSpeedY());
    else if (gameWindow->moveUp)
        network->move(gameWindow->getCoordX(), gameWindow->getCoordY(), gameWindow->getSpeedX(), -gameWindow->getSpeedY());
    else if (gameWindow->moveDown)
        network->move(gameWindow->getCoordX(), gameWindow->getCoordY(), -gameWindow->getSpeedX(), gameWindow->getSpeedY());
}

ServerAttributes::ServerAttributes(QWidget *parent): QWidget (parent) {
    this->setWindowTitle("Server Attributes");

    serverLineEdit = new QLineEdit("127.0.0.1");
    portLineEdit = new QLineEdit("8080");
    confirmButton = new QPushButton("Save Data");
    portLineEdit->setValidator(new QIntValidator(1, 65535, this));
    auto hostLabel = new QLabel(tr("&Server name:"));
    hostLabel->setBuddy(serverLineEdit);
    auto portLabel = new QLabel(tr("&Server port:"));
    portLabel->setBuddy(portLineEdit);
    auto buttonBox = new QDialogButtonBox;
    buttonBox->addButton(confirmButton,QDialogButtonBox::ActionRole);
    connect(confirmButton, &QAbstractButton::clicked, this, &QWidget::close);

    confirmButton->setDefault(true);
    QGridLayout *mainLayout = new QGridLayout;

    mainLayout->addWidget(hostLabel, 0, 0);
    mainLayout->addWidget(portLabel, 1, 0);
    mainLayout->addWidget(serverLineEdit, 0, 1);
    mainLayout->addWidget(portLineEdit, 1, 1);
    mainLayout->addWidget(buttonBox);

    this->setLayout(mainLayout);
}

void ServerAttributes::closeEvent(QCloseEvent *event) {
    dataSaved();
}

void ServerAttributes::dataSaved() {
    serverIp = serverLineEdit->text();
    serverPort = portLineEdit->text();
}

PlayerAttributes::PlayerAttributes(QWidget *parent): QWidget (parent) {
    this->setWindowTitle("Player Attributes");
    nicknameEdit= new QLineEdit("Client");
    confirmButton = new QPushButton("Save Data");

    auto nickLabel = new QLabel(tr("&Nickname:"));
    nickLabel->setBuddy(nicknameEdit);
    auto buttonBox = new QDialogButtonBox;
    buttonBox->addButton(confirmButton,QDialogButtonBox::ActionRole);
    connect(confirmButton, &QAbstractButton::clicked, this, &QWidget::close);

    confirmButton->setDefault(true);
    QGridLayout *mainLayout = new QGridLayout;
    mainLayout->addWidget(nickLabel, 0, 0);
    mainLayout->addWidget(nicknameEdit, 0, 1);
    mainLayout->addWidget(buttonBox);
    this->setLayout(mainLayout);
}

void PlayerAttributes::closeEvent(QCloseEvent *event) {
    saveNickname();
}

void PlayerAttributes::saveNickname() {
    nickname = nicknameEdit->text();
}

void MainWindow::openServerAttributes() {
   attrWindow->show();
}

void MainWindow::openPlayerAttributes() {
    playerWindow->show();
}
