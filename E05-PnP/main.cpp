#include "mainwindow.h"

#include <QApplication>
#include <QLocale>
#include <QTranslator>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
//    w.show();
    w.showMaximized();
    return a.exec();
}

/*

{
"PosAndVel":{
"Actual_Position":["167.178", "-0.575", "107.882", "-179.991", "-71.579", "115.291", "119.317", "-57.930", "170.007", "179.968", "-0.021", "0.005"],
"Actual_PCS_TCP":["-392.005", "89.176", "172.006", "179.969", "-0.022", "29.384"],
"Actual_PCS_Base":["-392.088", "89.216", "310.506", "179.963", "-0.008", "51.884"],
"Actual_PCS_Tool":["0.000", "0.000", "0.000", "0.000", "0.000", "0.000"],
"Actual_Joint_Current":["0.263", "1.873", "-1.494", "0.389", "0.034", "0.065"],
"Actual_Joint_Velocity":["0.000", "0.000", "0.020", "0.000", "0.000", "-0.020"],
"Actual_Joint_Acceleration":["0.004", "-0.014", "-0.002", "-0.002", "-0.005", "-0.003"],
"Actual_Override":"1.000"
},
"EndIO":{
"EndDI":[0, 0, 0, 0],
"EndDO":[0, 0, 0, 0],
"EndButton":[0, 0, 0, 0],
"EnableEndBTN":0,
"EndAI":["0.024", "0.024"]
},
"ElectricBoxIO":{
"BoxCI":[0, 0, 0, 0, 0, 0, 0, 0],
"BoxCO":[0, 0, 0, 0, 0, 0, 0, 0],
"BoxDI":[0, 0, 0, 0, 1, 0, 0, 0],
"BoxDO":[0, 0, 0, 0, 0, 0, 0, 0],
"Conveyor":"0.000",
"Encode":0
},
"ElectricBoxAnalogIO":{
"BoxAnalogOutMode_1":0,
"BoxAnalogOutMode_2":0,
"BoxAnalogOut_1":"0.000",
"BoxAnalogOut_2":"0.000",
"BoxAnalogIn_1":"-0.031",
"BoxAnalogIn_2":"-0.031"
},
"StateAndError":{
"robotState":33,
"robotEnabled":1,
"robotPaused":0,
"robotMoving":0,
"robotBlendingDone":1,
"InPos":1,
"Error_AxisID":0,
"Error_Code":0,
"IsReduceMode":0,
"BrakeState":[0, 0, 0, 0, 0, 0],
"nAxisStatus":[3, 3, 3, 3, 3, 3],
"nAxisErrorCode":[0, 0, 0, 0, 0, 0],
"nResetSafeSpace":[1],
"nAxisGroupStatus":[1],
"nAxisGroupErrorCode":[0]
},
"HardLoad":{
"EtherCAT_TotalFrame":1238025,
"EtherCAT_FramesPerSecond":251,
"EtherCAT_TotalLostFrame":2,
"EtherCAT_TxErrorFrame":0,
"EtherCAT_RxErrorFrame":40,
"Box48IN_Voltage":"47.979",
"Box48IN_Current":"0.565",
"Box48Out_Voltage":"48.338",
"Box48Out_Current":"0.565",
"Slave_temperature":["39.125", "44.188", "51.438"],
"Slave_Voltage":["48.435", "48.554", "48.391"]
},
"FTData":{
"FTControlState":0,
"FTData":["0.000", "0.000", "0.000", "0.000", "0.000", "0.000"],
"FTSrcData":["0.000", "0.000", "0.000", "0.000", "0.000", "0.000"]
},
"Script":{
"errorCode":"0",
"cmdid":["", "", "", "", "", ""],
"GlobalVar":[{
"pick_r":"0.000"
}, {
"pick_x":"0.000"
}, {
"pick_y":"0.000"
}, {
"pick_z":"0.000"
}, {
"place_r":"0.000"
}, {
"place_x":"0.000"
}, {
"place_y":"0.000"
}, {
"place_z":"0.000"
}, {
"runf":"0"
}]
}
}LTBRA
5
*/
