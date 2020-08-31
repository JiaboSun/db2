//
// Created by rrzhang on 2020/7/13.
//

#ifndef DBX1000_ROW_HANDLER_H
#define DBX1000_ROW_HANDLER_H

#include "common/global.h"


class txn_man;
namespace dbx1000 {

    class RowItem;
    class ManagerInstance;
    class RowHandler {
    public:
        RowHandler(ManagerInstance *);
        RC GetRow(uint64_t key, access_t type, txn_man *txn, RowItem *&);
        void ReturnRow(uint64_t key, access_t type, txn_man *txn, RowItem *);
        bool SnapShotReadRow(RowItem *row);
        bool ReadRow(RowItem *row);
        bool WriteRow(RowItem *row);

    private:
        ManagerInstance *manager_instance_;
    };
}


#endif //DBX1000_ROW_HANDLER_H
