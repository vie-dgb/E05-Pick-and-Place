#include "PnpController.h"

PnpController::PnpController(HansClient* const& robot, GeoMatch* const& matcher, CoordinateCvt* const& coordinate_converter, FlexibleFeed* const& flex_plate) {
  robot_ = robot;
  matcher_ = matcher;
  coor_converter_ = coordinate_converter;
  flex_plate_ = flex_plate;

  connect(robot_, &HansClient::RbSignal_StartMove,
          this, &PnpController::RobotStartMove);
  connect(robot_, &HansClient::RbSignal_MoveDone,
          this, &PnpController::RobotMoveDone);
}

PnpController::~PnpController() {}

void PnpController::PnpControllerInit() {
  position_standby_.X = -250.0;
  position_standby_.Y = 255.0;
  position_standby_.Z = 145.0;
  position_standby_.rX = 180.0;
  position_standby_.rY = 0.0;
  position_standby_.rZ = 0.0;
  position_standby_.plane = "Plane_pick";
  position_standby_.tcp = "TCP_dh_gripper";
}

void PnpController::PnpControllerStart() {
  emit PnpSignal_HasNewMessage("[PNP Controller]: Start process.");
  MoveToStandby();
}

void PnpController::PnpControllerStop() {
  robot_->RobotStopImmediate();
}

void PnpController::RobotStartMove() {
  switch (pnp_state_current_) {
    case PnpState::kPnpWaitMoveToStandby:
      emit PnpSignal_HasNewMessage("[PNP Controller]: Moving to standby position.");
      break;
    case PnpState::kPnpWaitMoveOutWorkSpace:
      emit PnpSignal_HasNewMessage("[PNP Controller]: Moving to pick objects.");
      break;
  }
}

void PnpController::RobotMoveDone() {
  switch (pnp_state_current_) {
    case PnpState::kPnpWaitMoveToStandby:
      emit PnpSignal_HasNewMessage("[PNP Controller]: Robot in standby position.");
      TriggerGrabFrame();
      break;
    case PnpState::kPnpWaitPickingObjects:
      emit PnpSignal_HasNewMessage("[PNP Controller]: Robot picked objects.");
      SetPnpState(PnpState::kPnpWaitMoveOutWorkSpace);
      break;
    case PnpState::kPnpWaitMoveOutWorkSpace:
      emit PnpSignal_HasNewMessage("[PNP Controller]: Robot moving to place position.");
      SetPnpState(PnpState::kPnpWaitPlacingObjects);
      break;
    case PnpState::kPnpWaitPlacingObjects:
      emit PnpSignal_HasNewMessage("[PNP Controller]: Robot placed objects.");
      TriggerGrabFrame();
      break;
  }
}

void PnpController::SetPnpState(PnpState state) {
  pnp_state_previous_ = pnp_state_current_;
  pnp_state_current_ = state;
  emit PnpSignal_StateChanged(PnpStateToQString(state));
}

void PnpController::MoveToStandby() {
  SetPnpState(PnpState::kPnpWaitMoveToStandby);
  robot_->pushCommand(HansCommand::SetOverride(0, 100));
  position_standby_.Y += 1;
  robot_->pushCommand(HansCommand::WayPointL(0,
                                             position_standby_,
                                             veloc_low_,
                                             accel_low_, 0));
  position_standby_.Y -= 1;
  robot_->pushCommand(HansCommand::WayPointL(0,
                                             position_standby_,
                                             veloc_low_,
                                             accel_low_, 0));
  robot_->pushCommand(HansCommand::WaitStartMove());
  robot_->pushCommand(HansCommand::WaitMoveDone());
  robot_->DHGripper_Open();
  robot_->pushCommand(HansCommand::WaitTime(2000));
//  robot_->pushCommand(HansCommand::WaitDhGripperArrived());
}

void PnpController::TriggerGrabFrame() {
  SetPnpState(PnpState::kPnpWaitGrabbingFrame);
  emit PnpSignal_GrabFrameTriggered();
  emit PnpSignal_HasNewMessage("[PNP Controller]: Camera grab frame triggered.");
}

void PnpController::ReceivedNewFrame(cv::Mat frame) {
  SetPnpState(PnpState::kPnpImageProcessing);
  emit PnpSignal_HasNewMessage("[PNP Controller]: Received frame, finding match objects.");
  matcher_->matching(frame, true, 2);
  frame_size_.width = frame.cols;
  frame_size_.height = frame.rows;
  emit PnpSignal_HasNewMessage("[PNP Controller]: Objects found.");
  emit PnpSignal_DisplayMatchingImage(matcher_->matchingResult.Image);

  is_found_2_objects_ = (matcher_->matchingResult.Objects.size() >= 2) ? true : false;
  is_less_than_limit_ = matcher_->matchingResult.isAreaLessThanLimits;

  if(matcher_->matchingResult.isFoundMatchObject) {
    MovePickAndPlace();
  }
//  else {
//    // in case not found any matching objects
//    if(isLessThanLimits) {
//      TriggerFeeeder();
//    }
//    else {
//      TriggerPlateScatt();
//    }
//  }
}

void PnpController::MovePickAndPlace() {
  SetPnpState(PnpState::kPnpWaitPickingObjects);
  emit PnpSignal_HasNewMessage("[PNP Controller]: Sending command to robot.");

  DescartesPoint pick_point;
  DescartesPoint place_point;

  MatchedObjects matchObj = matcher_->matchingResult.Objects.front();
  coor_converter_->xMaxPickDistance = 321.0;
  coor_converter_->yMaxPickDistance = 196.0;
  coor_converter_->convertPickCoordinates(pick_point, matchObj.coordinates, matchObj.angle, frame_size_);

  pick_point.rX = 180.0;
  pick_point.rY = 0.0;
  pick_point.Z = 0.0;
  pick_point.plane = "Plane_pick";
  pick_point.tcp = "TCP_dh_gripper";

  place_point.X = 0.0;
  place_point.Y = 0.0;
  place_point.Z = 1.0;
  place_point.rX = 180.0;
  place_point.rY = 0.0;
  place_point.rZ = 0.0;
  place_point.plane = "Plane_place";
  place_point.tcp = "TCP_dh_gripper";

  // push command to queue
  robot_->pushCommand(HansCommand::SetOverride(0, 100));
  robot_->pushCommand(HansCommand::WayPointLRelRef(0, pick_point,
                                                   0, 0, 120, 0,
                                                   veloc_fast_, accel_fast_, 10));
  robot_->pushCommand(HansCommand::WayPointL(0, pick_point, veloc_low_,
                                             accel_low_, 10));
  robot_->pushCommand(HansCommand::WaitStartMove());
  robot_->pushCommand(HansCommand::WaitMoveDone());
  robot_->DHGripper_Close();
  robot_->pushCommand(HansCommand::WaitTime(100));
//  robot_->pushCommand(HansCommand::WaitDhGripperHolding());
  robot_->pushCommand(HansCommand::WayPointLRelRef(0, pick_point,
                                                   0, 0, 120, 0,
                                                   veloc_fast_, accel_fast_, 10));

  robot_->pushCommand(HansCommand::WayPointLRelRef(0, place_point,
                                                   0, 0, 250, 0,
                                                   veloc_fast_, accel_fast_, 10));
  robot_->pushCommand(HansCommand::WayPointL(0, place_point, veloc_low_,
                                             accel_low_, 10));
  robot_->pushCommand(HansCommand::WaitStartMove());
  robot_->pushCommand(HansCommand::WaitMoveDone());
  robot_->DHGripper_Open();
  robot_->pushCommand(HansCommand::WaitTime(100));
//  robot_->pushCommand(HansCommand::WaitDhGripperArrived());
  robot_->pushCommand(HansCommand::WayPointLRelRef(0, place_point,
                                                   0, 0, 250, 0,
                                                   veloc_fast_, accel_fast_, 10));
  robot_->pushCommand(HansCommand::WaitStartMove());
  robot_->pushCommand(HansCommand::WaitMoveDone());
}

QString PnpController::PnpStateToQString(PnpState state) {
  switch (state) {
    case PnpState::kPnpInit:
      return "Initialize";
    case PnpState::kPnpWaitMoveToStandby:
      return "Moving to standby position";
    case PnpState::kPnpWaitGrabbingFrame:
      return "Grabbing image frame";
    case PnpState::kPnpImageProcessing:
      return "Image processing";
    case PnpState::kPnpWaitPickingObjects:
      return "Picking objects";
    case PnpState::kPnpWaitMoveOutWorkSpace:
      return "Moving out camera workspace";
    case PnpState::kPnpWaitPlacingObjects:
      return "Placing objects";
    case PnpState::kPnpWaitFeed:
      return "Feeding";
    case PnpState::kPnpWaitPlateScatt:
      return "Flexible plate scatting";
  }
  return "";
}
