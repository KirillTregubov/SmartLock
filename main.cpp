#include "EthernetNetIf.h"
#include "NTPClient.h"
#include "mbed.h"

EthernetNetIf eth; 
NTPClient ntp;

// main() runs in its own thread in the OS
int main() {
  printf("Start\n");

  printf("Setting up...\n");
  EthernetErr ethErr = eth.setup();
  if (ethErr) {
    printf("Error %d in setup.\n", ethErr);
    return -1;
  }
  printf("Setup OK\r\n");

  time_t ctTime;
  ctTime = time(NULL);
  printf("Current time is (UTC): %s\n", ctime(&ctTime));

  Host server(IpAddr(), 123, "0.uk.pool.ntp.org");
  ntp.setTime(server);

  ctTime = time(NULL);
  printf("\nTime is now (UTC): %s\n", ctime(&ctTime));
}
