#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    initUiEvent();
    connectUiEvent();
    ui_Language_Init();
    robot_UiInitialize();
}

MainWindow::~MainWindow()
{
    delete updateInfoTimer;
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

void MainWindow::initUiEvent() {
    updateInfoTimer = new QTimer;
    updateInfoTimer->setInterval(updateCyclicTime);
    updateInfoTimer->start();
}

void MainWindow::connectUiEvent() {
    /// TIMER EVENTS
    connect(updateInfoTimer, &QTimer::timeout, this, &MainWindow::on_Timeout_UpdateUiInfo);

    /// COMBOBOX EVENTS
    connect(ui->comboBox_Language, &QComboBox::currentIndexChanged, this, &MainWindow::ui_Language_Load);

    /// BUTTON EVENTS
    connect(ui->btn_robot_Connect, &QPushButton::clicked, this, &MainWindow::on_Click_Robot_Connect);
}

void MainWindow::ui_Language_Init() {
    // normal UI language is English
    currentLanguage = LanguagesUI::EN;
    this->setWindowTitle("Han's E05 pick and place control application");
}

void MainWindow::ui_Language_Load(const int index) {
    LanguagesUI lang = static_cast<LanguagesUI>(index);
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
    // robot lost connect
    connect(hansRobot, &rb::HansClient::rb_FeedbackPortReadError, this, [this] () {
        ui->label_robot_Connect->setText(lb_robot_Disconnected);
        QMessageBox::information(this,lb_robot_dialog_ConnectInfo, lb_robot_dialog_LostConnect, QMessageBox::Ok);
    });

    // robot change box digiatal output
    connect(ui->radioButton_DO_0, &QRadioButton::clicked, this, [this] (bool state) {
        hansRobot->pushCommand(rb::HansCommand::SetBoxDO(0, state));
    });
    connect(ui->radioButton_DO_1, &QRadioButton::clicked, this, [this] (bool state) {
        hansRobot->pushCommand(rb::HansCommand::SetBoxDO(1, state));
    });
    connect(ui->radioButton_DO_2, &QRadioButton::clicked, this, [this] (bool state) {
        hansRobot->pushCommand(rb::HansCommand::SetBoxDO(2, state));
    });
    connect(ui->radioButton_DO_3, &QRadioButton::clicked, this, [this] (bool state) {
        hansRobot->pushCommand(rb::HansCommand::SetBoxDO(3, state));
    });
    connect(ui->radioButton_DO_4, &QRadioButton::clicked, this, [this] (bool state) {
        hansRobot->pushCommand(rb::HansCommand::SetBoxDO(4, state));
    });
    connect(ui->radioButton_DO_5, &QRadioButton::clicked, this, [this] (bool state) {
        hansRobot->pushCommand(rb::HansCommand::SetBoxDO(5, state));
    });
    connect(ui->radioButton_DO_6, &QRadioButton::clicked, this, [this] (bool state) {
        hansRobot->pushCommand(rb::HansCommand::SetBoxDO(6, state));
    });
    connect(ui->radioButton_DO_7, &QRadioButton::clicked, this, [this] (bool state) {
        hansRobot->pushCommand(rb::HansCommand::SetBoxDO(7, state));
    });

    // robot gripper toggle
    hansRobot->DHGripper_Setup(0, 1, 0, 1);
    connect(ui->btn_robot_gripperToggle, &QPushButton::clicked, this, [this] {
        if(hansRobot->DHGripper_IsOpen()) {
            hansRobot->DHGripper_Close();
        } else {
            hansRobot->DHGripper_Open();
        }
    });

//    connect(hansRobot, &rb::HansClient::rb_RobotStateRefreshed, this, [this] () {
//        rb::HansRobotState state = hansRobot->GetRobotState();
//    });

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

void MainWindow::robot_UiUpdate() {
    if(ui->tab_RobotHans->isHidden()) {
        return;
    }

    rb::HansRobotState robotState = hansRobot->GetRobotState();
    ui->label_robot_MachineState->setText(lb_robot_StateMachine + ": " + robotState.MachineStateToQString());
    /// refresh Box digital input state
    ui->radioButton_DI_0->setChecked(hansRobot->GetRobotBoxDI(0));
    ui->radioButton_DI_1->setChecked(hansRobot->GetRobotBoxDI(1));
    ui->radioButton_DI_2->setChecked(hansRobot->GetRobotBoxDI(2));
    ui->radioButton_DI_3->setChecked(hansRobot->GetRobotBoxDI(3));
    ui->radioButton_DI_4->setChecked(hansRobot->GetRobotBoxDI(4));
    ui->radioButton_DI_5->setChecked(hansRobot->GetRobotBoxDI(5));
    ui->radioButton_DI_6->setChecked(hansRobot->GetRobotBoxDI(6));
    ui->radioButton_DI_7->setChecked(hansRobot->GetRobotBoxDI(7));
    /// refresh Box digital output state
    ui->radioButton_DO_0->setChecked(hansRobot->GetRobotBoxDO(0));
    ui->radioButton_DO_1->setChecked(hansRobot->GetRobotBoxDO(1));
    ui->radioButton_DO_2->setChecked(hansRobot->GetRobotBoxDO(2));
    ui->radioButton_DO_3->setChecked(hansRobot->GetRobotBoxDO(3));
    ui->radioButton_DO_4->setChecked(hansRobot->GetRobotBoxDO(4));
    ui->radioButton_DO_5->setChecked(hansRobot->GetRobotBoxDO(5));
    ui->radioButton_DO_6->setChecked(hansRobot->GetRobotBoxDO(6));
    ui->radioButton_DO_7->setChecked(hansRobot->GetRobotBoxDO(7));
}

//////////  TIMER ACTIONS
void MainWindow::on_Timeout_UpdateUiInfo() {
    if(hansRobot->robotIsConnected()) {
        // update robot info in UI
        robot_UiUpdate();
    }
}

//////////  BUTTON ACTIONS
void MainWindow::on_Click_Robot_Connect() {
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

//void MainWindow::on_click_robot_control() {
////    int firstSpit = raw.indexOf(',');
////    QString cmd = raw.mid(0, firstSpit);
////    raw = raw.right(raw.length() - firstSpit - 1);
////    qDebug() << "2nd String: " << raw;
////    QString errorString = raw.mid(0, raw.indexOf(','));
////    qDebug() << "Command: " << cmd;
////    qDebug() << "Error report: " << errorString;

////    hansRobot->pushCommand(rb::HansCommand::SetOverride(0, 100));

////    for(int i=0;i<100;i++) {
////    hansRobot->pushCommand( rb::HansCommand::WayPoint(0,
////                                rb::DescartesPoint(0,0,100,180,0,0),
////                                rb::JointPoint(0,0,0,0,0,0),
////                                "TCP_dh_gripper",
////                                "Plane_1",
////                                500,
////                                2500,
////                                100,
////                                rb::MoveL,
////                                false, false, 0, true));

////    hansRobot->pushCommand( rb::HansCommand::WayPoint(0,
////                                 rb::DescartesPoint(0,200,100,180,0,0),
////                                 rb::JointPoint(0,0,0,0,0,0),
////                                 "TCP_dh_gripper",
////                                 "Plane_1",
////                                 500,
////                                 2500,
////                                 100,
////                                 rb::MoveL,
////                                 false, false, 0, true));

////    hansRobot->pushCommand( rb::HansCommand::WayPoint(0,
////                                 rb::DescartesPoint(200,200,100,180,0,0),
////                                 rb::JointPoint(0,0,0,0,0,0),
////                                 "TCP_dh_gripper",
////                                 "Plane_1",
////                                 500,
////                                 2500,
////                                 100,
////                                 rb::MoveL,
////                                 false, false, 0, true));

////    hansRobot->pushCommand( rb::HansCommand::WayPoint(0,
////                                 rb::DescartesPoint(200,0,100,180,0,0),
////                                 rb::JointPoint(0,0,0,0,0,0),
////                                 "TCP_dh_gripper",
////                                 "Plane_1",
////                                 500,
////                                 2500,
////                                 100,
////                                 rb::MoveL,
////                                 false, false, 0, true));
////    }
//}
