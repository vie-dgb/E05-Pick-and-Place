#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QCloseEvent>
#include <QTranslator>
#include <QMessageBox>
#include <QTimer>
#include <QFileDialog>
#include <QInputDialog>

#include "dialog/InputFormDialog.h"

#include "camera/PylonGrab.h"
#include "camera/CalibCamera.h"
#include "camera/ChooseCameraDialog.h"
#include "camera/PatternDialog.h"
#include "camera/GeoMatch.h"
#include "camera/ImageCropper.h"

#include "robot/HansClient.h"
#include "robot/CoordinateCvt.h"

#include "plate/FlexibleFeed.h"
#include "dh-robotic/dhcontroller.h"
#include "dh-robotic/serialsettingdialog.h"
//#include "expandio/ModbusExpandIO.h"
#include "setting/SettingHandler.h"
#include "controller/PnpController.h"
#include "mc_protocol/fxremote.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

using namespace rb;
using namespace fx;

class MainWindow : public QMainWindow
{
  Q_OBJECT

public:
  enum LanguagesUI {
    EN = 0,
    JP,
    VN
  };

  enum FrameRetrieveMode {
    kFrameStream = 0,
    kFrameSingleShot,
    kFrameRoiCalibration,
    kFrameMatchingTest,
    kFramePnpRun,
  };

  MainWindow(QWidget *parent = nullptr);
  ~MainWindow();

private:
  /// MAINWINDOW EVENT HANDLE FUNCTIONS
  void closeEvent(QCloseEvent *event) override;
  void initUiEvent();
  void connectUiEvent();

  /// UI LANGUAGE HANDLE FUNCTIONS
  void ui_Language_Init();
  void ui_Language_Load(const int index);

  /// ROBOT UI FUNTIONS
  void robot_UiInitialize();
  bool robot_UserInputAddress();
  void robot_UiUpdate();

  /// CAMERA UI FUNCTIONS
  void camera_UiInitialize();
  void camera_GotNewFrame(cv::Mat frame);
  void camera_UpdateViewFrame(cv::Mat frame);
  void calibCamera_UiInitialize();
  void calibCamera_Closed(bool status);

  /// MODEL UI FUNCTIONS
  void model_UiInitialize();
  void model_UpdateViewList();
  void model_UpdateTemplateViewInfo();
  void model_PatternEdit(ImageMatch::GeoModel model);
  void model_MatchingTest(cv::Mat image);
  void DisplayImageFrame(QLabel *lableContainer, cv::Mat image, double scale_factor);

  /// FLEXIBLE PLATE UI FUNCTIONS
  void plate_UiInitialize();
  bool plate_UserInputAddress();

  /// DH-ROBOTICS DEVICE UI FUNCTIONS
  void dhr_UiInitialize();
  void dhr_DisplayRgiInfo();

  /// PICK AND PLACE CONTROLLER
  void PnpUiInitialize();
//  void PnpDisplayMatchingImage();

  /// TIMER ACTIONS
  void on_Timeout_UpdateUiInfo();

  /// VIEW LIST ACTIONS
  void on_ViewList_CurrentRowChanged_Model(int currentRow);
  void on_ViewList_DoubleClick_Model();

  /// BUTTON ACTIONS
  void on_Click_Dashboard_Start();
  void on_Click_Dashboard_Stop();
  void on_Click_Dashboard_Check();

  void on_Click_Robot_Connect();
  void on_Click_Robot_Enable();
  void on_Click_Robot_Close();
  void on_Click_Robot_GripperToggle();

  void on_Click_Camera_Connect();
  void on_Click_Camera_Stream();
  void on_Click_Camera_SingleShot();
  void on_Click_Camera_Calib();

  void on_Click_Model_Add();
  void on_Click_Model_Delete();
  void on_Click_Model_MatchingTest();

  void on_Click_Plate_Connect();
  void on_Click_Plate_PlateLightSwitch();

  void on_Click_Dhr_serial_connect();

  void on_Click_Setting_Load();
  void on_Click_Setting_Save();

signals:
  void calibCamera_UpdateFrame(cv::Mat frame);
  void PnpFrameGrabbed(cv::Mat frame);

private:
  Ui::MainWindow *ui;
  QTimer *updateInfoTimer;
  const int updateCyclicTime = 400;

  /// LANGUAGE
  QTranslator uiTranslator;
  LanguagesUI currentLanguage;

  /// HANS ROBOT
  rb::HansClient *hansRobot;
  QString robotAddress;
  CoordinateCvt *coorCvt;

  /// BALSER CAMERA
  Vision::PylonGrab *cameraControl;
  ImageMatch::GeoMatch *matcher;
  CalibCamera *calibCam;
  ImageCropper *imageCropper;
  QString choosePathModel = "D:/Work/ImageTest";
  FrameRetrieveMode retrieveMode;
  int currentModelModifyIndex = 0;

  /// FLEXIBLE PLATE
  FlexibleFeed *flexPlate;
  QString plateAddress;
  int plateServerPort;

  /// DH-ROBOTICS
  dhr::DHController *dhController;
  QString serial_port_name;
  int rgi_address;
  int pgc_address;
  int test_counter = 0;

  /// EXPAND IO MODULE
  FxRemote *fx_plc;

  /// CONTROLLER
  PnpController *pnp_controller_;
  bool pnp_status_dialog_showing = false;

  // UI lable text
  const QString lb_robot_Connect = "Connect";
  const QString lb_robot_Disconnect = "Disconnect";
  const QString lb_robot_Connected = "E05 Connected";
  const QString lb_robot_Disconnected = "E05 No Connection";
  const QString lb_robot_WaitConnect = "E05 Connecting";
  const QString lb_robot_StateMachine = "Robot state";
  const QString lb_robot_dialog_Address = "Ip address";
  const QString lb_robot_dialog_ConnectFail = "E05 connect fail. \nPlease check robot address.";
  const QString lb_robot_dialog_LostConnect = "E05 lost connect. \nPlease restart the robot's power.";
  const QString lb_robot_dialog_ConnectInfo = "E05 connection";

  const QString lb_plate_Connect = "Connect";
  const QString lb_plate_Disconnect = "Disconnect";
  const QString lb_plate_Connected = "Plate Connected";
  const QString lb_plate_Connecting = "Plate Connecting";
  const QString lb_plate_Disconnected = "Plate No Connection";
  const QString lb_plate_ConnectFail = "Connect to device fail: ";
  const QString lb_plate_ConnectTimeout= "Connect to device fail (timeout). \nPlease check host address and port.";
  const QString lb_plate_dialog_Address = "Ip address: ";
  const QString lb_plate_dialog_ServerPort = "Server port: ";
  const QString lb_plate_dialog_ConnectInfo = "Flexible plate connection ";
  const QString lb_plate_LightOn = "Light On";
  const QString lb_plate_LightOff = "Light Off";
  const QString lb_plate_VersionNumber = "Firmware version: %1";
};
#endif // MAINWINDOW_H
