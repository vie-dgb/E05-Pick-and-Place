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
  calibCamera_UiInitialize();
  model_UiInitialize();
  plate_UiInitialize();
  dhr_UiInitialize();
  ExioUiInitialize();
  PnpUiInitialize();
  // show main dashboard first
  ui->tabWidget_Main->setCurrentIndex(0);
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

  if(hansRobot->robotIsConnected()) {
    hansRobot->robotDisconnect();
  }

  if(cameraControl->isCameraConnected()) {
    cameraControl->cameraDisconnect();
  }

  if(flexPlate->isFeederConnected()) {
    flexPlate->FeederDisconnect();
  }

  if(dhController->DH_IsConnected()) {
    dhController->DH_Disconnect();
  }

  // free memories and delete objects
  hansRobot->deleteLater();
  cameraControl->deleteLater();
  calibCam->deleteLater();
  flexPlate->deleteLater();
//  dhController->deleteLater();
  modbus_exio->deleteLater();
  delete matcher;

  // accept event (accept close application)
  event->accept();
}

void MainWindow::initUiEvent() {
  updateInfoTimer = new QTimer;
  updateInfoTimer->setInterval(updateCyclicTime);
  updateInfoTimer->start();
}

void MainWindow::PnpUiInitialize() {
  pnp_controller_ = new PnpController(hansRobot, matcher, coorCvt, flexPlate, dhController);
  pnp_controller_->PnpControllerInit();

  connect(pnp_controller_, &PnpController::PnpSignal_StateChanged, this,
          [this] (PnpController::PnpState state) {
    ui->label_dashboard_pnp_state->setText("PNP state: " + PnpController::StateToQString(state));
  });

  connect(pnp_controller_, &PnpController::PnpSignal_HasNewMessage,
          this, [this] (QString content) {
    qDebug() << content;
    ui->statusbar->showMessage(content);
  });

  connect(pnp_controller_, &PnpController::PnpSignal_GrabFrameTriggered,
          this, [this] () {
    cameraControl->cameraTriggerSingleShot();
    retrieveMode = FrameRetrieveMode::kFramePnpRun;
  });

  connect(this, &MainWindow::PnpFrameGrabbed, pnp_controller_, &PnpController::ReceivedNewFrame);

  connect(pnp_controller_, &PnpController::PnpSignal_DisplayMatchingImage,
          this, [this] (cv::Mat frame) {
    double img_scale = 1.0;
    if(ui->label_dashboard_frame_view->height() < frame.rows) {
      img_scale = ((double)ui->label_dashboard_frame_view->height()) / ((double)frame.rows);
    }
    DisplayImageFrame(ui->label_dashboard_frame_view, frame, img_scale);

    if (matcher->matchingResult.isFoundMatchObject) {
      MatchedObjects object = matcher->matchingResult.Objects.front();
      ui->label_dashboard_object_name->setText("Matched name: " + QString::fromStdString(object.name));

      double angle = -object.angle*C_R2D;
      if (angle >= 180.0) {
        angle -= 360.0;
      } else if (angle <= -180.0) {
        angle += 360.0;
      }
      ui->label_dashboard_object_angle->setText("Matched angle: " + QString::number(angle));
      ui->label_dashboard_object_score->setText("Matched scores: " + QString::number(object.scores));
    }
  });
}

void MainWindow::connectUiEvent() {
  /// TIMER EVENTS
  connect(updateInfoTimer, &QTimer::timeout,
          this, &MainWindow::on_Timeout_UpdateUiInfo);

  /// COMBOBOX EVENTS
  connect(ui->comboBox_Language, &QComboBox::currentIndexChanged,
          this, &MainWindow::ui_Language_Load);

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
  connect(ui->btn_dashboard_start, &QPushButton::clicked,
          this, &MainWindow::on_Click_Dashboard_Start);
  connect(ui->btn_dashboard_stop, &QPushButton::clicked,
          this, &MainWindow::on_Click_Dashboard_Stop);

  connect(ui->btn_robot_Connect, &QPushButton::clicked,
          this, &MainWindow::on_Click_Robot_Connect);
  connect(ui->btn_robot_enable, &QPushButton::clicked,
          this, &MainWindow::on_Click_Robot_Enable);
  connect(ui->btn_robot_close, &QPushButton::clicked,
          this, &MainWindow::on_Click_Robot_Close);
  connect(ui->btn_robot_gripperToggle, &QPushButton::clicked,
          this, &MainWindow::on_Click_Robot_GripperToggle);

  connect(ui->btn_Camera_Connect, &QPushButton::clicked,
          this, &MainWindow::on_Click_Camera_Connect);
  connect(ui->btn_Camera_Stream, &QPushButton::clicked,
          this, &MainWindow::on_Click_Camera_Stream);
  connect(ui->btn_Camera_SingleShot, &QPushButton::clicked,
          this, &MainWindow::on_Click_Camera_SingleShot);
  connect(ui->btn_Calib_Camera, &QPushButton::clicked,
          this, &MainWindow::on_Click_Camera_Calib);

  connect(ui->btn_Model_Add, &QPushButton::clicked,
          this, &MainWindow::on_Click_Model_Add);
  connect(ui->btn_Model_Delete, &QPushButton::clicked,
          this, &MainWindow::on_Click_Model_Delete);
  connect(ui->list_Model_ViewList, &QListWidget::currentRowChanged,
          this, &MainWindow::on_ViewList_CurrentRowChanged_Model);
  connect(ui->list_Model_ViewList, &QListWidget::doubleClicked,
          this, &MainWindow::on_ViewList_DoubleClick_Model);
  connect(ui->btn_Model_MatchingTest, &QPushButton::clicked,
          this, &MainWindow::on_Click_Model_MatchingTest);

  connect(ui->btn_Plate_Connect, &QPushButton::clicked,
          this, &MainWindow::on_Click_Plate_Connect);
  connect(ui->btn_PlateLightSwitch, &QPushButton::clicked,
          this, &MainWindow::on_Click_Plate_PlateLightSwitch);

  connect(ui->btn_dhr_serial_connect, &QPushButton::clicked,
          this, &MainWindow::on_Click_Dhr_serial_connect);

  connect(ui->btn_expandio_connect, &QPushButton::clicked,
          this, &MainWindow::on_click_exio_connect);

  connect(ui->btn_Setting_Load, &QPushButton::clicked,
          this, &MainWindow::on_Click_Setting_Load);
  connect(ui->btn_Setting_Save, &QPushButton::clicked,
          this, &MainWindow::on_Click_Setting_Save);
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
  coorCvt = new CoordinateCvt;
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

  ui->label_robot_gripper_state->setText("Gripper state: " + rb::DhGripStateToQString(hansRobot->DHGripper_GetState()));

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
  cv::rotate(frame, frame, cv::ROTATE_180);
  switch (retrieveMode) {
    case FrameRetrieveMode::kFrameStream:
      camera_UpdateViewFrame(frame);
      break;
    case FrameRetrieveMode::kFrameSingleShot:
      camera_UpdateViewFrame(frame);
      break;
    case FrameRetrieveMode::kFrameRoiCalibration:
      emit calibCamera_UpdateFrame(frame);
      break;
    case FrameRetrieveMode::kFrameMatchingTest:
      model_MatchingTest(frame);
      break;
    case FrameRetrieveMode::kFramePnpRun:
      Mat newMat = imageCropper->cropRuntimeImage(frame);
      emit PnpFrameGrabbed(newMat);
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

void MainWindow::calibCamera_UiInitialize() {
  imageCropper = new ImageCropper;
  calibCam = new CalibCamera(this, imageCropper);
  connect(this, &MainWindow::calibCamera_UpdateFrame, calibCam, &CalibCamera::updateNewFrame);
  connect(calibCam, &CalibCamera::calibCameraDone, this, &MainWindow::calibCamera_Closed);
}

void MainWindow::calibCamera_Closed(bool status) {
  cameraControl->cameraStopStream();
}

void MainWindow::model_UiInitialize() {
  matcher = new ImageMatch::GeoMatch;
  matcher->pickingBoxSize.height = 100;
  matcher->pickingBoxSize.width = 40;
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

void MainWindow::model_MatchingTest(cv::Mat image) {
  Mat newMat = imageCropper->cropRuntimeImage(image);
  matcher->matching(newMat, false, 50);
  camera_UpdateViewFrame(matcher->matchingResult.Image.clone());
  rb::DescartesPoint pointPick;
  coorCvt->xMaxPickDistance = 321.0;
  coorCvt->yMaxPickDistance = 196.0;
  if(matcher->matchingResult.Objects.size() == 0) {
    return;
  }

  ImageMatch::MatchedObjects matchObj = matcher->matchingResult.Objects.front();
  Size imageSize;
  imageSize.width = newMat.cols;
  imageSize.height = newMat.rows;
  coorCvt->convertPickCoordinates(pointPick, matchObj.coordinates, matchObj.angle, imageSize);

  qDebug() << "Image width: " << imageSize.width;
  qDebug() << "Image height: " << imageSize.height;
  qDebug() << "X position: " << pointPick.X;
  qDebug() << "Y position: " << pointPick.Y;
  qDebug() << "Angle: " << pointPick.rZ;
}

void MainWindow::DisplayImageFrame(QLabel *lableContainer, cv::Mat image, double scale_factor) {
  cv::Mat tempFrame = image.clone();
  cv::Mat cvRGBFrame;
//  double scaleFactor = (double)lableContainer->minimumHeight() / (double)tempFrame.rows;
  // resize and change color format image to display
  cv::resize(tempFrame, cvRGBFrame, cv::Size(), scale_factor, scale_factor);
  cv::cvtColor(cvRGBFrame, cvRGBFrame, cv::COLOR_BGR2RGB);
  QImage qDisplayFrame = QImage((uchar*)cvRGBFrame.data, cvRGBFrame.cols, cvRGBFrame.rows, cvRGBFrame.step, QImage::Format_RGB888);
  lableContainer->setPixmap(QPixmap::fromImage(qDisplayFrame));
}

void MainWindow::plate_UiInitialize() {
  flexPlate = new FlexibleFeed;
  connect(flexPlate, &FlexibleFeed::FeederConnected, this, [this]() {
    ui->btn_Plate_Connect->setText(lb_plate_Disconnect);
    ui->label_Plate_Connect->setText(lb_plate_Connected);
  });

  connect(flexPlate, &FlexibleFeed::FeederDisconnected, this, [this]() {
    ui->btn_Plate_Connect->setText(lb_plate_Connect);
    ui->label_Plate_Connect->setText(lb_plate_Disconnected);
  });

  connect(flexPlate, &FlexibleFeed::FeederConnectInitFail, this, [this](QString msg) {
    ui->label_Plate_Connect->setText(lb_plate_Disconnected);
    QMessageBox::information(this,lb_plate_dialog_ConnectInfo, lb_plate_ConnectFail + msg, QMessageBox::Ok);
  });

  connect(flexPlate, &FlexibleFeed::FeederConnectFail, this, [this]() {
    ui->label_Plate_Connect->setText(lb_plate_Disconnected);
    QMessageBox::information(this,lb_plate_dialog_ConnectInfo, lb_plate_ConnectTimeout, QMessageBox::Ok);
  });

  connect(flexPlate, &FlexibleFeed::FeederConnecting, this, [this]() {
    ui->label_Plate_Connect->setText(lb_plate_Connecting);
    ui->label_Plate_IP->setText(lb_plate_dialog_Address + ": " +plateAddress);
    ui->label_Plate_Port->setText(lb_plate_dialog_ServerPort + ": " + QString::number(plateServerPort));
  });

  connect(flexPlate, &FlexibleFeed::FeederReadDone, this, [this](FlexibleFeed::FeederData newData) {
    ui->label_Plate_VersionNumber->setText(tr(lb_plate_VersionNumber.toUtf8()).arg(newData.versionNumber/100.0));
    if(newData.lightSourceSwitch) {
      ui->btn_PlateLightSwitch->setText(lb_plate_LightOff);
    }
    else {
      ui->btn_PlateLightSwitch->setText(lb_plate_LightOn);
    }
    ui->spinBox_PlateLightLuminance->setValue(newData.lightSourceLuminance);
    ui->spinBox_PlateOperateDelay->setValue(newData.lightDelayOffTme);
    if(newData.lightSourceMode == FlexibleFeed::LightSourceMode::Follow) {
      ui->radioButton_PlateFollowOperate->setChecked(true);
      ui->radioButton_PlateManualOperate->setChecked(false);
    }
    else if(newData.lightSourceMode == FlexibleFeed::LightSourceMode::Manual){
      ui->radioButton_PlateFollowOperate->setChecked(false);
      ui->radioButton_PlateManualOperate->setChecked(true);
    }
  });

  connect(ui->spinBox_PlateLightLuminance, &QSpinBox::editingFinished, this, [this] () {
    flexPlate->writeLightLuminance(ui->spinBox_PlateLightLuminance->value());
  });

  connect(ui->spinBox_PlateOperateDelay, &QSpinBox::editingFinished, this, [this] () {
    flexPlate->writeLightDelayOffTime(ui->spinBox_PlateOperateDelay->value());
  });

  connect(ui->radioButton_PlateManualOperate, &QRadioButton::clicked, this, [this] () {
    flexPlate->writeLightControlMode(FlexibleFeed::LightSourceMode::Manual);
  });

  connect(ui->radioButton_PlateFollowOperate, &QRadioButton::clicked, this, [this] () {
    flexPlate->writeLightControlMode(FlexibleFeed::LightSourceMode::Follow);
  });

  ////
  connect(ui->btn_PlateScatt, &QRadioButton::clicked, this, [this] () {
    flexPlate->writeCommunicateMode(FlexibleFeed::CommunicationMode::Scatter);
  });

  connect(ui->btn_PlateLowerLeft, &QRadioButton::clicked, this, [this] () {
    flexPlate->writeCommunicateMode(FlexibleFeed::CommunicationMode::BottomLeft);
  });

  connect(ui->btn_PlateUpper, &QRadioButton::clicked, this, [this] () {
    flexPlate->writeCommunicateMode(FlexibleFeed::CommunicationMode::FallingRise);
  });

  connect(ui->btn_PlateLowerRight, &QRadioButton::clicked, this, [this] () {
    flexPlate->writeCommunicateMode(FlexibleFeed::CommunicationMode::LowerRight);
  });

  connect(ui->btn_PlateUpperRight, &QRadioButton::clicked, this, [this] () {
    flexPlate->writeCommunicateMode(FlexibleFeed::CommunicationMode::UpperRight);
  });

  connect(ui->btn_PlateLower, &QRadioButton::clicked, this, [this] () {
    flexPlate->writeCommunicateMode(FlexibleFeed::CommunicationMode::Under);
  });

  connect(ui->btn_PlateUpperLeft, &QRadioButton::clicked, this, [this] () {
    flexPlate->writeCommunicateMode(FlexibleFeed::CommunicationMode::UpperLeft);
  });

  connect(ui->btn_PlateRight, &QRadioButton::clicked, this, [this] () {
    flexPlate->writeCommunicateMode(FlexibleFeed::CommunicationMode::Right);
  });

  connect(ui->btn_PlateLeft, &QRadioButton::clicked, this, [this] () {
    flexPlate->writeCommunicateMode(FlexibleFeed::CommunicationMode::LeftSide);
  });

  connect(ui->btn_PlateUpAndDown, &QRadioButton::clicked, this, [this] () {
    flexPlate->writeCommunicateMode(FlexibleFeed::CommunicationMode::HighAndLow);
  });

  connect(ui->btn_PlateAround, &QRadioButton::clicked, this, [this] () {
    flexPlate->writeCommunicateMode(FlexibleFeed::CommunicationMode::LeftAndRightSide);
  });

  connect(ui->btn_PlateStop, &QRadioButton::clicked, this, [this] () {
    flexPlate->writeCommunicateMode(FlexibleFeed::CommunicationMode::Stop);
  });


  ui->label_Plate_IP->setText(lb_plate_dialog_Address + ": *");
  ui->label_Plate_Port->setText(lb_plate_dialog_ServerPort + ": *");

  plateAddress = "192.168.1.247";
  plateServerPort = 502;
}

bool MainWindow::plate_UserInputAddress() {
  // setup input dialog form
  InputFormDialog::FormData data;
  data[lb_plate_dialog_Address] = plateAddress;
  data[lb_plate_dialog_ServerPort] = plateServerPort;
  // limits server port number
  InputFormDialog::FormOptions options;
  options.numericMin = 0;
  options.numericMax = 65535;

  while(true) {
    if(InputFormDialog::getInput("Enter Vibrating address", data, options)) {
      QHostAddress h_address(data.at<QString>(lb_plate_dialog_Address));
      if(h_address.protocol() == QAbstractSocket::IPv4Protocol) {
        plateAddress = data.at<QString>(lb_plate_dialog_Address);
        plateServerPort = data.at<int>(lb_plate_dialog_ServerPort);
        return true;
      }
    }
    else {
      return false;
    }
  }
}

void MainWindow::dhr_UiInitialize() {
  dhController = new DHController(QThread::NormalPriority, this);

  connect(dhController, &DHController::DHSignal_Connected, this, [this] () {
    ui->btn_dhr_serial_connect->setText("Disconnect");
    ui->label_dhr_serial_connect->setText("Serial port connected");
    ui->label_dhr_serial_name->setText("Port name: " + serial_port_name);
    if (ui->widget_Rgi_Control->IsAutoInit()) {
      dhController->DH_AddFuncToQueue(DH_RGI::SetInitDevice(rgi_address));
    }
    if (ui->widget_Pgc_Control->IsAutoInit()) {
      dhController->DH_AddFuncToQueue(DH_PGC::SetInitDevice(pgc_address));
    }
    ui->btn_dhr_serial_connect->setEnabled(true);
  });

  connect(dhController, &DHController::DHSignal_Disconnected, this, [this] () {
    ui->btn_dhr_serial_connect->setText("Connect");
    ui->label_dhr_serial_connect->setText("Serial port no connection");
    ui->label_dhr_serial_name->setText("Port name: *");
    ui->widget_Rgi_Control->SetSlaveEditBoxEnable(true);
    ui->widget_Pgc_Control->SetSlaveEditBoxEnable(true);
    ui->btn_dhr_serial_connect->setEnabled(true);
  });

  connect(dhController, &DHController::DHSignal_Connecting, this, [this] () {
    ui->btn_dhr_serial_connect->setText("Connecting");
    ui->label_dhr_serial_connect->setText("Serial port connecting");
  });

  connect(dhController, &DHController::DHSignal_ConnectFail, this,
          [this] (QString msg) {
    ui->btn_dhr_serial_connect->setText("Connect");
//    ui->label_dhr_serial_connect->setText("Serial port no connection");
    ui->label_dhr_serial_connect->setText("Serial " + msg);
    ui->label_dhr_serial_name->setText("Port name: *");
    ui->widget_Rgi_Control->SetSlaveEditBoxEnable(true);
    ui->widget_Pgc_Control->SetSlaveEditBoxEnable(true);
    ui->btn_dhr_serial_connect->setEnabled(true);
  });

  connect(dhController, &DHController::DHSignal_ErrorOccured, this,
          [this] (QString msg) {
    ui->statusbar->showMessage(msg, 5000);
  });

  connect(dhController, &DHController::DHSignal_PollingDisplayTriggered,
          this, &MainWindow::dhr_DisplayRgiInfo);

//  connect(dhController, &DHController::DHSignal_PgcGripperStateChanged,
//          this, [this] (DhGripperStatus state) {
//    ui->statusbar->showMessage("PGC Gripper: " + dhr::EnumConvert(state) +
//                               " - " + QString::number(test_counter));
//    test_counter += 1;
//  });

//  connect(dhController, &DHController::DHSignal_RgiGripperStateChanged,
//          this, [this] (DhGripperStatus state) {
//    ui->statusbar->showMessage("RGI Gripper: " + dhr::EnumConvert(state) +
//                               " - " + QString::number(test_counter));
//    test_counter += 1;
//  });

//  connect(dhController, &DHController::DHSignal_RgiRotateStateChanged,
//          this, [this] (DhRotationStatus state) {
//    ui->statusbar->showMessage("RGI Rotation: " + dhr::EnumConvert(state) +
//                               " - " + QString::number(test_counter));
//    test_counter += 1;
//  });

  connect(ui->widget_Rgi_Control, &DhRgiWidget::SignalsRgiInitialize, this, [this] () {
    dhController->DH_AddFuncToQueue(DH_RGI::SetInitDevice(rgi_address));
  });

  connect(ui->widget_Rgi_Control, &DhRgiWidget::SignalsGripper_PositionEdited,
          this, [this] (int value) {
    dhController->DH_AddFuncToQueue(
        DH_RGI::SetGripperPosition(rgi_address, value));
  });

  connect(ui->widget_Rgi_Control, &DhRgiWidget::SignalsGripper_ForceEdited,
          this, [this] (int value) {
    dhController->DH_AddFuncToQueue(
        DH_RGI::SetGripperForce(rgi_address, value));
  });

  connect(ui->widget_Rgi_Control, &DhRgiWidget::SignalsGripper_SpeedEdited,
          this, [this] (int value) {
    dhController->DH_AddFuncToQueue(
        DH_RGI::SetGripperSpeed(rgi_address, value));
  });

  connect(ui->widget_Rgi_Control, &DhRgiWidget::SignalsRotation_AngleEdited,
          this, [this] (int value) {
    dhController->DH_AddFuncToQueue(
        DH_RGI::SetRotationAngle(rgi_address, value));
  });

  connect(ui->widget_Rgi_Control, &DhRgiWidget::SignalsRotation_TorqueEdited,
          this, [this] (int value) {
    dhController->DH_AddFuncToQueue(
        DH_RGI::SetRotationTorque(rgi_address, value));
  });

  connect(ui->widget_Rgi_Control, &DhRgiWidget::SignalsRotation_SpeedEdited,
          this, [this] (int value) {
    dhController->DH_AddFuncToQueue(
        DH_RGI::SetRotationSpeed(rgi_address, value));
  });

  connect(ui->widget_Pgc_Control, &DhPgcWidget::SignalsRgiInitialize, this, [this] () {
    dhController->DH_AddFuncToQueue(DH_PGC::SetInitDevice(pgc_address));
  });

  connect(ui->widget_Pgc_Control, &DhPgcWidget::SignalsGripper_PositionEdited,
          this, [this] (int value) {
    dhController->DH_AddFuncToQueue(
        DH_PGC::SetGripperPosition(pgc_address, value));
  });

  connect(ui->widget_Pgc_Control, &DhPgcWidget::SignalsGripper_ForceEdited,
          this, [this] (int value) {
    dhController->DH_AddFuncToQueue(
        DH_PGC::SetGripperForce(pgc_address, value));
  });

  connect(ui->widget_Pgc_Control, &DhPgcWidget::SignalsGripper_SpeedEdited,
          this, [this] (int value) {
    dhController->DH_AddFuncToQueue(
        DH_PGC::SetGripperSpeed(pgc_address, value));
  });
}

void MainWindow::dhr_DisplayRgiInfo() {
  RGIData rgi_data;
  PGCData pgc_data;
  if (!dhController->DH_GetRgiData(rgi_address, rgi_data)) {
    this->statusBar()->showMessage("Get device info fail");
    return;
  }
  if (!dhController->DH_GetPgcData(pgc_address, pgc_data)) {
    this->statusBar()->showMessage("Get device info fail");
    return;
  }

  ui->widget_Rgi_Control->ShowRgiDeviceInfo(rgi_data.feedback);
  ui->widget_Pgc_Control->ShowPgcDeviceInfo(pgc_data.feedback);
}

void MainWindow::ExioUiInitialize() {
  modbus_exio = new ModbusExpandIO;
  connect(modbus_exio, &ModbusExpandIO::ModuleIoConnected, this, [this]() {
    ui->btn_expandio_connect->setText(lb_plate_Disconnect);
    ui->label_expandio_connect->setText(lb_plate_Connected);
  });

  connect(modbus_exio, &ModbusExpandIO::ModuleIoDisconnected, this, [this]() {
    ui->btn_expandio_connect->setText(lb_plate_Connect);
    ui->label_expandio_connect->setText(lb_plate_Disconnected);
  });

  connect(modbus_exio, &ModbusExpandIO::ModuleIoConnectInitFail, this, [this](QString msg) {
    ui->label_expandio_connect->setText(lb_plate_Disconnected);
    QMessageBox::information(this,lb_plate_dialog_ConnectInfo, lb_plate_ConnectFail + msg, QMessageBox::Ok);
  });

  connect(modbus_exio, &ModbusExpandIO::ModuleIoConnectFail, this, [this]() {
    ui->label_expandio_connect->setText(lb_plate_Disconnected);
    QMessageBox::information(this,lb_plate_dialog_ConnectInfo, lb_plate_ConnectTimeout, QMessageBox::Ok);
  });

  connect(modbus_exio, &ModbusExpandIO::ModuleIoConnecting, this, [this]() {
    ui->label_expandio_connect->setText(lb_plate_Connecting);
    ui->label_expandio_ip->setText(lb_plate_dialog_Address + ": " + modbus_exio_address);
    ui->label_expandio_port->setText(lb_plate_dialog_ServerPort + ": " + QString::number(modbus_exio_port));
  });

  connect(modbus_exio, &ModbusExpandIO::ModuleIoDataChanged, this, [this](ExpandIo io) {
    ui->checkBox_exio_out_0_0->setChecked(io.GetOutputAt(0));
    ui->checkBox_exio_out_0_1->setChecked(io.GetOutputAt(1));
    ui->checkBox_exio_out_0_2->setChecked(io.GetOutputAt(2));
    ui->checkBox_exio_out_0_3->setChecked(io.GetOutputAt(3));
    ui->checkBox_exio_out_0_4->setChecked(io.GetOutputAt(4));
    ui->checkBox_exio_out_0_5->setChecked(io.GetOutputAt(5));
  });

//  connect(ui->checkBox_exio_out_0_0, &QCheckBox::released, this, [this] () {
//    qDebug() << "Change coils: " << ui->checkBox_exio_out_0_0->isChecked();
////    modbus_exio->WriteCoils(0, ui->checkBox_exio_out_0_0->isChecked());
//  });

//  connect(ui->checkBox_exio_out_0_1, &QCheckBox::released, this, [this] () {
////    modbus_exio->WriteCoils(1, ui->checkBox_exio_out_0_1->isChecked());
//  });

//  modbus_exio_address = "192.168.1.12";
//  modbus_exio_port = 502;
}

bool MainWindow::ExioUserInputAddress() {
  // setup input dialog form
  InputFormDialog::FormData data;
  data[lb_plate_dialog_Address] = modbus_exio_address;
  data[lb_plate_dialog_ServerPort] = modbus_exio_port;
  // limits server port number
  InputFormDialog::FormOptions options;
  options.numericMin = 0;
  options.numericMax = 65535;

  while(true) {
    if(InputFormDialog::getInput("Enter Expand module IO (Modbus TCP) address", data, options)) {
      QHostAddress h_address(data.at<QString>(lb_plate_dialog_Address));
      if(h_address.protocol() == QAbstractSocket::IPv4Protocol) {
        modbus_exio_address = data.at<QString>(lb_plate_dialog_Address);
        modbus_exio_port = data.at<int>(lb_plate_dialog_ServerPort);
        return true;
      }
    }
    else {
      return false;
    }
  }
}

//////////  TIMER ACTIONS
void MainWindow::on_Timeout_UpdateUiInfo() {
  if(hansRobot->robotIsConnected()) {
    // update robot info in UI
    robot_UiUpdate();
  }
}

//////////  BUTTON ACTIONS
void MainWindow::on_Click_Dashboard_Start() {
  pnp_controller_->PnpControllerStart();
}

void MainWindow::on_Click_Dashboard_Stop() {
  pnp_controller_->PnpControllerStop();
}

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
      hansRobot->DHGripper_Open();
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
    retrieveMode = FrameRetrieveMode::kFrameStream;
  }
  else {
    cameraControl->cameraStopStream();
    ui->btn_Camera_Stream->setText("Stream");
    ui->btn_Camera_SingleShot->setEnabled(true);
  }
}

void MainWindow::on_Click_Camera_SingleShot() {
  if(!cameraControl->isCameraStreaming()) {
    retrieveMode = FrameRetrieveMode::kFrameSingleShot;
    cameraControl->cameraTriggerSingleShot();
  }
}

void MainWindow::on_Click_Camera_Calib() {
  calibCam->show_CalibDialog();
  // start stream
  retrieveMode = FrameRetrieveMode::kFrameRoiCalibration;
  if(!cameraControl->isCameraStreaming()) {
    cameraControl->cameraStartStream();
    ui->btn_Camera_Stream->setText("Stop");
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
  cv::Mat show_mat = modelSrc.at(currentRow).getImageOfModel().clone();
  double img_scale = ui->label_Model_PreView->maximumHeight() / (double)show_mat.rows;
  DisplayImageFrame(ui->label_Model_PreView, show_mat, img_scale);
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

void MainWindow::on_Click_Model_MatchingTest() {
  if(!cameraControl->isCameraConnected()) {
    return;
  }

  cameraControl->cameraTriggerSingleShot();
  retrieveMode = FrameRetrieveMode::kFrameMatchingTest;
}

void MainWindow::on_Click_Plate_Connect() {
  if(flexPlate->isFeederConnected()) {
    flexPlate->FeederDisconnect();
  }
  else {
    if(plate_UserInputAddress()) {
      flexPlate->FeederConnect(plateAddress, plateServerPort);
    }
  }
}

void MainWindow::on_click_exio_connect() {
  if(modbus_exio->IsModuleIoConnected()) {
    modbus_exio->ModuleIoDisconnect();
  }
  else {
    if(ExioUserInputAddress()) {
      modbus_exio->ModuleIoConnect(modbus_exio_address, modbus_exio_port);
    }
  }
}

void MainWindow::on_Click_Plate_PlateLightSwitch() {
  FlexibleFeed::FeederData m_data = flexPlate->getFeederData();
  flexPlate->writeLightSwitch(!m_data.lightSourceSwitch);
}

void MainWindow::on_Click_Dhr_serial_connect() {
  if(!dhController->DH_IsConnected()) {
    SerialSettingDialog *serial_dialog = new SerialSettingDialog(this);
    connect(serial_dialog, &SerialSettingDialog::UserAcceptSerialSetting,
            this, [this] (SerialSetting setting) {
      if (setting.name.isEmpty()) { return; }
      serial_port_name = setting.name;
      rgi_address = ui->widget_Rgi_Control->GetSlaveAddress();
      ui->widget_Rgi_Control->SetSlaveEditBoxEnable(false);
      pgc_address = ui->widget_Pgc_Control->GetSlaveAddress();
      ui->widget_Pgc_Control->SetSlaveEditBoxEnable(false);
      ui->btn_dhr_serial_connect->setEnabled(false);
      dhController->DH_SetRgiAddress(rgi_address);
      dhController->DH_SetPgcAddress(pgc_address);
      dhController->DH_Connect(setting);
    });
    serial_dialog->ShowDialog();
  } else {
    dhController->DH_Disconnect();
  }
}

void MainWindow::on_Click_Setting_Load() {
  QString fullPath = QFileDialog::getOpenFileName(this, "Choose setting file", "/home", "Json (*.json)");
  SettingHandler setting;
  setting.load(fullPath);

  imageCropper->setCropPointRuntimeImage(setting.matchingROI.TopLeft, 0);
  imageCropper->setCropPointRuntimeImage(setting.matchingROI.BottomRight, 1);
}

void MainWindow::on_Click_Setting_Save() {
  QString fullPath = QFileDialog::getSaveFileName(this, "Save setting file", "/", "Json (*.json)");
  SettingHandler setting;

  setting.matchingROI.TopLeft = imageCropper->getCropPointRuntimeImage(0);
  setting.matchingROI.BottomRight = imageCropper->getCropPointRuntimeImage(1);

  setting.save(fullPath);
}
