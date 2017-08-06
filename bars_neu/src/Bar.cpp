#include "Bar.h"

Bar::Bar(PubSubClient client)
  : Device ("StandardLight"){
    this->_client = client;
}
