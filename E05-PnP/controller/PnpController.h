#ifndef PNPCONTROLLER_H
#define PNPCONTROLLER_H

#include <QObject>
#include <QDebug>
#include <QTimer>

#include "camera/PylonGrab.h"
#include "camera/GeoMatch.h"
#include "robot/HansClient.h"
#include "robot/CoordinateCvt.h"
#include "camera/GeoMatch.h"
#include "plate/FlexibleFeed.h"
#include "dh-robotic/dhcontroller.h"
#include "mc_protocol/fxremote.h"


using namespace rb;
using namespace ImageMatch;
using namespace dhr;
using namespace fx;

class PnpController : public QObject
{
  Q_OBJECT
public:
  enum PnpState : int {
    kPnpStandby = 0,          // stand by state
    kPnpInit,                 // end when all condition true
    kPnpWaitMoveToStandby,    // end by pgc grip state changed == arrived
    kPnpWaitGrabbingFrame,    // end by call ReceivedNewFrame
    kPnpImageProcessing,      // end when image processing done
    kPnpWaitRobotMove,        // end when robot move to end position
    kPnpWaitFeed,
    kPnpWaitScatt,
    kPnpWaitUpper,
    kPnpWaitStable
  };

  enum PnpMoveState : int {
    kPnpMoveIdle = 1000,       // initial state, that mean robot not moving
    kPnpMoveToPickPosition,   // trigger when robot move done
    kPnpMovePicking,          // trigger when pgc grip state changed == clamping
    kPnpMoveToRotatePosition, // trigger when rgi grip state changed == clamping
    kPnpMoveRotateRelease,    // trigger when pgc grip state changed == arrived
    kPnpMoveRotateObject,     // trigger when rgi move state changed == angle reached
    kPnpMovePickAfterRotate,  // trigger when pgc grip state changed == clamping
    kPnpMoveRotateRelease2,   // trigger when rgi grip state changed == arrived
    kPnpMoveWaitPlcConfirm,   // enter this state when robot wait confirm from plc
    kPnpMoveToPlacePosition,  // trigger when move done
    kPnpMovePlacing,          // trigger when pgc grip state changed == arrived
    kPnpMoveToEndPosition,    // trigger when move done, trigger MOVE PICK AND PLACE DONE
  };

  enum PnpDevicePin : int {
    kPnpPin_RgiRotate_Zero,
    kPnpPin_RgiRotate_Angle,
    kPnpPin_RgiRotate_Negative,
    kPnpPin_RgiGrip_Open,
    kPnpPin_RgiGrip_Close,
    kPnpPin_PgcGrip_Open,
    kPnpPin_PgcGrip_Close,
    kPnpPin_PlcMoveConfirm
  };

  PnpController(HansClient* const& robot = nullptr,
                Vision::PylonGrab *camera = nullptr,
                GeoMatch* const& matcher = nullptr,
                CoordinateCvt* const& coordinate_converter = nullptr,
                FlexibleFeed* const& flex_plate = nullptr,
                DHController* const& dh_controller = nullptr,
                FxRemote* const& fx_plc = nullptr);
  ~PnpController();

  void PnpControllerStart();
  void PnpControllerStop();
  bool PnpControllerIsRunning();
  bool PnpCheckControllerReady(QStringList &check_list);
  void ReceivedNewFrame(cv::Mat frame);

  static QString StateToQString(PnpState state);
  static QString StateToQString(PnpMoveState state);

private:
  void SetPnpState(PnpState state);
  void SetPnpMoveState(PnpMoveState state);
  void InitProcess();
  void ErrorHandle();
  void RobotStartMove(int index);
  void RobotMoveDone(int index);
  void RobotOuputIntTriggered(int value);
  void RgiGripperStateChanged(DhGripperStatus state);
  void RgiRotateStateChanged(DhRotationStatus state);
  void PgcGripperStateChanged(DhGripperStatus state);
  void PlcMDeviceChanged(int number, bool value);
  void TimerTimeOut();
  void MoveToStandby();
  void TriggerGrabFrame();
  void MovePickAndPlace();
  void ReCheckMatchObjects();

  // Sub device control
  void TriggerFeeeder();
  void TriggerPlateScatt();
  void TriggerPlateUpper();
  void TriggerPlateWaitStable();
  void FeederEnable(bool state);

signals:
  void PnpSignal_StateChanged(PnpState state);
  void PnpSignal_MoveStateChanged(PnpMoveState state);
  void PnpSignal_HasNewMessage(QString content);
  void PnpSignal_GrabFrameTriggered();
  void PnpSignal_DisplayMatchingImage(cv::Mat frame);
  void PnpSignal_UpdateCheckList(bool is_ready, QStringList check_list);
  void PnpSignal_ErrorOccurred(QString msg);
  void PnpSignal_CompletePlaceObject(int counter);

public:
  DescartesPoint position_standby_;
  DescartesPoint position_rotate_;
  int pgc_address_ = 1;
  int rgi_address_ = 2;

  const double veloc_fast_ = 2500.0;
  const double veloc_low_ = 2000.0;
  const double accel_fast_ = 2500.0;
  const double accel_low_ = 2000.0;

  const int rgi_rotating_zero_ = 0;
  const int rgi_rotating_positive_ = 90;
  const int rgi_rotating_negative_ = -90;
  const int rgi_grip_open_pos_ = 1000;
  const int rgi_grip_close_pos_ = 5;
  const int pgc_grip_open_pos_ = 250;
  const int pgc_grip_close_pos_ = 0;

  const int wait_feeding_time_ = 1500;
  const int wait_plate_scatt_time_ = 3000;
  const int wait_plate_upper_time_ = 3000;
  const int wait_plate_stable_time_ = 1000;

private:
  HansClient *robot_ = nullptr;
  Vision::PylonGrab *camera_ = nullptr;
  GeoMatch *image_matcher_ = nullptr;
  CoordinateCvt *coor_converter_ = nullptr;
  FlexibleFeed *flex_plate_ = nullptr;
  DHController *dh_controller_ = nullptr;
  FxRemote *fx_plc_ = nullptr;

  PnpState pnp_state_current_;
  PnpState pnp_state_previous_;
  PnpMoveState move_state_current_;
  PnpMoveState move_state_previous_;
  bool is_found_2_objects_;
  bool is_less_than_limit_;
  Size frame_size_;

  QTimer *pnp_timer;

  int object_rotate_angle_;

  bool is_possible_pick_ = false;

  int sum_picked_objects_ = 0;
  int sum_picked_col_ = 0;
  int sum_picked_row_ = 0;
  const double col_distance_ = 35.0;
  const double row_distance_ = 74.0;

  int feeder_port_ = 7;
  int bit_rgi_rotate_zero_ = 0;
  int bit_rgi_rotate_positive_ = 1;
  int bit_rgi_rotate_negative_ = 2;
  int bit_rgi_grip_open_ = 3;
  int bit_rgi_grip_close_ = 4;
  int bit_pgc_grip_open_ = 5;
  int bit_pgc_grip_close_ = 6;

  int m_device_pnp_start_ = 0;
  int m_device_pnp_stop_ = 1;
  int m_device_pnp_error_ = 2;
  int m_device_place_confirm_ = 3;
  int m_device_place_complete_ = 4;
  int m_device_pnp_running_ = 5;
  int d_device_sample_counter_ = 0;
};

#endif // PNPCONTROLLER_H
