#include <iostream>
#include <vector>
#include <string>

#include "ftd2xx.h"
#include "libft4222.h"

#include "ft4222CustomLibrary.h"

using std::vector;

int main(int argc, char const *argv[]) {
  vector< FT_DEVICE_LIST_INFO_NODE > ft4222DeviceList;

  Ft4222CustomLibrary::getFtUsbDevices(ft4222DeviceList);
  Ft4222CustomLibrary::printFtUsbDevices(ft4222DeviceList);
}
