//
// Created by rrzhang on 2020/7/1.
//

#include <iostream>
#include <fstream>
#include <iomanip>
#include "util/parse_result.h"

using namespace std;


int main() {
    cout << fixed;
    ParseRunTime();
    ParseLatency();
    ParseThroughtput();
    ParseRemoteLockTime();


    return 0;
}