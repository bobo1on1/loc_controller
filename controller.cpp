#include <Arduino.h>
#include <EtherCard.h>
#include "controller.h"
#include "debugprint.h"

byte Ethernet::buffer[600];

#if STATIC
static byte myip[] = { 192,168,1,200 };
static byte gwip[] = { 192,168,1,1 };
#endif

extern void Transmit(uint8_t* data, uint16_t size, uint16_t sourceport, const uint8_t* destip, uint16_t destport);

CController::CController() : m_artnet(*this, Ethernet::buffer + UDP_DATA_P, ether.myip, ether.mymac)
{
}

#define DATAPIN 4
#define CLOCKPIN 1
#define SELECTPIN 6
#define ETHERRESETPIN 9

void CController::Initialize()
{
  //make the pixel/strip pin an input, and enable the internal pullup
  pinMode(SELECTPIN, INPUT);
  digitalWrite(SELECTPIN, HIGH);
  //let the pin rise if the jumper is open
  delay(10);

  //add led chip based on jumper position
  if (digitalRead(SELECTPIN))
    LEDS.addLeds<WS2811, DATAPIN, GRB>(m_leds, NUM_LEDS); //led strip
  else
    LEDS.addLeds<WS2801, DATAPIN, CLOCKPIN, BRG>(m_leds, NUM_LEDS); //led pixel

  //make all leds white
  memset(m_leds, 0xFF, sizeof(m_leds));
  LEDS.show();

  //init the led timestamp
  m_ledshowtime = millis();

  pinMode(ETHERRESETPIN, OUTPUT);

  bool success = false;
  do
  {
    //make the reset pin low for 100 ms, to reset the ENC28J60
    digitalWrite(ETHERRESETPIN, LOW);
    delay(100);
    digitalWrite(ETHERRESETPIN, HIGH);
    delay(100);

    uint8_t mac[] = { 0x70,0x69,0x69,0x2D,0x30,0x31 };
    if (ether.begin(sizeof(Ethernet::buffer), mac))
    {
      DBGPRINT("Ethernet controller set up\n");
    }
    else
    {
      DBGPRINT("Failed to set up Ethernet controller\n");
      continue;
    }

    //delay for the ethernet switch to bring up stuff
    delay(5000);

    //enable broadcast for dhcp and art-net
    ether.enableBroadcast();

#if STATIC
    ether.staticSetup(myip, gwip);
#else
    DBGPRINT("Requesting ip address using DHCP\n");

    if (ether.dhcpSetup())
    {
      DBGPRINT("DHCP succeeded\n");
    }
    else
    {
      DBGPRINT("DHCP failed\n");
      continue;
    }
#endif

    success = true;
  }
  while (!success);

  DBGPRINT("IP: %i.%i.%i.%i\n", ether.myip[0], ether.myip[1], ether.myip[2], ether.myip[3]);
  DBGPRINT("GW: %i.%i.%i.%i\n", ether.gwip[0], ether.gwip[1], ether.gwip[2], ether.gwip[3]);
  DBGPRINT("DNS: %i.%i.%i.%i\n", ether.dnsip[0], ether.dnsip[1], ether.dnsip[2], ether.dnsip[3]);
  DBGPRINT("MASK: %i.%i.%i.%i\n", ether.mymask[0], ether.mymask[1], ether.mymask[2], ether.mymask[3]);

  SetPortAddressFromIp();

  m_artnet.Initialize();
}

void CController::SetPortAddressFromIp()
{
  //store the ip address and subnetmask into uint32_t
  uint32_t address = 0;
  uint32_t mask = 0;
  for (uint8_t i = 0; i < 4; i++)
  {
    address |= (uint32_t)ether.myip[i] << (3 - i) * 8;
    mask |= (uint32_t)ether.mymask[i] << (3 - i) * 8;
  }

  //do a bitwise and with the subnetmask to get the host address
  uint32_t hostaddress = address & ~mask;
  //subtract one from the host address, take the 15 least significant bits, and use it as the art-net portaddress
  uint16_t portaddress = (hostaddress - 1) & 0x3FFF;

  m_artnet.SetPortAddress(portaddress);
  //set the ArtPollReply delay so that if a lot of these controllers are on the network,
  //and their ip addresses are numbered sequentually,
  //a controller will send ArtPollReply every 2 milliseconds
  //this prevents the ENC28J60 buffer filling up with a shitload of ArtPollReply broadcasts
  uint16_t delay = (hostaddress % 1000) * 2;
  m_artnet.SetPollReplyDelay(delay);

  DBGPRINT("address:%lu\n", address);
  DBGPRINT("mask:%lu\n", mask);
  DBGPRINT("hostaddress:%lu\n", hostaddress);
  DBGPRINT("portaddress:%u\n", portaddress);
  DBGPRINT("net:%i subnet:%i universe:%i\n", (portaddress >> 8) & 0xFF, (portaddress >> 4) & 0xF, portaddress & 0xF);
  DBGPRINT("artpollreply delay: %i\n", delay);
}

void CController::Process()
{
  uint32_t now = millis();
  m_artnet.Process(now);

  if (now - m_ledshowtime >= 1000)
  {
    LEDS.show();
    m_ledshowtime = now;
  }
}

void CController::HandlePacket(byte ip[4], uint8_t* data, uint16_t len)
{
  m_artnet.HandlePacket(ip, data, len);
}

void CController::Transmit(uint8_t* data, uint16_t size, uint16_t sourceport, const uint8_t* destip, uint16_t destport)
{
  ::Transmit(data, size, sourceport, destip, destport);
}

void CController::OnDmxData(uint8_t* data, uint16_t channels)
{
  memcpy(m_leds, data, min(channels, sizeof(m_leds)));
  LEDS.show();
  m_ledshowtime = millis();
}

