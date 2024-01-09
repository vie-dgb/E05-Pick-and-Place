#include "PnpController.h"

PnpController::PnpController(HansClient* const& robot,
                             GeoMatch* const& matcher,
                             CoordinateCvt* const& coordinate_converter,
                             FlexibleFeed* const& flex_plate,
                             DHController* const& dh_controller,
                             FxRemote* const& fx_plc) {
  robot_ = robot;
  image_matcher_ = matcher;
  coor_converter_ = coordinate_converter;
  flex_plate_ = flex_plate;
  dh_controller_ = dh_controller;
  fx_plc_ = fx_plc;
  pnp_timer = new QTimer;

  connect(pnp_timer, &QTimer::timeout,
          this, &PnpController::TimerTimeOut);
  connect(robot_, &HansClient::RbSignal_StartMove,
          this, &PnpController::RobotStartMove);
  connect(robot_, &HansClient::RbSignal_MoveDone,
          this, &PnpController::RobotMoveDone);
  connect(robot_, &HansClient::RbSignal_TriggeredOutputInt,
          this, &PnpController::RobotOuputIntTriggered);
  connect(dh_controller_, &DHController::DHSignal_RgiGripperStateChanged,
          this, &PnpController::RgiGripperStateChanged);
  connect(dh_controller_, &DHController::DHSignal_RgiRotateStateChanged,
          this, &PnpController::RgiRotateStateChanged);
  connect(dh_controller_, &DHController::DHSignal_PgcGripperStateChanged,
          this, &PnpController::PgcGripperStateChanged);
  connect(fx_plc_, &FxRemote::FxSignal_DeivceMChangedState,
          this, &PnpController::PlcMDeviceChanged);

  pnp_state_current_ = PnpState::kPnpStandby;
  pnp_state_previous_ = PnpState::kPnpStandby;
  move_state_current_ = PnpMoveState::kPnpMoveIdle;
  move_state_previous_ = PnpMoveState::kPnpMoveIdle;
}

PnpController::~PnpController() {}

void PnpController::PnpControllerStart() {

  if (!robot_->robotIsConnected()) {
    return;
  }

  if (!fx_plc_->FxIsConnected()) {
    return;
  }

  if (!flex_plate_->isFeederConnected()) {
    return;
  }

  if (!dh_controller_->DH_IsConnected()) {
    return;
  }

  if (pnp_state_current_ != PnpState::kPnpStandby) {
    emit PnpSignal_HasNewMessage("[PNP Controller]: Process is running.");
    return;
  }
  emit PnpSignal_HasNewMessage("[PNP Controller]: Start process.");
  InitProcess();
}

void PnpController::PnpControllerStop() {
  if (pnp_state_current_ = PnpState::kPnpStandby) {
    return;
  }

  robot_->RobotStopImmediate();
  if (pnp_timer->isActive()) {
    pnp_timer->stop();
  }
  flex_plate_->writeCommunicateMode(FlexibleFeed::CommunicationMode::Stop);
  SetPnpState(PnpState::kPnpStandby);
  SetPnpMoveState(PnpMoveState::kPnpMoveIdle);
  fx_plc_->FxWriteBit(FxPlc::kPLCDevice_M, m_device_pnp_running_, true);
  emit PnpSignal_HasNewMessage("[PNP Controller]: Process is stopped.");
}

bool PnpController::PnpControllerIsRunning() {
  return (pnp_state_current_ == PnpState::kPnpStandby) ? true : false;
}

void PnpController::ReceivedNewFrame(cv::Mat frame) {
  SetPnpState(PnpState::kPnpImageProcessing);
  emit PnpSignal_HasNewMessage("[PNP Controller]: Received frame, finding match objects.");
  image_matcher_->matching(frame, true, 2);
  frame_size_.width = frame.cols;
  frame_size_.height = frame.rows;
  emit PnpSignal_DisplayMatchingImage(image_matcher_->matchingResult.Image);

  is_found_2_objects_ =
      (image_matcher_->matchingResult.Objects.size() >= 2) ? true : false;
  is_less_than_limit_ = image_matcher_->matchingResult.isAreaLessThanLimits;

  if(image_matcher_->matchingResult.isFoundMatchObject) {
    emit PnpSignal_HasNewMessage("[PNP Controller]: Objects found.");
    is_possible_pick_ = true;
    if (move_state_current_ == PnpMoveState::kPnpMoveIdle) {
      MovePickAndPlace();
    } else {
      SetPnpState(PnpState::kPnpWaitRobotMove);
    }
  }
  else {
    // in case not found any matching objects
    if(is_less_than_limit_) {
      emit PnpSignal_HasNewMessage("[PNP Controller]: Objects in plate less then limits.");
      TriggerFeeeder();
    }
    else {
      emit PnpSignal_HasNewMessage("[PNP Controller]: Not found objects.");
      TriggerPlateScatt();
    }
  }
}

QString PnpController::StateToQString(PnpState state) {
  switch (state) {
    case PnpState::kPnpStandby:
      return "Standby";
    case PnpState::kPnpInit:
      return "Initialize";
    case PnpState::kPnpWaitMoveToStandby:
      return "Robot move to standby position";
    case PnpState::kPnpWaitGrabbingFrame:
      return "Grabbing image frame";
    case PnpState::kPnpImageProcessing:
      return "Find matching object";
    case PnpState::kPnpWaitRobotMove:
      return "Robot moving pick and place";
    case PnpState::kPnpWaitFeed:
      return "Feeding";
    case PnpState::kPnpWaitScatt:
      return "Flexble plate vibrating scatt";
    case PnpState::kPnpWaitUpper:
      return "Flexble plate vibrating upper";
    case PnpState::kPnpWaitStable:
      return "Wait object stop vibrating";
  }
  return "Illegal state";
}

QString PnpController::StateToQString(PnpMoveState state) {
  switch (state) {
    case PnpMoveState::kPnpMoveIdle:
      return "Robot not move";
    case PnpMoveState::kPnpMoveToPickPosition:
      return "Robot moving to pick position";
    case PnpMoveState::kPnpMovePicking:
      return "Robot picking object";
    case PnpMoveState::kPnpMoveToRotatePosition:
      return "Robot moving to rotate object position";
    case PnpMoveState::kPnpMoveRotateRelease:
      return "Release object for rotate";
    case PnpMoveState::kPnpMoveRotateObject:
      return "Rotate object";
    case PnpMoveState::kPnpMovePickAfterRotate:
      return "Robot picking object at rotate position";
    case PnpMoveState::kPnpMoveToPlacePosition:
      return "Robot moving to place object positon";
    case PnpMoveState::kPnpMovePlacing:
      return "Robot place object";
    case PnpMoveState::kPnpMoveToEndPosition:
      return "Robot moving to end position";
  }
  return "Illegal state";
}

void PnpController::SetPnpState(PnpState state) {
  pnp_state_previous_ = pnp_state_current_;
  pnp_state_current_ = state;
  emit PnpSignal_StateChanged(pnp_state_current_);
}

void PnpController::SetPnpMoveState(PnpMoveState state){
  move_state_previous_ = move_state_current_;
  move_state_current_ = state;
  emit PnpSignal_MoveStateChanged(move_state_current_);
}

void PnpController::InitProcess() {
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

  SetPnpMoveState(kPnpMoveIdle);
  is_possible_pick_ = false;
  sum_picked_objects_ = 0;
  sum_picked_col_ = 0;
  sum_picked_row_ = 0;
  flex_plate_->writeVibrationMode(FlexibleFeed::VibrationMode::Last);
  // add handle when get start flag from plc fail
//  fx_plc_->GetAuxiliaryRelayState(m_device_place_confirm_, plc_place_confirm_flag_);
  fx_plc_->FxWriteBit(FxPlc::kPLCDevice_M, m_device_pnp_running_, true);
  fx_plc_->FxWriteBit(FxPlc::kPLCDevice_M, m_device_place_complete_, false);
  fx_plc_->FxWriteWord(FxPlc::kPLCDevice_D,
                       d_device_sample_counter_, sum_picked_objects_);
  MoveToStandby();
}

void PnpController::RobotStartMove(int index) {
  PnpState main_state = static_cast<PnpState>(index);
  switch (main_state) {
    case PnpState::kPnpWaitMoveToStandby:
      emit PnpSignal_HasNewMessage("[PNP Controller]: Moving to standby position.");
      return;
  }

  PnpMoveState move_state = static_cast<PnpMoveState>(index);
  switch (move_state) {
    case PnpMoveState::kPnpMoveToPickPosition:
      fx_plc_->FxWriteBit(FxPlc::kPLCDevice_M, m_device_place_complete_, false);
      emit PnpSignal_HasNewMessage("[PNP Controller]: Robot moving to pick position.");
      break;
    case PnpMoveState::kPnpMoveToRotatePosition:
      emit PnpSignal_HasNewMessage("[PNP Controller]: Robot moving to rotate position.");
      break;
    case PnpMoveState::kPnpMoveToPlacePosition:
      emit PnpSignal_HasNewMessage("[PNP Controller]: Robot moving to place position.");
      break;
    case PnpMoveState::kPnpMoveToEndPosition:
      emit PnpSignal_HasNewMessage("[PNP Controller]: Robot moving to end position.");
      if (is_possible_pick_) {
        MovePickAndPlace();
      }
      break;
  }
}

void PnpController::RobotMoveDone(int index) {
  PnpState main_state = static_cast<PnpState>(index);
  switch (main_state) {
    case PnpState::kPnpWaitMoveToStandby:
      emit PnpSignal_HasNewMessage("[PNP Controller]: Robot in standby position.");
      return;
  }

  PnpMoveState move_state = static_cast<PnpMoveState>(index);
  switch (move_state) {
    case PnpMoveState::kPnpMoveToPickPosition:
      emit PnpSignal_HasNewMessage("[PNP Controller]: Robot in pick position.");
      SetPnpMoveState(kPnpMovePicking);
      break;
    case PnpMoveState::kPnpMoveToRotatePosition:
      emit PnpSignal_HasNewMessage("[PNP Controller]: Robot in rotate position.");
      ReCheckMatchObjects();
      break;
    case PnpMoveState::kPnpMoveToPlacePosition:
      emit PnpSignal_HasNewMessage("[PNP Controller]: Robot in place position.");
      SetPnpMoveState(kPnpMovePlacing);
      break;
    case PnpMoveState::kPnpMoveToEndPosition:
      emit PnpSignal_HasNewMessage("[PNP Controller]: Robot moving to end position.");
//      m_device_place_complete_
      fx_plc_->FxWriteBit(FxPlc::kPLCDevice_M, m_device_place_complete_, true);
      SetPnpMoveState(kPnpMoveIdle);
      sum_picked_objects_ += 1;
      fx_plc_->FxWriteWord(FxPlc::kPLCDevice_D,
                           d_device_sample_counter_, sum_picked_objects_);
      if (sum_picked_objects_ >= 21) {
        PnpControllerStop();
        break;
      }
      if (is_possible_pick_) {
        MovePickAndPlace();
      }
      break;
  }
}

// main control for DH-Robotic device, just use rotate positive
void PnpController::RobotOuputIntTriggered(int value) {
  PnpDevicePin pin = static_cast<PnpDevicePin>(value);
  switch (pin) {
    case PnpDevicePin::kPnpPin_RgiRotate_Zero:
      dh_controller_->DH_AddFuncToQueue(
          DH_RGI::SetRotationAngle(rgi_address_, rgi_rotating_zero_));
      break;
    case PnpDevicePin::kPnpPin_RgiRotate_Angle:
      dh_controller_->DH_AddFuncToQueue(
          DH_RGI::SetRotationAngle(rgi_address_, object_rotate_angle_));
      break;
    case PnpDevicePin::kPnpPin_RgiRotate_Negative:
      dh_controller_->DH_AddFuncToQueue(
          DH_RGI::SetRotationAngle(rgi_address_, rgi_rotating_negative_));
      break;
    case PnpDevicePin::kPnpPin_RgiGrip_Open:
      dh_controller_->DH_AddFuncToQueue(
          DH_RGI::SetGripperPosition(rgi_address_, rgi_grip_open_pos_));
      break;
    case PnpDevicePin::kPnpPin_RgiGrip_Close:
      dh_controller_->DH_AddFuncToQueue(
          DH_RGI::SetGripperPosition(rgi_address_, rgi_grip_close_pos_));
      break;
    case PnpDevicePin::kPnpPin_PgcGrip_Open:
      dh_controller_->DH_AddFuncToQueue(
          DH_PGC::SetGripperPosition(pgc_address_, pgc_grip_open_pos_));
      break;
    case PnpDevicePin::kPnpPin_PgcGrip_Close:
      dh_controller_->DH_AddFuncToQueue(
          DH_PGC::SetGripperPosition(pgc_address_, pgc_grip_close_pos_));
      break;
    case PnpDevicePin::kPnpPin_PlcMoveConfirm:
      bool confirm_flag = true;
      // in case auxiliary relay not in device map, it seem config flag ignore
      fx_plc_->GetAuxiliaryRelayState(m_device_place_confirm_, confirm_flag);
      if (confirm_flag) {
        emit PnpSignal_HasNewMessage("[PNP Controller]: PLC confirm signal is set.");
        robot_->SetContinueTrigger();
        SetPnpMoveState(PnpMoveState::kPnpMoveToPlacePosition);
      } else {
        SetPnpMoveState(PnpMoveState::kPnpMoveWaitPlcConfirm);
      }
      break;
  }
}

void PnpController::RgiGripperStateChanged(DhGripperStatus state) {
  switch (move_state_current_) {
    case PnpMoveState::kPnpMoveToRotatePosition:
      if (state == DhGripperStatus::kGriperClampingObject) {
        robot_->SetContinueTrigger();
        SetPnpMoveState(kPnpMoveRotateRelease);
      }
      break;
    case PnpMoveState::kPnpMoveRotateRelease2:
      if (state == DhGripperStatus::kGriperAtPosition)  {
        robot_->SetContinueTrigger();
//        SetPnpMoveState(kPnpMoveToPlacePosition);
        SetPnpMoveState(kPnpMoveWaitPlcConfirm);
      }
      break;
  }
}

void PnpController::RgiRotateStateChanged(DhRotationStatus state) {
  switch (move_state_current_) {
    case PnpMoveState::kPnpMoveRotateObject:
      if (state == DhRotationStatus::kRotationAtPosition) {
        robot_->SetContinueTrigger();
        SetPnpMoveState(kPnpMovePickAfterRotate);
      }
      // handle when rotate module is bloking
      break;
  }
}

void PnpController::PgcGripperStateChanged(DhGripperStatus state) {
  switch (pnp_state_current_) {
    case PnpState::kPnpWaitMoveToStandby:
      if (state == DhGripperStatus::kGriperAtPosition) {
        emit PnpSignal_HasNewMessage("[PNP Controller]: Hand grip opened for picking.");
        TriggerGrabFrame();
      }
      return;
  }

  switch (move_state_current_) {
    case PnpMoveState::kPnpMovePicking:
      if (state == DhGripperStatus::kGriperClampingObject) {
        robot_->SetContinueTrigger();
        SetPnpMoveState(kPnpMoveToRotatePosition);
      }
      // handle when robot pick objet fail.
      break;
    case PnpMoveState::kPnpMoveRotateRelease: // check again
      if (state == DhGripperStatus::kGriperAtPosition) {
        emit PnpSignal_HasNewMessage("[PNP Controller]: Hand grip opened for rotate.");
        robot_->SetContinueTrigger();
        SetPnpMoveState(kPnpMoveRotateObject);
      }
      break;
    case PnpMoveState::kPnpMovePickAfterRotate:
      if (state == DhGripperStatus::kGriperClampingObject) {
        emit PnpSignal_HasNewMessage("[PNP Controller]: Hand grip closed for picking object from rotate moudle.");
        robot_->SetContinueTrigger();
        SetPnpMoveState(kPnpMoveRotateRelease2);
      }
      break;
    case PnpMoveState::kPnpMovePlacing:
      if (state == DhGripperStatus::kGriperAtPosition) {
        emit PnpSignal_HasNewMessage("[PNP Controller]: Hand grip opened for place.");
        robot_->SetContinueTrigger();
        SetPnpMoveState(kPnpMoveToEndPosition);
      }
      break;
  }
}

void PnpController::PlcMDeviceChanged(int number, bool value) {
  if ((number == m_device_pnp_start_) && value) {
    PnpControllerStart();
  } else if ((number == m_device_pnp_stop_) && value) {
    PnpControllerStop();
  } else if ((number == m_device_place_confirm_) && value) {
    if (move_state_current_ == PnpMoveState::kPnpMoveWaitPlcConfirm) {
      robot_->SetContinueTrigger();
      SetPnpMoveState(kPnpMoveToPlacePosition);
    }
  }
}

void PnpController::TimerTimeOut() {
  emit PnpSignal_HasNewMessage("[PNP Controller]: Time Timeout.");
  switch (pnp_state_current_) {
    case PnpState::kPnpWaitFeed:
      pnp_timer->stop();
      FeederEnable(false);
      emit PnpSignal_HasNewMessage("[PNP Controller]: Feeder disable.");
      TriggerPlateUpper();
      break;
    case PnpState::kPnpWaitScatt:
      pnp_timer->stop();
      flex_plate_->writeCommunicateMode(FlexibleFeed::CommunicationMode::Stop);
      emit PnpSignal_HasNewMessage("[PNP Controller]: Flexible plate stop.");
      TriggerPlateWaitStable();
      break;
    case PnpState::kPnpWaitUpper:
      pnp_timer->stop();
      flex_plate_->writeCommunicateMode(FlexibleFeed::CommunicationMode::Stop);
      emit PnpSignal_HasNewMessage("[PNP Controller]: Flexible plate stop.");
      TriggerPlateWaitStable();
      break;
    case PnpState::kPnpWaitStable:
      pnp_timer->stop();
      emit PnpSignal_HasNewMessage("[PNP Controller]: Wait objects stable time out.");
      TriggerGrabFrame();
      break;
  }
}

void PnpController::MoveToStandby() {
  SetPnpState(PnpState::kPnpWaitMoveToStandby);
  robot_->pushCommand(HansCommand::SetOverride(0, 100));
  position_standby_.Y += 1;
  robot_->pushCommand(HansCommand::WayPointL(0, position_standby_,
                                             veloc_low_, accel_low_, 0));
  position_standby_.Y -= 1;
  robot_->pushCommand(HansCommand::WayPointL(0, position_standby_,
                                             veloc_low_,accel_low_, 0));
  robot_->pushCommand(
      HansCommand::WaitStartMove(static_cast<int>(pnp_state_current_)));
  robot_->pushCommand(
      HansCommand::WaitMoveDone(static_cast<int>(pnp_state_current_)));
  robot_->pushCommand(
      HansCommand::TriggerOutputInt(static_cast<int>(kPnpPin_PgcGrip_Open)));
}

void PnpController::TriggerGrabFrame() {
  SetPnpState(PnpState::kPnpWaitGrabbingFrame);
  emit PnpSignal_GrabFrameTriggered();
  emit PnpSignal_HasNewMessage("[PNP Controller]: Camera grab frame triggered.");
}



void PnpController::MovePickAndPlace() {
  is_possible_pick_ = false;
  SetPnpState(PnpState::kPnpWaitRobotMove);
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

  double pick_dist_col_ = sum_picked_col_*col_distance_;
  double pick_dist_row_ = sum_picked_row_*row_distance_;
  sum_picked_col_ += 1;
  if (sum_picked_col_ >= 7) {
    sum_picked_col_ = 0;
    sum_picked_row_ += 1;
    if (sum_picked_row_ >= 3) {
      sum_picked_row_ = 0;
    }
  }

  place_point.X = 0.0 + pick_dist_row_;
  place_point.Y = 0.0 + pick_dist_col_;
  place_point.Z = 3.0;
  place_point.rX = 180.0;
  place_point.rY = 0.0;
  place_point.rZ = -90.0;
  place_point.plane = "Plane_place";
  place_point.tcp = "TCP_dh_gripper";

  object_rotate_angle_ = 0;
  if (matchObj.indexOfSample == 0) {
    object_rotate_angle_ = rgi_rotating_negative_;
  } else if (matchObj.indexOfSample == 1) {
    object_rotate_angle_ = rgi_rotating_positive_;
  }
  qDebug() << "Object name: " << matchObj.name
           << "Rotate angle:" << object_rotate_angle_;

  // push command to queue
  // config sub parameters
  robot_->pushCommand(HansCommand::SetOverride(0, 100));

  // move to pick position
  robot_->pushCommand(
      HansCommand::WayPointLRelRef(0, pick_point, 0, 0, 120, 0,
                                   veloc_fast_, accel_fast_, 10));
  robot_->pushCommand(
      HansCommand::WayPointL(0, pick_point, veloc_low_, accel_low_, 10));
  robot_->pushCommand(
      HansCommand::WaitStartMove(static_cast<int>(kPnpMoveToPickPosition)));
  robot_->pushCommand(
      HansCommand::WaitMoveDone(static_cast<int>(kPnpMoveToPickPosition)));

  // pick object
  robot_->pushCommand(
      HansCommand::TriggerOutputInt(static_cast<int>(kPnpPin_PgcGrip_Close)));
  robot_->pushCommand(
      HansCommand::WaitContinueTrigger());


  if (object_rotate_angle_ != 0) {
    // move up
    robot_->pushCommand(
        HansCommand::WayPointLRelRef(0, pick_point, 0, 0, 120, 0,
                                     veloc_fast_, accel_fast_, 10));

    // go to rotate point and clamping at rotate position
    robot_->pushCommand(
        HansCommand::WayPointLRelRef(0, position_rotate_, 0, -50, 50, 0,
                                     veloc_fast_, accel_fast_, 10));
    robot_->pushCommand(
        HansCommand::WayPointLRelRef(0, position_rotate_, 0, -50, 0, 0,
                                     veloc_fast_, veloc_fast_, 10));
    robot_->pushCommand(
        HansCommand::WayPointL(0, position_rotate_, veloc_low_, accel_low_, 10));
    robot_->pushCommand(
        HansCommand::WaitStartMove(static_cast<int>(kPnpMoveToRotatePosition)));
    robot_->pushCommand(
        HansCommand::WaitMoveDone(static_cast<int>(kPnpMoveToRotatePosition)));
    robot_->pushCommand(
        HansCommand::TriggerOutputInt(static_cast<int>(kPnpPin_RgiGrip_Close)));
    robot_->pushCommand(
        HansCommand::WaitContinueTrigger());

    // robot hand grip release object
    robot_->pushCommand(
        HansCommand::TriggerOutputInt(static_cast<int>(kPnpPin_PgcGrip_Open)));
    robot_->pushCommand(
        HansCommand::WaitContinueTrigger());

    // move robot z up and rotate object
    robot_->pushCommand(
        HansCommand::WayPointLRelRef(0, position_rotate_, 0, 0, 50, 0,
                                     veloc_fast_, veloc_fast_, 10));
    robot_->pushCommand(
        HansCommand::WaitStartMove(static_cast<int>(-1)));
    robot_->pushCommand(
        HansCommand::WaitMoveDone(static_cast<int>(-1)));
    robot_->pushCommand(
        HansCommand::TriggerOutputInt(static_cast<int>(kPnpPin_RgiRotate_Angle)));
    robot_->pushCommand(
        HansCommand::WaitContinueTrigger());

    // pick object from rotate position
    robot_->pushCommand(
        HansCommand::WayPointL(0, position_rotate_, veloc_low_, accel_low_, 10));
    robot_->pushCommand(
        HansCommand::WaitStartMove(-1));
    robot_->pushCommand(
        HansCommand::WaitMoveDone(-1));
    robot_->pushCommand(
        HansCommand::TriggerOutputInt(static_cast<int>(kPnpPin_PgcGrip_Close)));
    robot_->pushCommand(
        HansCommand::WaitContinueTrigger());

    // rotate module release object
    robot_->pushCommand(
        HansCommand::TriggerOutputInt(static_cast<int>(kPnpPin_RgiGrip_Open)));
    robot_->pushCommand(
        HansCommand::WaitContinueTrigger());

    // moving out rotate point
    robot_->pushCommand(
        HansCommand::WayPointLRelRef(0, position_rotate_, 0, -50, 0, 0,
                                     veloc_fast_, veloc_fast_, 10));
    robot_->pushCommand(
        HansCommand::WaitStartMove(-1));
    robot_->pushCommand(
        HansCommand::WaitMoveDone(-1));
    robot_->pushCommand(
        HansCommand::TriggerOutputInt(static_cast<int>(kPnpPin_RgiRotate_Zero)));
  } else {
    // move up
    robot_->pushCommand(
        HansCommand::WayPointLRelRef(0, pick_point, 0, 0, 120, 0,
                                     veloc_fast_, accel_fast_, 10));
    robot_->pushCommand(
        HansCommand::WaitStartMove(-1));
    robot_->pushCommand(
        HansCommand::WaitMoveDone(-1));
  }

  // wait plc place confirm
  robot_->pushCommand(
      HansCommand::TriggerOutputInt(static_cast<int>(kPnpPin_PlcMoveConfirm)));
  robot_->pushCommand(
      HansCommand::WaitContinueTrigger());

  // go to place position
  robot_->pushCommand(
      HansCommand::WayPointLRelRef(0, place_point, 0, 0, 250, 0,
                                   veloc_fast_, accel_fast_, 10));
  robot_->pushCommand(
      HansCommand::WayPointL(0, place_point, veloc_low_, accel_low_, 10));
  robot_->pushCommand(
      HansCommand::WaitStartMove(static_cast<int>(kPnpMoveToPlacePosition)));
  robot_->pushCommand(
      HansCommand::WaitMoveDone(static_cast<int>(kPnpMoveToPlacePosition)));

  // placing objects
  robot_->pushCommand(
        HansCommand::TriggerOutputInt(static_cast<int>(kPnpPin_PgcGrip_Open)));
    robot_->pushCommand(
        HansCommand::WaitContinueTrigger());

  // move to end position
  robot_->pushCommand(
      HansCommand::WayPointLRelRef(0, place_point, 0, 0, 250, 90,
                                   veloc_fast_, accel_fast_, 10));
  robot_->pushCommand(
      HansCommand::WaitStartMove(static_cast<int>(kPnpMoveToEndPosition)));
  robot_->pushCommand(
      HansCommand::WaitMoveDone(static_cast<int>(kPnpMoveToEndPosition)));
  emit PnpSignal_HasNewMessage("[PNP Controller]: Sending command to robot done.");
}

void PnpController::ReCheckMatchObjects() {
  switch (pnp_state_current_) {
    case PnpState::kPnpWaitRobotMove:
      emit PnpSignal_HasNewMessage("[PNP Controller]: Re-Checking objects in plate.");
      if(is_less_than_limit_) {
        TriggerFeeeder();
      }
      else {
        if(!is_found_2_objects_) {
          TriggerPlateScatt();
        }
        else {
          TriggerGrabFrame();
        }
      }
      break;
  }
}

void PnpController::TriggerFeeeder() {
  FeederEnable(true);
  pnp_timer->start(wait_feeding_time_);
  emit PnpSignal_HasNewMessage("[PNP Controller]: Feeder enable.");
  SetPnpState(PnpState::kPnpWaitFeed);
}

void PnpController::TriggerPlateScatt() {
  flex_plate_->writeCommunicateMode(FlexibleFeed::CommunicationMode::Scatter);
  flex_plate_->writeCommunicateMode(FlexibleFeed::CommunicationMode::Scatter);
  pnp_timer->start(wait_plate_stable_time_);
  emit PnpSignal_HasNewMessage("[PNP Controller]: Flexible plate active scatt mode.");
  SetPnpState(PnpState::kPnpWaitScatt);
}

void PnpController::TriggerPlateUpper() {
  flex_plate_->writeCommunicateMode(FlexibleFeed::CommunicationMode::Under);
  pnp_timer->start(wait_plate_upper_time_);
  emit PnpSignal_HasNewMessage("[PNP Controller]: Flexible plate active upper mode.");
  SetPnpState(PnpState::kPnpWaitUpper);
}

void PnpController::TriggerPlateWaitStable() {
  pnp_timer->start(wait_plate_stable_time_);
  emit PnpSignal_HasNewMessage("[PNP Controller]: Wait objects in plate stable.");
  SetPnpState(PnpState::kPnpWaitStable);
}

void PnpController::FeederEnable(bool state) {
  robot_->pushCommandInFront(HansCommand::SetBoxDO(feeder_port_, state));
}
