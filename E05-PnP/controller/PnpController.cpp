#include "PnpController.h"

PnpController::PnpController(HansClient* const& robot,
                             GeoMatch* const& matcher,
                             CoordinateCvt* const& coordinate_converter,
                             FlexibleFeed* const& flex_plate,
                             DHController* const& dh_controller) {
  robot_ = robot;
  image_matcher_ = matcher;
  coor_converter_ = coordinate_converter;
  flex_plate_ = flex_plate;
  dh_controller_ = dh_controller;
  pnp_timer = new QTimer;

  connect(robot_, &HansClient::RbSignal_StartMove,
          this, &PnpController::RobotStartMove);
  connect(robot_, &HansClient::RbSignal_MoveDone,
          this, &PnpController::RobotMoveDone);
  connect(pnp_timer, &QTimer::timeout, this, &PnpController::TimerTimeOut);
  connect(robot_, &HansClient::RbSignal_VirtualDOChange,
          this, &PnpController::RobotVirtualDOChange);
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

  position_rotate_.X = -197.451;
  position_rotate_.Y = 555.911;
  position_rotate_.Z = 38.297;
  position_rotate_.rX = 180.0;
  position_rotate_.rY = 0.0;
  position_rotate_.rZ = 0.0;
  position_rotate_.plane = "Plane_pick";
  position_rotate_.tcp = "TCP_dh_gripper";
}

void PnpController::PnpControllerStart() {
  emit PnpSignal_HasNewMessage("[PNP Controller]: Start process.");
  MoveToStandby();
}

void PnpController::PnpControllerStop() {
  robot_->RobotStopImmediate();
}

void PnpController::RobotStartMove(int index) {
  PnpState state = static_cast<PnpState>(index);

  switch (state) {
    case PnpState::kPnpWaitMoveToStandby:
      emit PnpSignal_HasNewMessage("[PNP Controller]: Moving to standby position.");
      break;
    case PnpState::kPnpWaitMoveToPickPosition:
      emit PnpSignal_HasNewMessage("[PNP Controller]: Moving to pick objects.");
      break;
    case PnpState::kPnpWaitPickObject:
      emit PnpSignal_HasNewMessage("[PNP Controller]: Robot picking object.");
      break;
    case PnpState::kPnpWaitMoveToRotatePosition:
      emit PnpSignal_HasNewMessage("[PNP Controller]: Moving to rotate position.");
      break;
    case PnpState::kPnpWaitRotateObject:
      emit PnpSignal_HasNewMessage("[PNP Controller]: Rotating object.");
      break;
    case PnpState::kPnpWaitMoveToPlacePosition:
      emit PnpSignal_HasNewMessage("[PNP Controller]: Moving to place position.");
      break;
    case PnpState::kPnpWaitPlaceObject:
      emit PnpSignal_HasNewMessage("[PNP Controller]: Placing object.");
      break;
  }
}

void PnpController::RobotMoveDone(int index) {
  PnpState state = static_cast<PnpState>(index);
  switch (state) {
    case PnpState::kPnpWaitMoveToStandby:
      emit PnpSignal_HasNewMessage("[PNP Controller]: Robot in standby position.");
      TriggerGrabFrame();
      break;
    case PnpState::kPnpWaitMoveToPickPosition:
      emit PnpSignal_HasNewMessage("[PNP Controller]: Robot in pick position.");
      SetPnpState(PnpState::kPnpWaitPickObject);
      break;
    case PnpState::kPnpWaitPickObject:
      emit PnpSignal_HasNewMessage("[PNP Controller]: Robot picking object done.");
      SetPnpState(PnpState::kPnpWaitMoveToRotatePosition);
      break;
    case PnpState::kPnpWaitMoveToRotatePosition:
      emit PnpSignal_HasNewMessage("[PNP Controller]: Object in rotate position.");
      SetPnpState(PnpState::kPnpWaitRotateObject);
      break;
    case PnpState::kPnpWaitRotateObject:
      emit PnpSignal_HasNewMessage("[PNP Controller]: Rotating object.");
      SetPnpState(PnpState::kPnpWaitMoveToPlacePosition);
      break;
    case PnpState::kPnpWaitMoveToPlacePosition:
      emit PnpSignal_HasNewMessage("[PNP Controller]: Robot in place position.");
      SetPnpState(PnpState::kPnpWaitPlaceObject);
      break;
    case PnpState::kPnpWaitPlaceObject:
      emit PnpSignal_HasNewMessage("[PNP Controller]: Object placed.");
      TriggerGrabFrame();
      break;
  }
}

void PnpController::RobotVirtualDOChange(int bit_index, bool state) {
  if ((bit_index == virtual_rotate_zero_) && state) {
    dh_controller_->DH_AddFuncToQueue(
        DH_RGI::SetRotationAngle(rgi_address, rotating_zero_));

  } else if ((bit_index == virtual_rotate_positive_) && state) {
    dh_controller_->DH_AddFuncToQueue(
        DH_RGI::SetRotationAngle(rgi_address, rotating_positive_));

  } else if ((bit_index == virtual_rotate_negative_) && state) {
    dh_controller_->DH_AddFuncToQueue(
        DH_RGI::SetRotationAngle(rgi_address, rotating_positive_));

  } else if ((bit_index == virtual_grip_open_) && state) {
    dh_controller_->DH_AddFuncToQueue(
        DH_RGI::SetGripperPosition(rgi_address, grip_open_));

  } else if ((bit_index == virtual_grip_close_) && state) {
    dh_controller_->DH_AddFuncToQueue(
        DH_RGI::SetGripperPosition(rgi_address, grip_close_));
  }
}

void PnpController::TimerTimeOut() {
//  switch (pnp_state_current_) {
//    case PnpState::kPnpWaitFeed:
//      robot_->pushCommandInFront(HansCommand::SetBoxDO(feeder_port_, false));
//      emit PnpSignal_HasNewMessage("[PNP Controller]: Feeder disable.");
//      break;
//  }
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
  robot_->pushCommand(HansCommand::WaitStartMove(static_cast<int>(pnp_state_current_)));
  robot_->pushCommand(HansCommand::WaitMoveDone(static_cast<int>(pnp_state_current_)));
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
  image_matcher_->matching(frame, true, 2);
  frame_size_.width = frame.cols;
  frame_size_.height = frame.rows;
  emit PnpSignal_HasNewMessage("[PNP Controller]: Objects found.");
  emit PnpSignal_DisplayMatchingImage(image_matcher_->matchingResult.Image);

  is_found_2_objects_ = (image_matcher_->matchingResult.Objects.size() >= 2) ? true : false;
  is_less_than_limit_ = image_matcher_->matchingResult.isAreaLessThanLimits;

  if(image_matcher_->matchingResult.isFoundMatchObject) {
    MovePickAndPlace();
  }
//  else {
//    // in case not found any matching objects
//    if(is_less_than_limit_) {
//      TriggerFeeeder();
//    }
//    else {
//      TriggerPlateScatt();
//    }
//  }
}

void PnpController::MovePickAndPlace() {
  SetPnpState(PnpState::kPnpWaitMoveToPickPosition);
  emit PnpSignal_HasNewMessage("[PNP Controller]: Sending command to robot.");

  DescartesPoint pick_point;
  DescartesPoint place_point;

  MatchedObjects matchObj = image_matcher_->matchingResult.Objects.front();
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
  place_point.Z = 5.0;
  place_point.rX = 180.0;
  place_point.rY = 0.0;
  place_point.rZ = 0.0;
  place_point.plane = "Plane_place";
  place_point.tcp = "TCP_dh_gripper";

  // push command to queue
  robot_->pushCommand(HansCommand::SetOverride(0, 100));
  // move to pick position
  robot_->pushCommand(
      HansCommand::WayPointLRelRef(0, pick_point, 0, 0, 120, 0,
                                   veloc_fast_, accel_fast_, 10));
  robot_->pushCommand(
      HansCommand::WayPointL(0, pick_point, veloc_low_, accel_low_, 10));
  robot_->pushCommand(
      HansCommand::WaitStartMove(static_cast<int>(kPnpWaitMoveToPickPosition)));
  robot_->pushCommand(
      HansCommand::WaitMoveDone(static_cast<int>(kPnpWaitMoveToPickPosition)));

  // pick object
  robot_->DHGripper_Close();
  robot_->pushCommand(HansCommand::WaitTime(100));
//  robot_->pushCommand(HansCommand::WaitDhGripperHolding());
  robot_->pushCommand(
      HansCommand::WayPointLRelRef(0, pick_point, 0, 0, 120, 0,
                                   veloc_fast_, accel_fast_, 10));

  if (matchObj.indexOfSample == 0) {
    // go to rotate point
    robot_->pushCommand(
        HansCommand::WayPointLRelRef(0, position_rotate_, 0, -50, 50, 0,
                                     veloc_fast_, accel_fast_, 10));
    robot_->pushCommand(
        HansCommand::WayPointLRelRef(0, position_rotate_, 0, -50, 0, 0,
                                     veloc_fast_, veloc_fast_, 10));
    robot_->pushCommand(
        HansCommand::WayPointL(0, position_rotate_, veloc_low_, accel_low_, 10));
    robot_->pushCommand(
        HansCommand::WaitStartMove(static_cast<int>(kPnpWaitMoveToRotatePosition)));
    robot_->pushCommand(
        HansCommand::WaitMoveDone(static_cast<int>(kPnpWaitMoveToRotatePosition)));
    // close rotate grip and move to wait position
    robot_->pushCommand(
        HansCommand::SetVirtualDO(virtual_grip_close_, true));
    robot_->pushCommand(
        HansCommand::SetVirtualDO(virtual_grip_close_, false));
    robot_->pushCommand(
        HansCommand::WaitTime(300));
    robot_->DHGripper_Open();
    robot_->pushCommand(
        HansCommand::WaitTime(wait_hans_gripper_time_));
    robot_->pushCommand(
        HansCommand::WayPointLRelRef(0, position_rotate_, 0, 0, 50, 0,
                                     veloc_fast_, veloc_fast_, 10));
    robot_->pushCommand(
        HansCommand::WaitStartMove(static_cast<int>(kPnpWaitRotateObject)));
    robot_->pushCommand(
        HansCommand::WaitMoveDone(static_cast<int>(kPnpWaitRotateObject)));
    // rotate object
    robot_->pushCommand(
        HansCommand::SetVirtualDO(virtual_rotate_positive_, true));
    robot_->pushCommand(
        HansCommand::SetVirtualDO(virtual_rotate_positive_, false));
    robot_->pushCommand(
        HansCommand::WaitTime(300));
    // get position from rotate moudle
    robot_->pushCommand(
        HansCommand::WayPointL(0, position_rotate_, veloc_low_, accel_low_, 10));
    robot_->pushCommand(
        HansCommand::WaitStartMove(99));
    robot_->pushCommand(
        HansCommand::WaitMoveDone(99));
    robot_->DHGripper_Close();
    robot_->pushCommand(
        HansCommand::WaitTime(wait_hans_gripper_time_));
    robot_->pushCommand(HansCommand::SetVirtualDO(virtual_grip_open_, true));
    robot_->pushCommand(HansCommand::SetVirtualDO(virtual_grip_open_, false));
    robot_->pushCommand(
        HansCommand::WaitTime(300));
    robot_->pushCommand(
        HansCommand::WayPointLRelRef(0, position_rotate_, 0, -50, 0, 0,
                                     veloc_fast_, veloc_fast_, 10));
    robot_->pushCommand(
        HansCommand::WaitStartMove(99));
    robot_->pushCommand(
        HansCommand::WaitMoveDone(99));
    robot_->pushCommand(HansCommand::SetVirtualDO(virtual_rotate_zero_, true));
    robot_->pushCommand(HansCommand::SetVirtualDO(virtual_rotate_zero_, false));
  }

  // go to place position
  robot_->pushCommand(
      HansCommand::WayPointLRelRef(0, place_point, 0, 0, 250, 0,
                                   veloc_fast_, accel_fast_, 10));
  robot_->pushCommand(
      HansCommand::WayPointL(0, place_point, veloc_low_, accel_low_, 10));
  robot_->pushCommand(
      HansCommand::WaitStartMove(static_cast<int>(kPnpWaitMoveToPlacePosition)));
  robot_->pushCommand(
      HansCommand::WaitMoveDone(static_cast<int>(kPnpWaitMoveToPlacePosition)));
  robot_->DHGripper_Open();
  robot_->pushCommand(HansCommand::WaitTime(wait_hans_gripper_time_));
//  robot_->pushCommand(HansCommand::WaitDhGripperArrived());
  robot_->pushCommand(
      HansCommand::WayPointLRelRef(0, place_point, 0, 0, 250, 0,
                                   veloc_fast_, accel_fast_, 10));
  robot_->pushCommand(
      HansCommand::WaitStartMove(static_cast<int>(kPnpWaitPlaceObject)));
  robot_->pushCommand(
      HansCommand::WaitMoveDone(static_cast<int>(kPnpWaitPlaceObject)));

  emit PnpSignal_HasNewMessage("[PNP Controller]: Sending command to robot done.");
}

void PnpController::TriggerFeeeder() {
//  emit PnpSignal_HasNewMessage("[PNP Controller]: Feeder enable.");
//  robot_->pushCommandInFront(HansCommand::SetBoxDO(7, true));
//  pnp_timer->start(waitFeeding);
//  SetPnpState(kPnpWaitFeed);
}

void PnpController::TriggerPlateScatt() {

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
    case PnpState::kPnpWaitMoveToPickPosition:
      return "Moving to pick position";
    case PnpState::kPnpWaitPickObject:
      return "Picking object";
    case PnpState::kPnpWaitMoveToRotatePosition:
      return "Moving to rotate object positon";
    case PnpState::kPnpWaitRotateObject:
      return "Rotating object";
    case PnpState::kPnpWaitMoveToPlacePosition:
      return "Moving to place position";
    case PnpState::kPnpWaitPlaceObject:
      return "Placing object";
  }
  return "";
}

//robot_->pushCommand(HansCommand::SetOverride(0, 100));
//robot_->pushCommand(HansCommand::WayPointLRelRef(0, pick_point,
//                                                 0, 0, 120, 0,
//                                                 veloc_fast_, accel_fast_, 10));
//robot_->pushCommand(HansCommand::WayPointL(0, pick_point, veloc_low_,
//                                           accel_low_, 10));
//robot_->pushCommand(HansCommand::WaitStartMove());
//robot_->pushCommand(HansCommand::WaitMoveDone());
//robot_->DHGripper_Close();
//robot_->pushCommand(HansCommand::WaitTime(100));
////  robot_->pushCommand(HansCommand::WaitDhGripperHolding());
//robot_->pushCommand(HansCommand::WayPointLRelRef(0, pick_point,
//                                                 0, 0, 120, 0,
//                                                 veloc_fast_, accel_fast_, 10));

//robot_->pushCommand(HansCommand::WayPointLRelRef(0, place_point,
//                                                 0, 0, 250, 0,
//                                                 veloc_fast_, accel_fast_, 10));
//robot_->pushCommand(HansCommand::WayPointL(0, place_point, veloc_low_,
//                                           accel_low_, 10));
//robot_->pushCommand(HansCommand::WaitStartMove());
//robot_->pushCommand(HansCommand::WaitMoveDone());
//robot_->DHGripper_Open();
//robot_->pushCommand(HansCommand::WaitTime(100));
////  robot_->pushCommand(HansCommand::WaitDhGripperArrived());
//robot_->pushCommand(HansCommand::WayPointLRelRef(0, place_point,
//                                                 0, 0, 250, 0,
//                                                 veloc_fast_, accel_fast_, 10));
//robot_->pushCommand(HansCommand::WaitStartMove());
//robot_->pushCommand(HansCommand::WaitMoveDone());
