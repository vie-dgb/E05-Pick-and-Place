#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    // initialize ui language
    ui_Language_Init();

    // connect event button
    connect(ui->btn_robot_Connect, &QPushButton::clicked, this, &MainWindow::on_click_robot_connect);
    connect(ui->btn_robot_Control, &QPushButton::clicked, this, &MainWindow::on_click_robot_control);

    // init robot
    robot_UiInitialize();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::closeEvent(QCloseEvent *event) {
    // check user really want to close application or not
    // show message box with "Yes" and "Cancel" button, focus on Cancel button
    QMessageBox::StandardButton resBtn = QMessageBox::question( this, this->windowTitle(),
                                                               tr("Are you sure?\n"),
                                                               QMessageBox::Yes | QMessageBox::No,
                                                               QMessageBox::No);
    if (resBtn != QMessageBox::Yes) {
        event->ignore();
        return;
    }

    // accept event (accept close application)
    event->accept();
}

void MainWindow::ui_Language_Init() {
    // normal UI language is English
    currentLanguage = LanguagesUI::EN;

    connect(ui->actionEnglish, &QAction::triggered, this, [this]() {
        ui_Language_Load(LanguagesUI::EN);
    });
    connect(ui->actionJapanese, &QAction::triggered, this, [this]() {
        ui_Language_Load(LanguagesUI::JP);
    });
    connect(ui->actionVietnamese, &QAction::triggered, this, [this]() {
        ui_Language_Load(LanguagesUI::VN);
    });

    this->setWindowTitle("Han's E05 pick and place control application");
}

void MainWindow::ui_Language_Load(const LanguagesUI lang) {
    if(currentLanguage == lang) {
        return;
    }

    QString translatorPath = QApplication::applicationDirPath();
    bool loadState = false;

    switch (lang) {
    case LanguagesUI::VN:
        translatorPath.append("/E05-PnP_vi_VN.qm");
        loadState = uiTranslator.load(translatorPath);
        qApp->installTranslator(&uiTranslator);
        break;
    case LanguagesUI::JP:
        translatorPath.append("/E05-PnP_ja_JP.qm");
        loadState = uiTranslator.load(translatorPath);
        qApp->installTranslator(&uiTranslator);
        break;
    case LanguagesUI::EN:
        loadState = qApp->removeTranslator(&uiTranslator);
        break;
    }

    if(loadState) {
        ui->retranslateUi(this);
        currentLanguage = lang;
    }
}

/// ROBOT UI FUNTIONS
void MainWindow::robot_UiInitialize() {
    hansRobot = new rb::HansClient(QThread::NormalPriority);
    robotAddress = "192.168.1.177";

    // robot connected
    connect(hansRobot, &rb::HansClient::rb_Connected, this, [this] () {
        ui->btn_robot_Connect->setText(lb_robot_Disconnect);
        ui->label_robot_Connect->setText(lb_robot_Connected);
        ui->label_robot_IP->setText(lb_robot_dialog_Address + ": " + robotAddress);
    });
    // robot disconnected
    connect(hansRobot, &rb::HansClient::rb_Disconnected, this, [this] () {
        ui->btn_robot_Connect->setText(lb_robot_Connect);
        ui->label_robot_Connect->setText(lb_robot_Disconnected);
    });
    // robot connect fail
    connect(hansRobot, &rb::HansClient::rb_ConnectFail, this, [this] () {
        ui->label_robot_Connect->setText(lb_robot_Disconnected);
        QMessageBox::information(this,lb_robot_dialog_ConnectInfo, lb_robot_dialog_ConnectFail, QMessageBox::Ok);
    });

    ui->btn_robot_Connect->setText(lb_robot_Connect);
    ui->label_robot_Connect->setText(lb_robot_Disconnected);
}

bool MainWindow::robot_UserInputAddress() {
    // setup input dialog form
    InputFormDialog::FormData data;
    data[lb_robot_dialog_Address] = robotAddress;

    while(true) {
        if(InputFormDialog::getInput("Enter Han's robot address", data)) {
            QHostAddress h_address(data.at<QString>(lb_robot_dialog_Address));
            if(h_address.protocol() == QAbstractSocket::IPv4Protocol) {
                robotAddress = data.at<QString>(lb_robot_dialog_Address);
                return true;
            }
        }
        else {
            return false;
        }
    }
}

//////////  BUTTON ACTION
void MainWindow::on_click_robot_connect() {
    if(hansRobot->robotIsConnected()) {
        hansRobot->robotDisconnect();
    }
    else {
        if(robot_UserInputAddress()) {
            hansRobot->robotConnect(robotAddress);
            ui->label_robot_Connect->setText("Wait connect");
        }
    }
}

void MainWindow::on_click_robot_control() {
    hansRobot->pushCommand(rb::HansCommand::SetOverride(0, 2));
    hansRobot->pushCommand(rb::HansCommand::SetUCSByName(0, "Plane_1"));
    hansRobot->pushCommand(rb::HansCommand::MoveL(0,
                rb::DescartesPoint(0,0,100,180,0,0)));
    hansRobot->pushCommand(rb::HansCommand::MoveL(0,
                rb::DescartesPoint(100,100,50,180,0,0)));
//    hansRobot->pushCommand(rb::HansCommand::MoveL(0,
//                rb::DescartesPoint(-396.255,34,200,180,0,0)));
}
