#ifndef PNPCONTROLLER_H
#define PNPCONTROLLER_H

#include <QObject>
#include <QDebug>
#include <QTimer>

#include "robot/HansClient.h"
#include "robot/CoordinateCvt.h"
#include "camera/GeoMatch.h"
#include "plate/FlexibleFeed.h"
#include "dh-robotic/dhcontroller.h"

using namespace rb;
using namespace ImageMatch;
using namespace dhr;

class PnpController : public QObject
{
  Q_OBJECT
public:
  enum PnpState : int {
    kPnpInit = 0,             // end when all condition true
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
    kPnpMoveIdle = 100,       // initial state, that mean robot not moving
    kPnpMoveToPickPosition,   // end by move done
    kPnpMovePicking,          // end by pgc grip state changed == clamping
    kPnpMoveToRotatePosition, // end by rgi grip state changed == clamping
    kPnpMoveRotateRelease,    // end by pgc grip state changed == arrived
    kPnpMoveRotateObject,     // end by rgi move state changed == angle reached
    kPnpMovePickAfterRotate,  // end by pgc grip state changed == clamping
    kPnpMoveRotateRelease2,   // end by rgi grip state changed == arrived
    kPnpMoveToPlacePosition,  // end by move done
    kPnpMovePlacing,          // end by pgc grip state changed == arrived
    kPnpMoveToEndPosition,    // end by move done, trigger MOVE PICK AND PLACE DONE
  };

  enum PnpDevicePin : int {
    kPnpPin_RgiRotate_Zero,
    kPnpPin_RgiRotate_Positive,
    kPnpPin_RgiRotate_Negative,
    kPnpPin_RgiGrip_Open,
    kPnpPin_RgiGrip_Close,
    kPnpPin_PgcGrip_Open,
    kPnpPin_PgcGrip_Close,
  };

  PnpController(HansClient* const& robot = nullptr,
                GeoMatch* const& matcher = nullptr,
                CoordinateCvt* const& coordinate_converter = nullptr,
                FlexibleFeed* const& flex_plate = nullptr,
                DHController* const& dh_controller = nullptr);
  ~PnpController();

  void PnpControllerInit();
  void PnpControllerStart();
  void PnpControllerStop();
  void ReceivedNewFrame(cv::Mat frame);

  static QString StateToQString(PnpState state);
  static QString StateToQString(PnpMoveState state);

private:
  void SetPnpState(PnpState state);
  void SetPnpMoveState(PnpMoveState state);

  void RobotStartMove(int index);
  void RobotMoveDone(int index);
  void RobotOuputIntTriggered(int value);
  void RgiGripperStateChanged(DhGripperStatus state);
  void RgiRotateStateChanged(DhRotationStatus state);
  void PgcGripperStateChanged(DhGripperStatus state);
  void TimerTimeOut();
  void MoveToStandby();
  void TriggerGrabFrame();
  void MovePickAndPlace();

  // Sub device control
  void TriggerFeeeder();
  void TriggerPlateScatt();
  void FeederEnable(bool state);

signals:
  void PnpSignal_StateChanged(PnpState state);
  void PnpSignal_MoveStateChanged(PnpMoveState state);
  void PnpSignal_HasNewMessage(QString content);
  void PnpSignal_GrabFrameTriggered();
  void PnpSignal_DisplayMatchingImage(cv::Mat frame);

public:
  DescartesPoint position_standby_;
  DescartesPoint position_rotate_;
  int pgc_address = 1;
  int rgi_address = 2;

private:
  HansClient *robot_;
  GeoMatch *image_matcher_;
  CoordinateCvt *coor_converter_;
  FlexibleFeed *flex_plate_;
  DHController *dh_controller_;

  PnpState pnp_state_current_;
  PnpState pnp_state_previous_;
  PnpMoveState move_state_current_;
  PnpMoveState move_state_previous_;
  bool is_found_2_objects_;
  bool is_less_than_limit_;
  Size frame_size_;

//  const double veloc_fast_ = 500.0;
//  const double veloc_low_ = 300.0;
  const double veloc_fast_ = 2500.0;
  const double veloc_low_ = 2000.0;
  const double accel_fast_ = 2500.0;
  const double accel_low_ = 2000.0;

  QTimer *pnp_timer;
  int waitFeeding;
  int waitScatt;
  int waitUpper;
  int waitPlateStable;
  int wait_hans_gripper_time_ = 100;

  int object_rotate_angle_;

  int rgi_rotating_zero_ = 0;
  int rgi_rotating_positive_ = 90;
  int rgi_rotating_negative_ = -90;
  int rgi_grip_open_pos_ = 1000;
  int rgi_grip_close_pos_ = 0;
  int pgc_grip_open_pos_ = 250;
  int pgc_grip_close_pos_ = 0;

  int feeder_port_ = 7;
  int bit_rgi_rotate_zero_ = 0;
  int bit_rgi_rotate_positive_ = 1;
  int bit_rgi_rotate_negative_ = 2;
  int bit_rgi_grip_open_ = 3;
  int bit_rgi_grip_close_ = 4;
  int bit_pgc_grip_open_ = 5;
  int bit_pgc_grip_close_ = 6;
};

#endif // PNPCONTROLLER_H
