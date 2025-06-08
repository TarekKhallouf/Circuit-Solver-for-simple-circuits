#include <iostream>
#include <vector>
using namespace std;

class component {
  public:
    string name;
    float value;
    char point1, point2, conn;

    bool isSeries(component obj) {
      if ((obj.conn == 's' && this->point2 == obj.point1) || (this->conn == 's' && this->point1 == obj.point2)) {
        return true;  
      } else
        return false; 
    }

    bool isParallel(component obj) {
      if (obj.conn == 'p' && ((this->point1 == obj.point1 && this->point2 == obj.point2) || (this->point2 == obj.point1 && this->point1 == obj.point2))) {
        return true;
      } else
        return false;
    }
};

class resistor : public component {
  public:
    resistor() {}
    resistor(string componentName, float componentValue, char point1, char point2, char conn) {
        this->name = componentName;
        this->value = componentValue;
        this->point1 = point1;
        this->point2 = point2;
        this->conn = conn;
    }

    resistor operator+(resistor obj) {
      resistor req;
      req.value = this->value + obj.value;
      req.point1 = this->point1;
      req.point2 = obj.point2;
      req.conn = 'p';
      return req;
    }

    resistor operator||(resistor obj) {
      resistor req;
      req.value = (this->value * obj.value) / (this->value + obj.value);
      req.point1 = this->point1;
      req.point2 = obj.point2;
      req.conn = 's';
      return req;
    }
};

class voltSource : public component {
  public:
    voltSource() {}
    voltSource(string componentName, float componentValue, char point1, char point2) {
      this->name = componentName;
      this->value = componentValue;
      this->point1 = point1;
      this->point2 = point2;
    }
};

class currentSource : public component {
  public:
    currentSource() {}
    currentSource(string componentName, float componentValue, char point1, char point2, char conn) {
      this->name = componentName;
      this->value = componentValue;
      this->point1 = point1;
      this->point2 = point2;
      this->conn = conn;
    }
};

resistor getLoadResistor(vector<resistor> resistors, char loadNode1, char loadNode2) {
  resistor loadResistor;

  for(int i = 0; i < resistors.size(); i++) {
    if(resistors[i].point1 == loadNode1 && resistors[i].point2 == loadNode2)
      loadResistor = resistors[i];
  }
  return loadResistor;
}

float calculateRth(vector<resistor>& resistors, vector<voltSource>& volts, vector<currentSource>& currs, char loadNode1, char loadNode2) {
  resistor Rth;
  vector<char> openNodes = {currs[0].point1, currs[0].point2};

  // Delete load Resistor
  for(int i = 0; i < resistors.size(); i++) {
    if(resistors[i].point1 == loadNode1 && resistors[i].point2 == loadNode2) {
      resistors.erase(resistors.begin() + i);
      break;
    }
  }

  // Combining parallel
  for(int i = resistors.size() - 1; i >= 1; i--) {
    if(resistors[i - 1].isParallel(resistors[i])) {
      resistors[i - 1] = resistors[i - 1] || resistors[i];
      resistors.erase(resistors.begin() + i);
    }
  }

  // Delete all series with current source
  for (int i = resistors.size() - 1; i >= 1; i--) {
    if(resistors[i].point1 == openNodes[1]) {
      openNodes[1] = resistors[i].point2;
      resistors.erase(resistors.begin() + i);
    } else if(resistors[i].point2 == openNodes[0]) {
      openNodes[0] = resistors[i].point1;
      resistors.erase(resistors.begin() + i);
    }
  }

  if(resistors.size() == 1) {
      Rth = resistors[0];
  } else {
    for (int i = resistors.size() - 1; i >= 1; i--) {
      if(resistors[i - 1].isSeries(resistors[i])) {
          resistors[i - 1] = resistors[i - 1] + resistors[i];
      } else if(resistors[i - 1].isParallel(resistors[i])) {
          resistors[i - 1] = resistors[i - 1] || resistors[i];
      }
      Rth = resistors[i - 1];
    }
  }
  return Rth.value;
}

float calculateVth(vector<resistor>& resistors, vector<voltSource>& volts, vector<currentSource>& currs, char node1, char node2) {
  // Combining parallel resistors
  for(int i = resistors.size() - 1; i >= 1; i--) {
    if (resistors[i - 1].isParallel(resistors[i])) {
        resistors[i - 1] = resistors[i - 1] || resistors[i];
        resistors.erase(resistors.begin() + i);
    }
  }
  // Deleting Load resistor
  for(int i = 0; i < resistors.size(); i++) {
    if (resistors[i].point1 == node1 && resistors[i].point2 == node2) {
        resistors.erase(resistors.begin() + i);
    }
    // Checking if there are resistors parallel with the load resistor
    if(resistors[i].conn == 'p' && resistors[i].point1 == node1) {
        resistors[i].conn = 's';
    }
  }

  // Calculate the voltage
  float Vth = 0.0;
  float Iq = currs[0].value;

  if(!volts.empty()) {
    Vth = volts[0].value;
  }

  for(int i = 0; i < resistors.size(); i++) {
    if(!volts[0].isSeries(resistors[i])){
      break;
    }
    else {
      Vth -= (resistors[i].value * Iq);
      volts[0].point2 = resistors[i].point2;
    }
  }
  return Vth;
  }

float calculateIn(vector<resistor>& resistors, vector<voltSource>& volts, vector<currentSource>& currs, char node1, char node2){
  float In,Ir,Iq=currs[0].value;

  for(int i = 0; i < resistors.size(); i++) { // shorten load resistor
    if(resistors[i].point1 == node1 && resistors[i].point2 == node2) {
      resistors[i].point1 = resistors[i].point2;
    }
  }

  int i = 0;
  resistor Req;
  Req.value = 0;

  while (volts[0].isSeries(resistors[i])){
    Req = Req + resistors[i];
    volts[0].point2 = resistors[i].point2;
  }

  Ir = volts[0].value / Req.value;
  In = Ir - Iq;
  return In;
}

int main() {
  string vName;
  float vValue;
  char vPoint1, vPoint2;

  cout << "Enter voltage source name: ";
  cin >> vName;
  cout << "Enter voltage source value(volt): ";
  cin >> vValue;
  cout << "Enter voltage source point1: ";
  cin >> vPoint1;
  cout << "Enter voltage source point2: ";
  cin >> vPoint2;

  voltSource v1(vName, vValue, vPoint1, vPoint2);

  int numResistors;
  cout << "Enter number of resistors: ";
  cin >> numResistors;

  vector<resistor> resistors;
  vector<resistor> resistorsCopy1;
  vector<resistor> resistorsCopy2;
  vector<resistor> resistorsCopy3;

  for(int i = 0; i < numResistors; i++) {
    string rName;
    float rValue;
    char rPoint1, rPoint2, rConn;

    cout << "Enter resistor R" << (i + 1) << " name: ";
    cin >> rName;
    cout << "Enter resistor R" << (i + 1) << " value(ohm): ";
    cin >> rValue;
    cout << "Enter resistor R" << (i + 1) << " point1: ";
    cin >> rPoint1;
    cout << "Enter resistor R" << (i + 1) << " point2: ";
    cin >> rPoint2;
    cout << "Enter resistor R" << (i + 1) << " connection type (s for series, p for parallel): ";
    cin >> rConn;

    resistors.emplace_back(rName, rValue, rPoint1, rPoint2, rConn);
    resistorsCopy1.emplace_back(rName, rValue, rPoint1, rPoint2, rConn);
    resistorsCopy2.emplace_back(rName, rValue, rPoint1, rPoint2, rConn);
    resistorsCopy3.emplace_back(rName, rValue, rPoint1, rPoint2, rConn);
  }

  string cName;
  float cValue;
  char cPoint1, cPoint2, cConn;

  cout << "Enter current source name: ";
  cin >> cName;
  cout << "Enter current source value(A): ";
  cin >> cValue;
  cout << "Enter current source point1: ";
  cin >> cPoint1;
  cout << "Enter current source point2: ";
  cin >> cPoint2;
  cout << "Enter current source connection type (s for series): ";
  cin >> cConn;

  currentSource c1(cName, cValue, cPoint1, cPoint2, cConn);

  vector<currentSource> currs = {c1};
  vector<voltSource> volts = {v1};

  char loadNode1, loadNode2;
  cout << "Enter load node 1: ";
  cin >> loadNode1;
  cout << "Enter load node 2: ";
  cin >> loadNode2;

  float Rth = calculateRth(resistors, volts, currs, loadNode1, loadNode2);
  float Vth = calculateVth(resistorsCopy1, volts, currs, loadNode1, loadNode2);
  float Iload = Vth / (Rth + getLoadResistor(resistorsCopy2, loadNode1, loadNode2).value);

  cout << endl;
  cout << "Load Resistor Value: " << getLoadResistor(resistorsCopy2, loadNode1, loadNode2).value << "(ohm)" <<  endl << endl;
  cout << "Rth = RN : " << Rth << "(ohm)" << endl << endl;
  cout << "Vth: " << Vth << "(volt)" << endl << endl;

  float IN = calculateIn(resistorsCopy3,volts,currs,loadNode1,loadNode2);
  cout << "IN: " << IN << "(A)" << endl << endl;
  cout << "Load Current: " << (Iload*1000) << "(mA)" << endl;

  return 0;
}