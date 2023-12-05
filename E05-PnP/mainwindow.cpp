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
    camera_UiInitialize();
    model_UiInitialize();
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

    /// RADIO BUTTON
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

    /// BUTTON EVENTS
    connect(ui->btn_robot_Connect, &QPushButton::clicked, this, &MainWindow::on_Click_Robot_Connect);
    connect(ui->btn_robot_enable, &QPushButton::clicked, this, &MainWindow::on_Click_Robot_Enable);
    connect(ui->btn_robot_close, &QPushButton::clicked, this, &MainWindow::on_Click_Robot_Close);
    connect(ui->btn_robot_gripperToggle, &QPushButton::clicked, this, &MainWindow::on_Click_Robot_GripperToggle);

    connect(ui->btn_Camera_Connect, &QPushButton::clicked, this, &MainWindow::on_Click_Camera_Connect);
    connect(ui->btn_Camera_Stream, &QPushButton::clicked, this, &MainWindow::on_Click_Camera_Stream);
    connect(ui->btn_Camera_SingleShot, &QPushButton::clicked, this, &MainWindow::on_Click_Camera_SingleShot);

    connect(ui->btn_Model_Add, &QPushButton::clicked, this, &MainWindow::on_Click_Model_Add);
    connect(ui->btn_Model_Delete, &QPushButton::clicked, this, &MainWindow::on_Click_Model_Delete);
    connect(ui->list_Model_ViewList, &QListWidget::currentRowChanged, this, &MainWindow::on_ViewList_CurrentRowChanged_Model);
    connect(ui->list_Model_ViewList, &QListWidget::doubleClicked, this, &MainWindow::on_ViewList_DoubleClick_Model);
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

    hansRobot->DHGripper_Setup(0, 1, 0, 1);

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
    switch (robotState.MachineState) {
    case HansMachineState::Disable:
        ui->btn_robot_enable->setText("Enable");
        ui->btn_robot_enable->setEnabled(true);
        break;
    case HansMachineState::StandBy:
        ui->btn_robot_enable->setText("Disable");
        break;
    case HansMachineState::Blackout_48V:
        ui->btn_robot_enable->setText("Open");
        break;
    case HansMachineState::EmergencyStop:
        ui->btn_robot_enable->setText("Reset");
        break;
    case HansMachineState::Electrifying48V:
        ui->btn_robot_enable->setText("Waiting");
        ui->btn_robot_enable->setEnabled(false);
        break;
    }

    if(hansRobot->DHGripper_IsOpen()) {
        ui->btn_robot_gripperToggle->setText("Close gripper");
    }
    else {
        ui->btn_robot_gripperToggle->setText("Open gripper");
    }

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

void MainWindow::camera_UiInitialize() {
    cameraControl = new Vision::PylonGrab;

    connect(cameraControl, &Vision::PylonGrab::signal_cameraConnected, this, [this] () {
        CDeviceInfo device = cameraControl->getDeviceInfo();
        ui->btn_Camera_Connect->setText("Disconnect");
        ui->label_Camera_ConnectStatus->setText("Camera connected");
        ui->label_Camera_Address->setText("IP Address: " + QString::fromUtf8(device.GetAddress().c_str()));
        ui->label_Camera_Name->setText("Name: " + QString::fromUtf8(device.GetModelName().c_str()));
    });

    connect(cameraControl, &Vision::PylonGrab::signal_cameraDisconnected, this, [this] () {
        ui->btn_Camera_Connect->setText("Connnect");
        ui->label_Camera_ConnectStatus->setText("No camera connection");
        ui->label_Camera_Address->setText("IP Address: ??");
        ui->label_Camera_Name->setText("Name: ??");
    });

    connect(cameraControl, &Vision::PylonGrab::signal_NewFrameRead,
            this, &MainWindow::camera_GotNewFrame);
}

void MainWindow::camera_GotNewFrame(cv::Mat frame) {
    switch (retrieveMode) {
    case FrameRetrieveMode::STREAM:
        camera_UpdateViewFrame(frame);
        break;
    case FrameRetrieveMode::SINGLE_SHOT:
        camera_UpdateViewFrame(frame);
        break;
    case FrameRetrieveMode::CALIB_CAMERA:
//        emit calibCamera_UpdateFrame(frame);
        break;
    case FrameRetrieveMode::MATCHING_TEST:
//        process_MactchingTest(imageCropper->cropRuntimeImage(frame));
        break;
    case FrameRetrieveMode::MATCHING_AUTO:
//        cv::Mat matchingFrame = imageCropper->cropRuntimeImage(frame);
//        matcher->matchingResult.imageCols = matchingFrame.cols;
//        matcher->matchingResult.imageRows = matchingFrame.rows;
//        controller->ImageProcessing(matchingFrame);
//        //        emit controller->image(matchingFrame);
        break;
    }
}

void MainWindow::camera_UpdateViewFrame(cv::Mat frame) {
    if(ui->comboBox_Camera_ViewMode->currentText() == "Full size mode") {
        ui->label_Camera_FrameView->setMaximumHeight(frame.rows);
        ui->label_Camera_FrameView->setMaximumWidth(frame.cols);
    }
    else if(ui->comboBox_Camera_ViewMode->currentText() == "Fit size mode") {
        ui->label_Camera_FrameView->setMaximumHeight(ui->scrollArea->geometry().height());
        ui->label_Camera_FrameView->setMaximumWidth(ui->scrollArea->geometry().width());
    }

    cv::Mat cvRGBFrame;
    double scaleFactor = (double)ui->label_Camera_FrameView->maximumHeight() / (double)frame.rows;
    cv::resize(frame, cvRGBFrame, cv::Size(), scaleFactor, scaleFactor);
    cv::cvtColor(cvRGBFrame, cvRGBFrame, cv::COLOR_BGR2RGB);
    QImage qDisplayFrame = QImage((uchar*)cvRGBFrame.data, cvRGBFrame.cols, cvRGBFrame.rows, cvRGBFrame.step, QImage::Format_RGB888);
    ui->label_Camera_FrameView->setPixmap(QPixmap::fromImage(qDisplayFrame));
}

void MainWindow::model_UiInitialize() {
    matcher = new ImageMatch::GeoMatch;

}

void MainWindow::model_UpdateViewList() {
    vector<ImageMatch::GeoModel> modelSrc = matcher->getModelSource();

    ui->list_Model_ViewList->clear();

    for(int listCounter=0;listCounter<modelSrc.size();listCounter++) {
        ui->list_Model_ViewList->addItem(QString::fromStdString(modelSrc.at(listCounter).nameOfModel));
    }
}

void MainWindow::model_UpdateTemplateViewInfo() {
    vector<ImageMatch::GeoModel> modelSrc = matcher->getModelSource();
    int currentRow = ui->list_Model_ViewList->currentRow();
    if(currentRow >= 0) {
        ui->label_Model_Name->setText("Model: " + QString::fromStdString(modelSrc[currentRow].nameOfModel));
        ui->label_Model_FileName->setText("File name: " + QString::fromStdString(modelSrc[currentRow].getModelFileName()));
        ui->label_Model_MinScores->setText("Min scores: " + QString::number(modelSrc[currentRow].minScores));
    }
}

void MainWindow::model_PatternEdit(ImageMatch::GeoModel model) {
    matcher->modifyMatchModelAt(currentModelModifyIndex, model);
    model_UpdateViewList();
    ui->list_Model_ViewList->setCurrentRow(currentModelModifyIndex);
}

void MainWindow::DisplayImageFrame(QLabel *lableContainer, cv::Mat image) {
    cv::Mat tempFrame = image.clone();
    cv::Mat cvRGBFrame;
    double scaleFactor = (double)lableContainer->minimumHeight() / (double)tempFrame.rows;
    // resize and change color format image to display
    cv::resize(tempFrame, cvRGBFrame, cv::Size(), scaleFactor, scaleFactor);
    cv::cvtColor(cvRGBFrame, cvRGBFrame, cv::COLOR_BGR2RGB);
    QImage qDisplayFrame = QImage((uchar*)cvRGBFrame.data, cvRGBFrame.cols, cvRGBFrame.rows, cvRGBFrame.step, QImage::Format_RGB888);
    lableContainer->setPixmap(QPixmap::fromImage(qDisplayFrame));
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

void MainWindow::on_Click_Robot_Enable() {
    HansRobotState state = hansRobot->GetRobotState();
    switch (state.MachineState) {
        case HansMachineState::Disable:
            hansRobot->pushCommand(rb::HansCommand::GrpPowerOn(0));
            break;
        case HansMachineState::StandBy:
            hansRobot->pushCommand(rb::HansCommand::GrpPowerOff(0));
            break;
        case HansMachineState::Blackout_48V:
            hansRobot->pushCommand(rb::HansCommand::Electrify());
            break;
        case HansMachineState::EmergencyStop:
            hansRobot->pushCommand(rb::HansCommand::GrpReset(0));
            break;
    }
}

void MainWindow::on_Click_Robot_Close() {
    HansRobotState state = hansRobot->GetRobotState();
    switch (state.MachineState) {
    case HansMachineState::Disable:
            hansRobot->pushCommand(rb::HansCommand::BlackOut());
            break;
    case HansMachineState::StandBy:
            hansRobot->pushCommand(rb::HansCommand::GrpPowerOff(0));
            hansRobot->pushCommand(rb::HansCommand::BlackOut());
            break;
    }
}

void MainWindow::on_Click_Robot_GripperToggle() {
    hansRobot->DHGripper_Toggle();
//    hansRobot->pushCommand(HansCommand::SetOverride(0, 1));
//    hansRobot->pushCommand(HansCommand::MoveJ(0, JointPoint(171.206,-22.911,91.417,-180.001,-65.672,119.328)));
//    hansRobot->pushCommand(HansCommand::WaitStartMove());
//    hansRobot->pushCommand(HansCommand::WaitMoveDone());
//    for(int i=0;i<10;i++) {
//        hansRobot->pushCommand(HansCommand::WayPointL(0, DescartesPoint(0,0,100,180,0,0), "TCP_dh_gripper", "Plane_1", 500, 2500, 50));
//        hansRobot->pushCommand(HansCommand::WayPointL(0, DescartesPoint(0,200,100,180,0,0), "TCP_dh_gripper", "Plane_1", 500, 2500, 50));
//        hansRobot->pushCommand(HansCommand::WayPointL(0, DescartesPoint(200,200,100,180,0,0), "TCP_dh_gripper", "Plane_1", 500, 2500, 50));
//        hansRobot->pushCommand(HansCommand::WayPointL(0, DescartesPoint(200,0,100,180,0,0), "TCP_dh_gripper", "Plane_1", 500, 2500, 50));
//    }
}

void MainWindow::on_Click_Camera_Connect() {
    if(!cameraControl->isCameraConnected()) {
            ChooseCameraDialog *cameraDialog = new ChooseCameraDialog;
            connect(cameraDialog, &ChooseCameraDialog::signal_userAccepted, this, [this] (Pylon::CDeviceInfo device) {
                cameraControl->setDeviceInfo(device);
                cameraControl->cameraConnect();
            });
            cameraDialog->showSelectedCameraDialog();
    }
    else {
            cameraControl->cameraDisconnect();
    }
}

void MainWindow::on_Click_Camera_Stream() {
    if(!cameraControl->isCameraStreaming()) {
            cameraControl->cameraStartStream();
            ui->btn_Camera_Stream->setText("Stop");
            ui->btn_Camera_SingleShot->setEnabled(false);
            retrieveMode = FrameRetrieveMode::STREAM;
    }
    else {
            cameraControl->cameraStopStream();
            ui->btn_Camera_Stream->setText("Stream");
            ui->btn_Camera_SingleShot->setEnabled(true);
    }
}

void MainWindow::on_Click_Camera_SingleShot() {
    if(!cameraControl->isCameraStreaming()) {
            retrieveMode = FrameRetrieveMode::SINGLE_SHOT;
            cameraControl->cameraTriggerSingleShot();
    }
}

void MainWindow::on_Click_Model_Add() {
    QString filePath = QFileDialog::getOpenFileName(this,
                                                    "Select template file",
                                                    choosePathModel,
                                                    "Images (*.bmp)");

    if(filePath.isEmpty()) {
            return;
    }
    choosePathModel = filePath.left(filePath.lastIndexOf('/'));

    // get sample name
    QString sampleName = QInputDialog::getText(0, "Input sample name", "Name:");
    if(sampleName.isEmpty()) {
            return;
    }

    matcher->addGeoMatchModel(filePath.toStdString(), sampleName.toStdString());

    model_UpdateViewList();
    int lastIndex = ui->list_Model_ViewList->count() - 1;
    ui->list_Model_ViewList->setCurrentRow(lastIndex);
    model_UpdateTemplateViewInfo();
}

void MainWindow::on_Click_Model_Delete() {
    int deleteIndex = ui->list_Model_ViewList->currentRow();
    if(deleteIndex < 0) {
            return;
    }

    matcher->removeMatchModel(deleteIndex);
    model_UpdateViewList();
    int lastIndex = ui->list_Model_ViewList->count() - 1;
    ui->list_Model_ViewList->setCurrentRow(lastIndex);
    model_UpdateTemplateViewInfo();
}

void MainWindow::on_ViewList_CurrentRowChanged_Model(int currentRow) {
    if(currentRow < 0) {
            return;
    }
    vector<ImageMatch::GeoModel> modelSrc = matcher->getModelSource();
    DisplayImageFrame(ui->label_Model_PreView, modelSrc.at(currentRow).getImageOfModel());
    model_UpdateTemplateViewInfo();
}

void MainWindow::on_ViewList_DoubleClick_Model() {
    int currentRow = ui->list_Model_ViewList->currentRow();
    if(currentRow < 0) {
            return;
    }

    PatternDialog *editPat = new PatternDialog;
    connect(editPat, &PatternDialog::patternModelEditFinhished,
            this, &MainWindow::model_PatternEdit);

    vector<ImageMatch::GeoModel> modelSrc = matcher->getModelSource();
    ImageMatch::GeoModel model = modelSrc.at(currentRow);
    currentModelModifyIndex = currentRow;

    editPat->showPatternEdit(model);
    model_UpdateTemplateViewInfo();
}
