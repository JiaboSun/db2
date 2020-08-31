//
// Created by rrzhang on 2020/7/1.
//
#include <cassert>
#include <fstream>
#include <iostream>
#include <sstream>
#include <mutex>
#include "unistd.h"
#include "config.h"
using namespace std;

#include "parse_result.h"

std::mutex mtx;

void AppendRunTime(uint64_t run_time, uint64_t ins_id) {
    std::lock_guard<std::mutex> lck(mtx);
    ofstream out(std::string("runtime") + "_" + to_string(ins_id), ios::app | ios::out);
    assert(out.is_open());
    out << run_time << endl;
}

void AppendLatency(uint64_t latency, uint64_t ins_id) {
    std::lock_guard<std::mutex> lck(mtx);
    ofstream out(std::string("latency") + "_" + to_string(ins_id), ios::app | ios::out);
    assert(out.is_open());
    out << latency << endl;
}
void AppendThroughtput(uint64_t throughtput, uint64_t ins_id) {
    std::lock_guard<std::mutex> lck(mtx);
    ofstream out(std::string("throughtput") + "_" + to_string(ins_id), ios::app | ios::out);
    assert(out.is_open());
    out << throughtput << endl;
}
void AppendRemoteLockTime(uint64_t remoteLockTime, uint64_t ins_id) {
    std::lock_guard<std::mutex> lck(mtx);
    ofstream out(std::string("remote_lock_time") + "_" + to_string(ins_id), ios::app | ios::out);
    assert(out.is_open());
    out << remoteLockTime << endl;
}


void ParseRunTime(){
    uint64_t temp;
    int ins_count = 0;
    uint64_t run_time = 0;

    for(int i = 0; i < PROCESS_CNT; i++) {
        if(access((std::string("runtime") + "_" + to_string(i)).data(), F_OK) < 0){ continue; }
        ins_count++;
        ifstream in(std::string("runtime") + "_" + to_string(i), ios::app | ios::out);
        assert(in.is_open());
        std::stringstream ss;
        ss << in.rdbuf();

        while (ss >> temp) {
            run_time += temp;
        }
        in.close();
    }
    cout << "run_time : " << run_time / ins_count << ", ins_count : " << ins_count << endl;
}
void ParseLatency(){
    uint64_t temp;
    int ins_count = 0;
    uint64_t total_latency = 0;

    for(int i = 0; i < PROCESS_CNT; i++) {
        if(access((std::string("latency") + "_" + to_string(i)).data(), F_OK) < 0){ continue; }
        ins_count++;
        ifstream in(std::string("latency") + "_" + to_string(i), ios::app | ios::out);
        assert(in.is_open());
        std::stringstream ss;
        ss << in.rdbuf();

        while (ss >> temp) {
            total_latency += temp;
        }
        in.close();
    }
    cout << "total_latency : " << total_latency / ins_count << ", ins_count : " << ins_count << endl;
}

void ParseThroughtput(){
    uint64_t temp;
    int ins_count = 0;
    uint64_t throughtput = 0;

    for(int i = 0; i < PROCESS_CNT; i++) {
        if(access((std::string("throughtput") + "_" + to_string(i)).data(), F_OK) < 0){ continue; }
        ins_count++;
        ifstream in(std::string("throughtput") + "_" + to_string(i), ios::app | ios::out);
        assert(in.is_open());
        std::stringstream ss;
        ss << in.rdbuf();

        while (ss >> temp) {
            throughtput += temp;
        }
        in.close();
    }
    cout << "throughtput : " << throughtput << ", ins_count : " << ins_count << endl;
}
void ParseRemoteLockTime(){
    uint64_t temp;
    int ins_count = 0;
    uint64_t remote_lock_time = 0;

    for(int i = 0; i < PROCESS_CNT; i++) {
        if(access((std::string("throughtput") + "_" + to_string(i)).data(), F_OK) < 0){ continue; }
        ins_count++;
        ifstream in(std::string("remote_lock_time") + "_" + to_string(i), ios::app | ios::out);
        assert(in.is_open());
        std::stringstream ss;
        ss << in.rdbuf();

        while (ss >> temp) {
            remote_lock_time += temp;
        }
        in.close();
    }
    cout << "remote_lock_time : " << remote_lock_time << ", ins_count : " << ins_count << endl;
}