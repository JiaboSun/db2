//
// Created by rrzhang on 2020/4/27.
//

#include "myhelper.h"

namespace dbx1000 {
    int MyHelper::RCToInt(RC rc) {
        if(RC::RCOK == rc)   { return 0;}
        if(RC::Commit == rc) { return 1;}
        if(RC::Abort == rc)  { return 2;}
        if(RC::WAIT == rc)   { return 3;}
        if(RC::ERROR == rc)  { return 4;}
        if(RC::FINISH == rc) { return 5;}
    }

    RC MyHelper::IntToRC(int i) {
        if(0 == i) { return RC::RCOK; }
        if(1 == i) { return RC::Commit; }
        if(2 == i) { return RC::Abort; }
        if(3 == i) { return RC::WAIT; }
        if(4 == i) { return RC::ERROR; }
        if(5 == i) { return RC::FINISH; }
    }

    int MyHelper::AccessToInt(access_t type) {
        if(RD == type)   { return 0;}
        if(WR == type)   { return 1;}
        if(XP == type)   { return 2;}
        if(SCAN == type) { return 3;}
    }

    access_t MyHelper::IntToAccess(int i) {
        if(0 == i) { return RD; }
        if(1 == i) { return WR; }
        if(2 == i) { return XP; }
        if(3 == i) { return SCAN; }
    }

    int MyHelper::TsTypeToInt(TsType ts_type) {
        if(R_REQ == ts_type)  { return 0; }
        if(W_REQ == ts_type)  { return 1; }
        if(P_REQ == ts_type)  { return 2; }
        if(XP_REQ == ts_type) { return 3; }
    }

    TsType MyHelper::IntToTsType(int i) {
        if(0 == i) { return R_REQ; }
        if(1 == i) { return W_REQ; }
        if(2 == i) { return P_REQ; }
        if(3 == i) { return XP_REQ; }
    }

    std::chrono::time_point<std::chrono::system_clock, std::chrono::nanoseconds> MyHelper::GetSysClock() {
        return std::chrono::system_clock::now();
    }
}