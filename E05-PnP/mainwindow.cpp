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

    connect(hansRobot, &rb::HansClient::rb_RobotStateRefreshed, this, [this] () {
        rb::HansRobotState state = hansRobot->GetRobotState();
        qDebug() << state.MachineStateToQString();
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
    QString raw = "ReadCurFSM,Fail,20007,;";
    QStringList parsed;
    if(!(raw.right(2) == ",;")) {
        return;
    }
    raw = raw.left(raw.length() - 2);
    qDebug() << "Raw line: " << raw;
    parsed = raw.split(',');

    for(int i=0;i<parsed.size();i++) {
        qDebug() << "After parsed: " << parsed[i].toLower();
    }

//    int firstSpit = raw.indexOf(',');
//    QString cmd = raw.mid(0, firstSpit);
//    raw = raw.right(raw.length() - firstSpit - 1);
//    qDebug() << "2nd String: " << raw;
//    QString errorString = raw.mid(0, raw.indexOf(','));
//    qDebug() << "Command: " << cmd;
//    qDebug() << "Error report: " << errorString;

//    hansRobot->pushCommand(rb::HansCommand::SetOverride(0, 100));

//    for(int i=0;i<100;i++) {
//    hansRobot->pushCommand( rb::HansCommand::WayPoint(0,
//                                rb::DescartesPoint(0,0,100,180,0,0),
//                                rb::JointPoint(0,0,0,0,0,0),
//                                "TCP_dh_gripper",
//                                "Plane_1",
//                                500,
//                                2500,
//                                100,
//                                rb::MoveL,
//                                false, false, 0, true));

//    hansRobot->pushCommand( rb::HansCommand::WayPoint(0,
//                                 rb::DescartesPoint(0,200,100,180,0,0),
//                                 rb::JointPoint(0,0,0,0,0,0),
//                                 "TCP_dh_gripper",
//                                 "Plane_1",
//                                 500,
//                                 2500,
//                                 100,
//                                 rb::MoveL,
//                                 false, false, 0, true));

//    hansRobot->pushCommand( rb::HansCommand::WayPoint(0,
//                                 rb::DescartesPoint(200,200,100,180,0,0),
//                                 rb::JointPoint(0,0,0,0,0,0),
//                                 "TCP_dh_gripper",
//                                 "Plane_1",
//                                 500,
//                                 2500,
//                                 100,
//                                 rb::MoveL,
//                                 false, false, 0, true));

//    hansRobot->pushCommand( rb::HansCommand::WayPoint(0,
//                                 rb::DescartesPoint(200,0,100,180,0,0),
//                                 rb::JointPoint(0,0,0,0,0,0),
//                                 "TCP_dh_gripper",
//                                 "Plane_1",
//                                 500,
//                                 2500,
//                                 100,
//                                 rb::MoveL,
//                                 false, false, 0, true));
//    }
}
