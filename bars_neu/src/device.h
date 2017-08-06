#ifndef device_h
#define device_h

#include <Arduino.h>

class Device {
  protected:
    String _type;
    int _number;
  public:
    Device(String type);
    String getType();
    int getNumber();
    void setNumber(int number);
    void onboardledblink();
    void onboardledblink(int n);
};

#endif
