#include <iostream>
#include <fstream>
#include <string>

using namespace std;

int main() {
  string line;

  // Test opening device multiple times at the same time.
  ifstream device("/dev/sram-dev");
  ifstream device1("/dev/sram-dev");

  if (device.is_open()) {
    while (getline(device, line) ) {
      cout << line << endl;
    }
    device.close();
  } else {
    cout << "Unable to open file";
  }

  if (device1.is_open()) {
    while (getline(device1, line) ) {
      cout << line << endl;
    }
    device1.close();
  } else {
    cout << "Unable to open file";
  }

  cout << endl;
  return 0;
}
