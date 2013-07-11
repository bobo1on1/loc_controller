#include <Arduino.h>
#include "artnet.h"
#include "controller.h"
#include "debugprint.h"

#if DEBUG
static const PROGMEM char* OpcodeToStr(uint16_t opcode)
{
  switch(opcode)
  {
    case OpPoll:
      return PSTR("OpPoll");

    case OpPollReply:
      return PSTR("OpPollReply");

    case OpDiagData:
      return PSTR("OpDiagData");

    case OpCommand:
      return PSTR("OpCommand");

    case OpOutput:
      return PSTR("OpOutput");

    case OpNzs:
      return PSTR("OpNzs");

    case OpAddress:
      return PSTR("OpAddress");

    case OpInput:
      return PSTR("OpInput");

    case OpTodRequest:
      return PSTR("OpTodRequest");

    case OpTodData:
      return PSTR("OpTodData");

    case OpTodControl:
      return PSTR("OpTodControl");

    case OpRdm:
      return PSTR("OpRdm");

    case OpRdmSub:
      return PSTR("OpRdmSub");

    case OpVideoSetup:
      return PSTR("OpVideoSetup");

    case OpVideoPalette:
      return PSTR("OpVideoPalette");

    case OpVideoData:
      return PSTR("OpVideoData");

    case OpMacMaster:
      return PSTR("OpMacMaster");

    case OpMacSlave:
      return PSTR("OpMacSlave");

    case OpFirmwareMaster:
      return PSTR("OpFirmwareMaster");

    case OpFirmwareReply:
      return PSTR("OpFirmwareReply");

    case OpFileTnMaster:
      return PSTR("OpFileTnMaster");
      
    case OpFileFnMaster:
      return PSTR("OpFileFnMaster");

    case OpFileFnReply:
      return PSTR("OpFileFnReply");

    case OpIpProg:
      return PSTR("OpIpProg");

    case OpIpProgReply:
      return PSTR("OpIpProgReply");

    case OpMedia:
      return PSTR("OpMedia");

    case OpMediaPatch:
      return PSTR("OpMediaPatch");

    case OpMediaControl:
      return PSTR("OpMediaControl");

    case OpMediaContrlReply:
      return PSTR("OpMediaContrlReply");

    case OpTimeCode:
      return PSTR("OpTimeCode");

    case OpTimeSync:
      return PSTR("OpTimeSync");

    case OpTrigger:
      return PSTR("OpTrigger");

    case OpDirectory:
      return PSTR("OpDirectory");

    case OpDirectoryReply:
      return PSTR("OpDirectoryReply");

    default:
      return PSTR("UNKNOWN OPCODE");
  }
}

static const PROGMEM char* PriorityCodeToStr(uint8_t prioritycode)
{
  switch(prioritycode)
  {
    case DpLow:
      return PSTR("DpLow");
    case DpMed:
      return PSTR("DpMed");
    case DpHigh:
      return PSTR("DpHigh");
    case DpCritical:
      return PSTR("DpCritical");
    case DpVolatile:
      return PSTR("DpVolatile");
    default:
      return PSTR("UNKNOWN PRIORITY CODE");
  }
}

#endif

static const char g_artnetstr[] = "Art-Net";
static const uint8_t g_broadcastaddress[] = {255, 255, 255, 255};

CArtNet::CArtNet(CController& controller, uint8_t* buf, uint8_t* ip, uint8_t* mac) :
  m_controller(controller), m_transmitbuf(buf), m_ip(ip), m_mac(mac)
{
  m_portaddress = 0;
  m_sendpollreply = false;
  m_sendpollreplytime = 0;
  m_sendpollreplydelay = 0;
}

void CArtNet::Initialize()
{
  //ArtPollReply needs to be sent when the controller comes online
  SendPollReply();
}

void CArtNet::Process(uint32_t now)
{
  //if an ArtPollReply packet is scheduled, send it and clear the flag
  if (m_sendpollreply && now - m_sendpollreplytime >= m_sendpollreplydelay)
  {
    SendPollReply();
    m_sendpollreply = false;
  }
}

void CArtNet::HandlePacket(byte ip[4], uint16_t port, uint8_t* data, uint16_t len)
{
  //test if the first bytes are "Art-Net", including the null terminator
  if (len < 10 || strncmp((const char*)data, g_artnetstr, sizeof(g_artnetstr)) != 0)
  {
    DBGPRINT("Received packet with invalid Art-Net header of size %u from %u.%u.%u.%u\n", len, ip[0], ip[1], ip[2], ip[3]);
    return;
  }

  uint16_t opcode = *(uint16_t*)(data + 8);

  DBGPRINT("Received Art-Net packet of size %u from %u.%u.%u.%u\n", len, ip[0], ip[1], ip[2], ip[3]);
  DBGPRINT("Opcode: %u: %S\n", opcode, OpcodeToStr(opcode));

  if (opcode == OpPoll)
    HandlePoll(ip, port, data, len);
  else if (opcode == OpOutput)
    HandleOutput(data, len);
  else
    DBGPRINT("Unhandled packet with opcode %u:%S\n", opcode, OpcodeToStr(opcode));
}

void CArtNet::HandlePoll(byte ip[4], uint16_t port, uint8_t* data, uint16_t len)
{
  if (len < sizeof(SArtPoll))
  {
    DBGPRINT("Received OpPoll with invalid size %u\n", len);
    return;
  }

  SArtPoll* pollmsg = (SArtPoll*)data;

  DBGPRINT("Protocol version high:%u low:%u\n", pollmsg->ProtVerHi, pollmsg->ProtVerLow);
  DBGPRINT("Diagnostic messages are %S\n", pollmsg->TalkToMe.Unicast ? PSTR("unicast") : PSTR("broadcast"));
  DBGPRINT("%S me diagnostic messages\n", pollmsg->TalkToMe.SendDiag ? PSTR("Send") : PSTR("Do not send"));
  DBGPRINT("Send ArtPollReply %S\n", pollmsg->TalkToMe.SendPollOnChange ? PSTR("when node conditions change") :
           PSTR("in reponse to ArtPoll or ArtAddress only"));
  DBGPRINT("Priority code: %u:%S\n", pollmsg->Priority, PriorityCodeToStr(pollmsg->Priority));

  //data is valid
  m_controller.OnValidData();

  if (port == ARTNETPORT)
  {
    //schedule a send of an ArtPollReply packet, if not already done
    if (!m_sendpollreply)
    {
      m_sendpollreply = true;
      m_sendpollreplytime = millis();
    }
  }
  else
  {
    //send an artpollreply as unicast
    SendPollReply(ip);
  }
}

void CArtNet::HandleOutput(uint8_t* data, uint16_t len)
{
  if (len < sizeof(SArtDmx))
  {
    DBGPRINT("Received OpOutput with invalid size %u\n", len);
    return;
  }

  SArtDmx* dmxmsg = (SArtDmx*)data;

  uint16_t portaddress = ((uint16_t)dmxmsg->Net << 8) | ((uint16_t)dmxmsg->SubUni);
  if (portaddress != 0 && portaddress != m_portaddress)
  {
    DBGPRINT("Received dmx output for another universe %u, my universe is %u\n", portaddress, m_portaddress);
    return;
  }

  //data is valid
  m_controller.OnValidData();

  DBGPRINT("Received dmx output for my universe %u\n", portaddress);

  //check if the length specified in the art-net packet is valid
  //and is less or equal than the actual number of databytes sent
  uint16_t maxlength = min(512, len - (sizeof(SArtDmx) - sizeof(dmxmsg->Data)));
  uint16_t length = ((uint16_t)dmxmsg->LengthHi << 8) | (uint16_t)dmxmsg->Length;
  if (length > maxlength || length < 2)
  {
    DBGPRINT("Received dmx output with invalid length %u\n", length);
    length = maxlength;
  }
  DBGPRINT("Received %u dmx channels\n", (int)length);

  //pass dmx data buffer to the controller
  m_controller.OnDmxData(dmxmsg->Data, length);
}

void CArtNet::SendPollReply(uint8_t* ip /*= NULL*/)
{
  DBGPRINT("Sending PollReply\n");

  SArtPollReply* reply = (SArtPollReply*)m_transmitbuf;

  memcpy(reply->ID, g_artnetstr, sizeof(g_artnetstr));
  reply->OpCode = OpPollReply;
  memcpy(reply->IpAddress, m_ip, 4);
  reply->Port = ARTNETPORT;
  reply->VersInfoH = 5;
  reply->VersInfo = 57;
  reply->NetSwitch = (m_portaddress & 0x7F00) >> 8;
  reply->SubSwitch = (m_portaddress & 0xF0) >> 4;
  reply->OemHi = 0;
  reply->Oem = 0;
  reply->UbeaVersion = 0;
  reply->Status1.IndicatorState = 0;
  reply->Status1.PortAddressProgrammingAuthority = 0;
  reply->Status1.unused = 0;
  reply->Status1.BootedFromROM = 0;
  reply->Status1.CapableOfRemoteDeviceManagement = 0;
  reply->Status1.UBEAPresent = 0;
  reply->EstaManLo = 'L';
  reply->EstaManHi = 'O';
  strcpy((char*)reply->ShortName, "LED strip");
  strcpy((char*)reply->LongName, "LED strip controller for OHM 2013");
  memset(reply->NodeReport, 0, sizeof(reply->NodeReport));
  reply->NumPortsHi = 0;
  reply->NumPortsLo = 2;
  reply->PortTypes[0].CanOutputDataFromArtNet = 1;
  reply->PortTypes[0].CanInputDataToArtNet = 0;
  reply->PortTypes[0].Type = DMX512;
  reply->PortTypes[1].CanOutputDataFromArtNet = 1;
  reply->PortTypes[1].CanInputDataToArtNet = 0;
  reply->PortTypes[1].Type = DMX512;
  memset(reply->PortTypes + 2, 0, sizeof(reply->PortTypes[0]) * 2);
  memset(reply->GoodInput, 0, sizeof(reply->GoodInput));
  memset(reply->GoodOutput, 0, sizeof(reply->GoodOutput));
  memset(reply->SwIn, 0, sizeof(reply->SwIn));
  reply->SwOut[0] = 0;
  reply->SwOut[1] = m_portaddress & 15;
  reply->SwOut[2] = 0;
  reply->SwOut[3] = 0;
  reply->SwVideo = 0;
  memset(&reply->SwMacro, 0, sizeof(reply->SwMacro));
  memset(&reply->SwRemote, 0, sizeof(reply->SwRemote));
  memset(reply->Spare, 0, sizeof(reply->Spare));
  reply->Style = StNode;
  memcpy(reply->MAC, m_mac, sizeof(reply->MAC));
  memcpy(reply->BindIp, m_ip, 4);
  reply->BindIndex = 0;
  reply->Status2.unused = 0;
  reply->Status2.NodeSupports15BitPortAddress = 1;
  reply->Status2.NodeIsDHCPCapable = !STATIC;
  reply->Status2.NodesIPIsDHCPConfigured = !STATIC;
  reply->Status2.ProductSupportsWebBrowserConfiguration = 0;
  memset(reply->Filler, 0, sizeof(reply->Filler));
  
  //if ip is not set, send it to the broadcast address
  //if it is set, send it as unicast at art-net port minus one
  if (!ip)
    m_controller.Transmit(m_transmitbuf, sizeof(SArtPollReply), ARTNETPORT, g_broadcastaddress, ARTNETPORT);
  else
    m_controller.Transmit(m_transmitbuf, sizeof(SArtPollReply), ARTNETPORT - 1, ip, ARTNETPORT - 1);
}

