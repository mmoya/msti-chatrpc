#include "config.h"

program CHATRPC {
  version VERSION {
    string REGISTER (string) = 1;
    int SENDMSG (string, string, string) = 2;
    string RECVMSG (string, string) = 3;
    int DELMSG (string, string) = 4;
  } = RPCVERSIONID;
} = RPCPROGRAMID;
