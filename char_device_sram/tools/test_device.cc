#include <iostream>
#include <fstream>
#include <string>

using namespace std;

int main() {
  string line;

  {
    // Test writing device multiple times at the same time.
    ofstream device("/dev/sram-dev");
    ofstream device1("/dev/sram-dev");

    if (device.is_open()) {
      device << "line 1\n";
      device << "line 2\n";
      device.close();
    } else {
      cout << "Unable to open file";
    }

    if (device1.is_open()) {
      device1 << "line 1\n";
      device1 << "line 2\n";
      device1.close();
    } else {
      cout << "Unable to open file";
    }
  }

  {
    // Test reading device multiple times at the same time.
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
  }

  cout << endl;
  return 0;
}
