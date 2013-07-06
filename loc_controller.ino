#include <avr/wdt.h>
#include <EtherCard.h>
#include <IPAddress.h>
#include "controller.h"
#include "debugprint.h"

CController g_controller;

void setup()
{
  SetupWatchdog();
  SetupDebug();
  DBGPRINT("board started\n");

  g_controller.Initialize();
  ether.udpServerListenOnPort(&UdpArtNet, ARTNETPORT);
  //when data is received on one port lower than the art-net port
  //the artpollreply is sent as unicast, this is to prevent filling up the buffers
  //of all art-net controllers on the network
  ether.udpServerListenOnPort(&UdpArtNet, ARTNETPORT - 1);
}

void loop()
{
  ether.packetLoop(ether.packetReceive());
  g_controller.Process();
}

#if DEBUG
static FILE uartout = {0};
static int uart_putchar (char c, FILE *stream)
{
    Serial.write(c);
    return 0 ;
}
#endif

void SetupWatchdog()
{
  //the WDTON fuse should be programmed to 0 in the high fuse byte,
  //to make the watchdog timer permanently enabled
  //here the watchdog timer is set to the comfortable timeout of 8 seconds
  cli();
  wdt_reset();
  wdt_enable(WDTO_8S);
  sei();
}

void SetupDebug()
{
#if DEBUG
  //set up the serial port, and redirect stdout to it, this will allow using printf
  Serial.begin(57600);
  fdev_setup_stream(&uartout, uart_putchar, NULL, _FDEV_SETUP_WRITE);
  stdout = &uartout;
#endif
}

void UdpArtNet(word port, byte ip[4], const char* data, word len)
{
#if DEBUG
  DBGPRINT("--------------------------------------------\n");
  DBGPRINT("IP: %i.%i.%i.%i\n", ip[0], ip[1], ip[2], ip[3]);
  DBGPRINT("PORT: %i\n", port);
  DBGPRINT("LEN: %i\n", len);
  DBGPRINT("DATA: ");
  int i;
  for (i = 0; i < len - 1; i++)
  {
    if (data[i] >= 32)
      DBGPRINTAUX("%c", data[i]);
    else
      DBGPRINTAUX("?");
  }

  if (data[i] != '\n')
  {
    if (data[i] >= 32)
      DBGPRINTAUX("%c", data[i]);
    else
      DBGPRINTAUX("?");
  }

  DBGPRINTAUX("\n");
#endif

  g_controller.HandlePacket(ip, port, (uint8_t*)data, len);
}

void Transmit(uint8_t* data, uint16_t size, uint16_t sourceport, const uint8_t* destip, uint16_t destport)
{
  ether.sendUdp((char*)data, size, sourceport, (uint8_t*)destip, destport);
}

