#ifndef PNPCONTROLLER_H
#define PNPCONTROLLER_H

#include <QObject>
#include <QDebug>

#include "robot/HansClient.h"
#include "robot/CoordinateCvt.h"
#include "camera/GeoMatch.h"
#include "plate/FlexibleFeed.h"

using namespace rb;
using namespace ImageMatch;

class PnpController : public QObject
{
  Q_OBJECT
public:
  enum PnpState : int {
    kPnpInit = 0,
    kPnpWaitMoveToStandby,
    kPnpWaitGrabbingFrame,
    kPnpImageProcessing,
    kPnpWaitPickingObjects,
    kPnpWaitMoveOutWorkSpace,
    kPnpWaitPlacingObjects,
    kPnpWaitFeed,
    kPnpWaitPlateScatt
  };

  PnpController(HansClient* const& robot = nullptr,
                GeoMatch* const& matcher = nullptr,
                CoordinateCvt* const& coordinate_converter = nullptr,
                FlexibleFeed* const& flex_plate = nullptr);
  ~PnpController();

  void PnpControllerInit();
  void PnpControllerStart();
  void PnpControllerStop();

  void ReceivedNewFrame(cv::Mat frame);

  static QString PnpStateToQString(PnpState state);

private:
  void RobotStartMove();
  void RobotMoveDone();
  void SetPnpState(PnpState state);
  void MoveToStandby();
  void TriggerGrabFrame();
  void MovePickAndPlace();

signals:
  void PnpSignal_StateChanged(QString state_content);
  void PnpSignal_HasNewMessage(QString content);
  void PnpSignal_GrabFrameTriggered();
  void PnpSignal_DisplayMatchingImage(cv::Mat frame);

public:
  DescartesPoint position_standby_;

private:
  HansClient *robot_;
  GeoMatch *matcher_;
  CoordinateCvt *coor_converter_;
  FlexibleFeed *flex_plate_;

  PnpState pnp_state_current_;
  PnpState pnp_state_previous_;
  bool is_found_2_objects_;
  bool is_less_than_limit_;
  Size frame_size_;

  const double veloc_fast_ = 300.0;
  const double veloc_low_ = 100.0;
  const double accel_fast_ = 2500.0;
  const double accel_low_ = 500.0;
};

#endif // PNPCONTROLLER_H
