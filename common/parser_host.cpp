//
// Created by rrzhang on 2020/6/6.
//
#include <cassert>
#include <map>
#include <fstream>
#include <iostream>

using namespace std;

#include "json/json.h"

int parser_host(int argc, char *argv[], std::map<int, std::string> &hosts_map) {
    int this_instance_id;
    for (int i = 1; i < argc; i++) {
        assert(argv[i][0] == '-');
        if (std::string(&argv[i][1], 11) == "instance_id") {
            this_instance_id = std::stoi(&argv[i][13]);
        }
    }

    std::ifstream in("../config.json", std::ios::out | std::ios::binary);
    assert(in.is_open());
    Json::Value root;
    in >> root;
    Json::Value::Members members = root.getMemberNames();
    for (auto iter = members.begin(); iter != members.end(); iter++) {
        hosts_map.insert(std::pair<int, std::string>(
                root[*iter]["id"].asInt(), root[*iter]["ip"].asString() + ":" + root[*iter]["port"].asString()));
    }
    in.close();
    return this_instance_id;
}

void Test_parser_host(int argc, char *argv[]) {
    std::map<int, std::string> hosts_map;
    cout << parser_host(argc, argv, hosts_map) << endl;
    for(auto iter : hosts_map) {
        cout << "id:" << iter.first << ", host:" << iter.second << endl;
    }
}

//int main(int argc, char *argv[]) {
//    Test_parser_host(argc, argv);
//    return 0;
//}