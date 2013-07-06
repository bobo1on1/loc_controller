#ifndef ARTNET_H
#define ARTNET_H

#define ARTNETPORT 6454

enum Opcode
{
  OpPoll = 0x2000, //This is an ArtPoll packet, no other data is contained in this UDP packet.
  OpPollReply = 0x2100, //This is an ArtPollReply Packet. It contains device status information.
  OpDiagData = 0x2300, //Diagnostics and data logging packet.
  OpCommand = 0x2400, //Used to send text based parameter commands.
  OpOutput = 0x5000, //This is an ArtDmx data packet. It contains zero start code DMX512 information for a single Universe.
  OpNzs = 0x5100, //This is an ArtNzs data packet. It contains non-zero start code (except RDM) DMX512 information for a single Universe.
  OpAddress = 0x6000, //This is an ArtAddress packet. It contains remote programming information for a Node.
  OpInput = 0x7000, //This is an ArtInput packet. It contains enable – disable data for DMX inputs.
  OpTodRequest = 0x8000, //This is an ArtTodRequest packet. It is used to request a Table of Devices (ToD) for RDM discovery.
  OpTodData = 0x8100, //This is an ArtTodData packet. It is used to send a Table of Devices (ToD) for RDM discovery.
  OpTodControl = 0x8200, //This is an ArtTodControl packet. It is used to send RDM discovery control messages.
  OpRdm = 0x8300, //This is an ArtRdm packet. It is used to send all non discovery RDM messages.
  OpRdmSub = 0x8400, //This is an ArtRdmSub packet. It is used to send compressed, RDM Sub-Device data.
  OpVideoSetup = 0xa010, //This is an ArtVideoSetup packet. It contains video screen setup information for nodes that implement the extended video features.
  OpVideoPalette = 0xa020, //This is an ArtVideoPalette packet. It contains colour palette setup information for nodes that implement the extended video features.
  OpVideoData = 0xa040, //This is an ArtVideoData packet. It contains display data for nodes that implement the extended video features.
  OpMacMaster = 0xf000, //This is an ArtMacMaster packet. It is used to program the Node’s MAC address, Oem device type and ESTA manufacturer code. This is for factory initialisation of a Node. It is not to be used by applications.
  OpMacSlave = 0xf100, //This is an ArtMacSlave packet. It is returned by the node to acknowledge receipt of an ArtMacMaster packet. 
  OpFirmwareMaster = 0xf200, //This is an ArtFirmwareMaster packet. It is used to upload new firmware or firmware extensions to the Node.
  OpFirmwareReply = 0xf300, //This is an ArtFirmwareReply packet. It is returned by the node to acknowledge receipt of an ArtFirmwareMaster packet or ArtFileTnMaster packet.
  OpFileTnMaster = 0xf400, //Uploads user file to node.
  OpFileFnMaster = 0xf500, //Downloads user file from node.
  OpFileFnReply = 0xf600, //Node acknowledge for downloads.
  OpIpProg = 0xf800, //This is an ArtIpProg packet. It is used to reprogramme the IP, Mask and Port address of the Node
  OpIpProgReply = 0xf900, //This is an ArtIpProgReply packet. It is returned by the node to acknowledge receipt of an ArtIpProg packet.
  OpMedia = 0x9000, //This is an ArtMedia packet. It is Unicast by a Media Server and acted upon by a Controller.
  OpMediaPatch = 0x9100, //This is an ArtMediaPatch packet. It is Unicast by a Controller and acted upon by a Media Server.
  OpMediaControl = 0x9200, //This is an ArtMediaControl packet. It is Unicast by a Controller and acted upon by a Media Server.
  OpMediaContrlReply  = 0x9300, //This is an ArtMediaControlReply packet. It is Unicast by a Media Server and acted upon by a Controller.
  OpTimeCode = 0x9700, //This is an ArtTimeCode packet. It is used to transport time code over the network.
  OpTimeSync = 0x9800, //Used to synchronise real time date and clock
  OpTrigger = 0x9900, //Used to send trigger macros
  OpDirectory = 0x9a00, //Requests a node's file list
  OpDirectoryReply = 0x9b00, //Replies to OpDirectory with file list
};

enum PriorityCode
{
  DpLow = 0x10, //Low priority message.
  DpMed = 0x40, //Medium priority message.
  DpHigh = 0x80, //High priority message.
  DpCritical = 0xe0, //Critical priority message.
  DpVolatile = 0xff, //Volatile message. Messages of this type are displayed on a single line in the DMX-Workshop diagnostics display. All other types are displayed in a list box.
};

enum NodeReport
{
  RcDebug = 0x0000, //Booted in debug mode (Only used in development)
  RcPowerOk = 0x0001, //Power On Tests successful
  RcPowerFail = 0x0002, //Hardware tests failed at Power On
  RcSocketWr1 = 0x0003, //Last UDP from Node failed due to truncated length, Most likely caused by a collision.
  RcParseFail = 0x0004, //Unable to identify last UDP transmission. Check OpCode and packet length.
  RcUdpFail = 0x0005, //Unable to open Udp Socket in last transmission attempt
  RcShNameOk = 0x0006, //Confirms that Short Name programming via ArtAddress, was successful.
  RcLoNameOk = 0x0007, //Confirms that Long Name programming via ArtAddress, was successful.
  RcDmxError = 0x0008, //DMX512 receive errors detected.
  RcDmxUdpFull = 0x0009, //Ran out of internal DMX transmit buffers.
  RcDmxRxFull = 0x000a, //Ran out of internal DMX Rx buffers.
  RcSwitchErr = 0x000b, //Rx Universe switches conflict.
  RcConfigErr = 0x000c, //Product configuration does not match firmware.
  RcDmxShort = 0x000d, //DMX output short detected. See GoodOutput field.
  RcFirmwareFail = 0x000e, //Last attempt to upload new firmware failed.
};

enum Style
{
  StNode = 0x00, //A DMX to / from Art-Net device
  StController = 0x01, //A lighting console.
  StMedia = 0x02, //A Media Server.
  StRoute = 0x03, //A network routing device.
  StBackup = 0x04, //A backup device.
  StConfig = 0x05, //A configuration or diagnostic tool.
  StVisual = 0x06, //A visualiser.
};

enum Datatype
{
  DMX512       = 0,
  MIDI         = 1,
  AVAB         = 2,
  ColortranCMX = 3,
  ADB62dot5    = 4,
  ArtNet       = 5
};

struct STalkToMe
{
  uint8_t unused1 : 4;
  uint8_t Unicast : 1;
  uint8_t SendDiag : 1;
  uint8_t SendPollOnChange : 1;
  uint8_t unused2 : 1;
} __attribute__((packed));

struct SArtPoll
{
  uint8_t   ID[8];
  uint16_t  OpCode;
  uint8_t   ProtVerHi;
  uint8_t   ProtVerLow;
  STalkToMe TalkToMe;
  uint8_t   Priority;
} __attribute__((packed));

struct SStatus1
{
  uint8_t IndicatorState : 2;
  uint8_t PortAddressProgrammingAuthority : 2;
  uint8_t unused : 1;
  uint8_t BootedFromROM : 1;
  uint8_t CapableOfRemoteDeviceManagement : 1;
  uint8_t UBEAPresent : 1;
} __attribute__((packed));

struct SPortType
{
  uint8_t  CanOutputDataFromArtNet : 1;
  uint8_t  CanInputDataToArtNet : 1;
  Datatype Type : 6;
} __attribute__((packed));

struct SGoodInput
{
  uint8_t Datareceived : 1;
  uint8_t ChannelIncludesDMX512TestPackets : 1;
  uint8_t ChannelIncludesDMX512SIPs : 1;
  uint8_t ChannelIncludesDMX512TextPackets : 1;
  uint8_t InputIsDisabled : 1;
  uint8_t ReceiveErrorsDetected : 1;
  uint8_t unused : 2;
} __attribute__((packed));

struct SGoodOutput
{
  uint8_t DataIsBeingTransmitted : 1;
  uint8_t ChannelIncludesDMX512TestPackets : 1;
  uint8_t ChannelIncludesDMX512SIPs : 1;
  uint8_t ChannelIncludesDMX512TextPackets : 1;
  uint8_t OutputIsMergingArtNetData : 1;
  uint8_t DMXOutputShortDetectedOnPowerUp : 1;
  uint8_t MergeModeIsLTP : 1;
  uint8_t unused : 1;
} __attribute__((packed));

struct SSwMacro
{
  uint8_t Macro8Active : 1;
  uint8_t Macro7Active : 1;
  uint8_t Macro6Active : 1;
  uint8_t Macro5Active : 1;
  uint8_t Macro4Active : 1;
  uint8_t Macro3Active : 1;
  uint8_t Macro2Active : 1;
  uint8_t Macro1Active : 1;
} __attribute__((packed));

struct SSwRemote
{
  uint8_t Remote8Active : 1;
  uint8_t Remote7Active : 1;
  uint8_t Remote6Active : 1;
  uint8_t Remote5Active : 1;
  uint8_t Remote4Active : 1;
  uint8_t Remote3Active : 1;
  uint8_t Remote2Active : 1;
  uint8_t Remote1Active : 1;
} __attribute__((packed));

struct SStatus2
{
  uint8_t unused : 4;
  uint8_t NodeSupports15BitPortAddress : 1;
  uint8_t NodeIsDHCPCapable : 1;
  uint8_t NodesIPIsDHCPConfigured : 1;
  uint8_t ProductSupportsWebBrowserConfiguration : 1;
} __attribute__((packed));

struct SArtPollReply
{
  uint8_t     ID[8];
  uint16_t    OpCode;
  uint8_t     IpAddress[4];
  uint16_t    Port;
  uint8_t     VersInfoH;
  uint8_t     VersInfo;
  uint8_t     NetSwitch;
  uint8_t     SubSwitch;
  uint8_t     OemHi;
  uint8_t     Oem;
  uint8_t     UbeaVersion;
  SStatus1    Status1;
  uint8_t     EstaManLo;
  uint8_t     EstaManHi;
  uint8_t     ShortName[18];
  uint8_t     LongName[64];
  uint8_t     NodeReport[64];
  uint8_t     NumPortsHi;
  uint8_t     NumPortsLo;
  SPortType   PortTypes[4];
  SGoodInput  GoodInput[4];
  SGoodOutput GoodOutput[4];
  uint8_t     SwIn[4];
  uint8_t     SwOut[4];
  uint8_t     SwVideo;
  SSwMacro    SwMacro;
  SSwRemote   SwRemote;
  uint8_t     Spare[3];
  uint8_t     Style;
  uint8_t     MAC[6];
  uint8_t     BindIp[4];
  uint8_t     BindIndex;
  SStatus2    Status2;
  uint8_t     Filler[26];
} __attribute__((packed));

struct SArtDmx
{
  uint8_t     ID[8];
  uint16_t    OpCode;
  uint8_t     ProtVerHi;
  uint8_t     ProtVerLow;
  uint8_t     Sequence;
  uint8_t     Physical;
  uint8_t     SubUni;
  uint8_t     Net;
  uint8_t     LengthHi;
  uint8_t     Length;
  uint8_t     Data[2]; //minimum number of dmx bytes sent is 2
} __attribute__((packed));

class CController;

class CArtNet
{
  public:
    CArtNet(CController& controller, uint8_t* buf, uint8_t* ip, uint8_t* mac);
    void     Initialize();
    void     Process(uint32_t now);
    void     SetPortAddress(uint16_t portaddress) { m_portaddress = portaddress; }
    void     SetPollReplyDelay(uint16_t delay)    { m_sendpollreplydelay = delay;    }
    void     HandlePacket(byte ip[4], uint16_t port, uint8_t* data, uint16_t len);

  private:
    void HandlePoll(byte ip[4], uint16_t port, uint8_t* data, uint16_t len);
    void HandleOutput(uint8_t* data, uint16_t len);

    void SendPollReply(uint8_t* ip = NULL);

    CController& m_controller;
    uint8_t*     m_transmitbuf;
    uint8_t*     m_ip;
    uint8_t*     m_mac;
    uint16_t     m_portaddress;
    bool         m_sendpollreply;
    uint32_t     m_sendpollreplytime;
    uint16_t     m_sendpollreplydelay;
};

#endif //ARTNET_H
