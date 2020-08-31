//
// Created by rrzhang on 2020/7/1.
//

#ifndef DBX1000_PARESE_RESULT_H
#define DBX1000_PARESE_RESULT_H

void AppendRunTime(uint64_t run_time, uint64_t ins_id);
void AppendLatency(uint64_t latency, uint64_t ins_id);
void AppendThroughtput(uint64_t throughtput, uint64_t ins_id);
void AppendRemoteLockTime(uint64_t remoteLockTime, uint64_t ins_id);
void ParseRunTime();
void ParseLatency();
void ParseThroughtput();
void ParseRemoteLockTime();


#endif //DBX1000_PARESE_RESULT_H
