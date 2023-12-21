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
    kPnpInit = 0,
    kPnpWaitMoveToStandby,
    kPnpWaitGrabbingFrame,
    kPnpImageProcessing,
    kPnpWaitMoveToPickPosition,
    kPnpWaitPickObject,
    kPnpWaitMoveToRotatePosition,
    kPnpWaitRotateObject,
    kPnpWaitMoveToPlacePosition,
    kPnpWaitPlaceObject
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

  static QString PnpStateToQString(PnpState state);

private:
  void RobotStartMove(int index);
  void RobotMoveDone(int index);
  void RobotVirtualDOChange(int bit_index, bool state);
  void TimerTimeOut();
  void SetPnpState(PnpState state);
  void MoveToStandby();
  void TriggerGrabFrame();
  void MovePickAndPlace();
  void TriggerFeeeder();
  void TriggerPlateScatt();

signals:
  void PnpSignal_StateChanged(QString state_content);
  void PnpSignal_HasNewMessage(QString content);
  void PnpSignal_GrabFrameTriggered();
  void PnpSignal_DisplayMatchingImage(cv::Mat frame);

public:
  DescartesPoint position_standby_;
  DescartesPoint position_rotate_;
  int rgi_address = 1;

private:
  HansClient *robot_;
  GeoMatch *image_matcher_;
  CoordinateCvt *coor_converter_;
  FlexibleFeed *flex_plate_;
  DHController *dh_controller_;

  PnpState pnp_state_current_;
  PnpState pnp_state_previous_;
  bool is_found_2_objects_;
  bool is_less_than_limit_;
  Size frame_size_;

  const double veloc_fast_ = 500.0;
  const double veloc_low_ = 300.0;
  const double accel_fast_ = 2500.0;
  const double accel_low_ = 1000.0;

  QTimer *pnp_timer;
  int waitFeeding;
  int waitScatt;
  int waitUpper;
  int waitPlateStable;
  int wait_hans_gripper_time_ = 100;

  int rotating_zero_ = 0;
  int rotating_positive_ = 180;
  int rotating_negative_ = -180;
  int grip_open_ = 1000;
  int grip_close_ = 0;

  int feeder_port_ = 7;
  int virtual_rotate_zero_ = 0;
  int virtual_rotate_positive_ = 1;
  int virtual_rotate_negative_ = 2;
  int virtual_grip_open_ = 3;
  int virtual_grip_close_ = 4;
};

#endif // PNPCONTROLLER_H
