#include "fxremote.h"

namespace fx {
FxRemote::FxRemote(QThread::Priority priority) {
  thread_priority_ = priority;
  is_thread_running_ = false;
}

FxRemote::~FxRemote() {

}

void FxRemote::FxSetConnectTimeOut(int time_out) {
  connect_time_out_ = time_out;
}

void FxRemote::FxSetReponseTimeOut(int time_out) {
  response_time_out_ = time_out;
}

void FxRemote::FxConnectToPlc(QString address, int port) {
  if (this->isRunning()) {
    return;
  }

  if (!is_thread_running_) {
    connect_address_ = address;
    connect_port_ = port;
    is_thread_running_ = true;
    start(thread_priority_);
  }
}

void FxRemote::FxDisconnectFromPlc() {
  if (is_thread_running_) {
    is_thread_running_ = false;
    quit();
  }
}

bool FxRemote::FxIsConnected() {
  return is_fx_connected;
}

void FxRemote::FxWriteBit(FxPlc::DeviceName device_name,
                quint32 device_number, bool value) {
  mutex_data_.tryLock(FX_MUTEX_BLOCK_TIME);
  SendCommand command;
  command.device_name = device_name;
  command.device_start_number = device_number;
  QList<bool> val_list;
  val_list.push_back(value);
  command.send_string = McProtocol::BatchRWriteBitUnits(device_name,
                                                        device_number,
                                                        val_list,
                                                        monitoring_time_,
                                                        pc_number_);
  send_queue_.push_back(command);
  mutex_data_.unlock();
}

void FxRemote::FxWriteWord(FxPlc::DeviceName device_name,
                 quint32 device_number,
                 quint16 value) {
  mutex_data_.tryLock(FX_MUTEX_BLOCK_TIME);
  SendCommand command;
  command.device_name = device_name;
  command.device_start_number = device_number;
  QList<quint16> val_list;
  val_list.push_back(value);
  command.send_string = McProtocol::BatchRWriteWordUnits(device_name,
                                                         device_number,
                                                         val_list,
                                                         monitoring_time_,
                                                         pc_number_);
  send_queue_.push_back(command);
  mutex_data_.unlock();
}

void FxRemote::FxSetPlcRunMode() {
  mutex_data_.tryLock(FX_MUTEX_BLOCK_TIME);
  SendCommand command;
  command.device_name = FxPlc::kPLCDevice_Unknown;
  command.device_start_number = 0;
  command.send_string = McProtocol::RemoteRun(monitoring_time_, pc_number_);
  send_queue_.push_back(command);
  mutex_data_.unlock();
}

void FxRemote::FxSetPlcStopMode() {
  mutex_data_.tryLock(FX_MUTEX_BLOCK_TIME);
  SendCommand command;
  command.device_name = FxPlc::kPLCDevice_Unknown;
  command.device_start_number = 0;
  command.send_string = McProtocol::RemoteStop(monitoring_time_, pc_number_);
  send_queue_.push_back(command);
  mutex_data_.unlock();
}

void FxRemote::FxGetModelName() {
  mutex_data_.tryLock(FX_MUTEX_BLOCK_TIME);
  SendCommand command;
  command.device_name = FxPlc::kPLCDevice_Unknown;
  command.device_start_number = 0;
  command.send_string = McProtocol::ReadPlcName(monitoring_time_, pc_number_);
  send_queue_.push_back(command);
  mutex_data_.unlock();
}

QString FxRemote::FxPlcModelName() {
  return plc_model_name_;
}

bool FxRemote::GetAuxiliaryRelayState(int device_number, bool &get_value) {
  bool is_get_success = false;
  mutex_data_.tryLock(FX_MUTEX_BLOCK_TIME);
  if (device_map_.m_map.contains(device_number)) {
    get_value = device_map_.m_map[device_number];
//    qDebug() << "Get value M Device:" << device_map_.m_map[device_number];
    is_get_success = true;
  }
  mutex_data_.unlock();
  return is_get_success;
}

bool FxRemote::FxIsPlcRunMode() {
  return plc_is_running_mode_;
}

bool FxRemote::FxIsPlcErrorOccurred() {
  return plc_is_error_occured_;
}

void FxRemote::FxSetDeviceMap(FxPlc::DeviceMap &device_map) {
  mutex_data_.tryLock(FX_MUTEX_BLOCK_TIME);
  device_map_ = device_map;
  mutex_data_.unlock();
}

void FxRemote::run() {
  InitSocket();

  if (!is_fx_connected) {
    return;
  }

  send_queue_.clear();
  FxGetModelName();
  polling_time_counter_ = new TimeCounter;

  while (is_thread_running_) {
    PollingQueryHandle();
    QueueCommandHandle();
  }
}

void FxRemote::InitSocket() {
  socket_ = new QTcpSocket;
  connect(this, &FxRemote::finished, socket_, &FxRemote::deleteLater);
  connect(socket_, &QTcpSocket::destroyed, this, &FxRemote::DeleteSocket);

  socket_->connectToHost(connect_address_, connect_port_);
  if (!socket_->waitForConnected(connect_time_out_)) {
    is_fx_connected = false;
    emit FxSignal_ConnectFail();
    return;
  }
  is_fx_connected = true;
  emit FxSignal_Connected();
}

void FxRemote::DeleteSocket() {
  if (is_fx_connected) {
    emit FxSignal_Disconnected();
  }
  is_thread_running_ = false;
  is_fx_connected = false;
}

void FxRemote::PollingQueryHandle() {
  if (!polling_time_counter_->StartTimeCounter(polling_time_out_)) {
    return;
  }

  QList<int> m_list = device_map_.m_map.keys();
  QList<int> y_list = device_map_.y_map.keys();
  QList<int> d_list = device_map_.d_map.keys();
  FindQueryList(m_list, query_list_m_);
  FindQueryList(y_list, query_list_y_);
  FindQueryList(d_list, query_list_d_);

  QueryDeviceM();
  QueryDeviceY();
  QueryDeviceD();
  QuerySpecialRelay();
  emit FxSignal_PollingQueryStriggered(device_map_);
}

void FxRemote::QueryDeviceM() {
  if (query_list_m_.isEmpty()) {
    return;
  }

  for (int index=0;index<query_list_m_.count();index++) {
    SendCommand command;
    command.device_name = FxPlc::kPLCDevice_M;
    command.device_start_number = query_list_m_[index].start;
    command.send_string = McProtocol::BatchReadBitUnits(FxPlc::kPLCDevice_M,
                                                        query_list_m_[index].start,
                                                        query_list_m_[index].number,
                                                        monitoring_time_,
                                                        pc_number_);
    SendRequest(command);
  }
}

void FxRemote::QueryDeviceY() {
  if (query_list_y_.isEmpty()) {
    return;
  }

  for (int index=0;index<query_list_y_.count();index++) {
    SendCommand command;
    command.device_name = FxPlc::kPLCDevice_Y;
    command.device_start_number = query_list_y_[index].start;
    command.send_string = McProtocol::BatchReadBitUnits(FxPlc::kPLCDevice_Y,
                                                        query_list_y_[index].start,
                                                        query_list_y_[index].number,
                                                        monitoring_time_,
                                                        pc_number_);
    SendRequest(command);
  }
}

void FxRemote::QueryDeviceD() {
  if (query_list_d_.isEmpty()) {
    return;
  }

  for (int index=0;index<query_list_d_.count();index++) {
    SendCommand command;
    command.device_name = FxPlc::kPLCDevice_D;
    command.device_start_number = query_list_d_[index].start;
    command.send_string = McProtocol::BatchReadWordUnits(FxPlc::kPLCDevice_D,
                                                        query_list_d_[index].start,
                                                        query_list_d_[index].number,
                                                        monitoring_time_,
                                                        pc_number_);
    SendRequest(command);
  }
}

void FxRemote::QuerySpecialRelay() {
  SendCommand command;
  command.device_name = FxPlc::kPLCDevice_M;
  command.device_start_number = 8000;
  command.send_string = McProtocol::BatchReadBitUnits(FxPlc::kPLCDevice_M,
                                                      command.device_start_number,
                                                      10,
                                                      monitoring_time_,
                                                      pc_number_);
  SendRequest(command);
}

void FxRemote::QueueCommandHandle() {
  mutex_data_.tryLock(FX_MUTEX_BLOCK_TIME);
  bool is_queue_empty = send_queue_.isEmpty();
  if (is_queue_empty) {
    mutex_data_.unlock();
    return;
  }

  SendCommand command = send_queue_.front();
  send_queue_.pop_front();
  mutex_data_.unlock();
  SendRequest(command);
}

void FxRemote::SendRequest(SendCommand  &command) {
  if (command.send_string.isEmpty()) {
    return;
  }

  socket_->write(command.send_string);
  if (socket_->waitForReadyRead(response_time_out_)) {
    QByteArray response_string = socket_->readAll();
    McProtocol::McResult result;
    result.device_name = command.device_name;
    result.device_start_number = command.device_start_number;
    if (McProtocol::ParseResponse(response_string, result)) {
      ResponseHandle(result);
    } else {
      // error reponse handle
      QString error_hex = "Ox" + QString::number(result.sub_header, 16);
      qDebug() << "Parse error:" << error_hex;
    }
  } else {
    // time-out handle
    qDebug() << "Response time out";
  }
}

void FxRemote::ResponseHandle(McProtocol::McResult &result) {
  // handle reponse
  switch (result.device_name) {
    case FxPlc::kPLCDevice_M:
      for (int index=0;index<result.response_data.count();index++) {
        int map_key = result.device_start_number + index;
        if (device_map_.m_map.contains(map_key)) {
          bool new_state = (result.response_data[index] == 0) ? false : true;
          if (device_map_.m_map[map_key] != new_state) {
//            qDebug() << "New state at M" << QString::number(map_key)
//                     << ":" << new_state;
            emit FxSignal_DeivceMChangedState(map_key, new_state);
          }
          device_map_.m_map[map_key] = new_state;
        }
        SpecialAuxiliaryRelayCheck(map_key, result.response_data[index]);
      }
      break;
    case FxPlc::kPLCDevice_Y:
      for (int index=0;index<result.response_data.count();index++) {
        int map_key = result.device_start_number + index;
        if (device_map_.y_map.contains(map_key)) {
          bool new_state = (result.response_data[index] == 0) ? false : true;
          if (device_map_.y_map[map_key] != new_state) {
//            qDebug() << "New state at Y" << QString::number(map_key)
//                     << ":" << new_state;
            emit FxSignal_DeivceYChangedState(map_key, new_state);
          }
          device_map_.y_map[map_key] = new_state;
        }
      }
      break;
    case FxPlc::kPLCDevice_D:
      for (int index=0, key_count=0;
           index<result.response_data.count();
           index+=2, key_count++) {
        int map_key = result.device_start_number + key_count;
        if (device_map_.d_map.contains(map_key)) {
          quint8 low_byte = static_cast<quint8>(result.response_data[index]);
          quint8 high_byte = static_cast<quint8>(result.response_data[index+1]);
          quint16 value = (high_byte << 8) + low_byte;
          quint16 previous_value = device_map_.d_map[map_key];
          if (previous_value != value) {
//            qDebug() << "New state at D" << QString::number(map_key)
//                     << ":" << value;
            emit FxSignal_DeviceDChangedValue(map_key, value);
          }
          device_map_.d_map[map_key] = value;
        }
      }
      break;
    case FxPlc::kPLCDevice_Unknown:
      if (result.sub_header == McProtocol::kSubHeader_ReadPlcName) {
        plc_model_name_ = McProtocol::ConvertToPLCName(result.response_data[0]);
//        qDebug() << plc_model_name_;
      }
      break;
  }
}

void FxRemote::SpecialAuxiliaryRelayCheck(int &relay_number, char &value) {
  if (relay_number == 8000) {
    plc_is_running_mode_ = (value == 0) ? false : true;
    return;
  }
  if (relay_number == 8004)  {
    plc_is_error_occured_ = (value == 0) ? false : true;
    return;
  }
}

void FxRemote::FindQueryList(QList<int> &raw_list,
                             QList<DeviceRange> &query_list) {
  query_list.clear();
  if (raw_list.count() <= 0) {
    return;
  }

  DeviceRange range;
  range.start = raw_list[0];
  range.number = 1;
  int previous_num_device = range.start;

  for (int index=1;index<raw_list.count();index++) {
    if (raw_list[index] == previous_num_device + 1) {
      previous_num_device = raw_list[index] ;
      range.number += 1;
    } else {
      query_list.push_back(range);
      range.start = raw_list[index];
      previous_num_device = range.start;
      range.number = 1;
    }
    // last index
    if (index == (raw_list.count() - 1)) {
      query_list.push_back(range);
    }
  }
  // push when has only one device
  if (query_list.count() == 0) {
    query_list.push_back(range);
  };
}
}
