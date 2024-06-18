
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/wifi-module.h"
#include "ns3/mobility-module.h"
#include "ns3/yans-wifi-helper.h"
#include "ns3/spectrum-wifi-helper.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/applications-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/csma-module.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("WirelessInterferenceSimulation");

/*void ConfigureChannel(YansWifiChannelHelper &channel, bool useSpectrum)
{
    if (useSpectrum)
    {
        SpectrumWifiChannelHelper spectrumChannel;
        Ptr<MultiModelSpectrumChannel> spectrum = spectrumChannel.Create ();
        channel.SetChannel (spectrum);
    }
    else
    {
        channel.SetPropagationDelay ("ns3::ConstantSpeedPropagationDelayModel");
        channel.AddPropagationLoss ("ns3::FriisPropagationLossModel");
    }
}*/

int main (int argc, char *argv[])
{   
    Time::SetResolution (Time::NS);

    LogComponentEnable("UdpEchoClientApplication", LOG_LEVEL_INFO);
    LogComponentEnable("UdpEchoServerApplication", LOG_LEVEL_INFO);

    // Creazione dei nodi
    NodeContainer wifiApNodes;
    wifiApNodes.Create (2);
    NodeContainer wifiStaNodes;
    wifiStaNodes.Create (2);

    // Configurazione del canale
    YansWifiChannelHelper channel;
    //ConfigureChannel(channel, useSpectrum);
    channel.SetPropagationDelay ("ns3::ConstantSpeedPropagationDelayModel");
    channel.AddPropagationLoss ("ns3::FriisPropagationLossModel");

    
    YansWifiPhyHelper phy;
    phy.SetChannel (channel.Create ());

    // Configurazione dello standard WiFi
    WifiHelper wifi;
    wifi.SetStandard (WIFI_STANDARD_80211b);

    // Configurazione della rete MAC
    WifiMacHelper mac;
    Ssid ssid1 = Ssid ("ns-3-ssid-1");
    Ssid ssid2 = Ssid ("ns-3-ssid-2");

    // Configurazione AP 1
    mac.SetType ("ns3::ApWifiMac",
                 "Ssid", SsidValue (ssid1));
    NetDeviceContainer ap1Devices = wifi.Install (phy, mac, wifiApNodes.Get (0));

    // Configurazione AP 2
    mac.SetType ("ns3::ApWifiMac",
                 "Ssid", SsidValue (ssid2));
    NetDeviceContainer ap2Devices = wifi.Install (phy, mac, wifiApNodes.Get (1));

    // Configurazione STA 1
    mac.SetType ("ns3::StaWifiMac",
                 "Ssid", SsidValue (ssid1), "ActiveProbing", BooleanValue(false));
    NetDeviceContainer sta1Devices = wifi.Install (phy, mac, wifiStaNodes.Get (0));

    // Configurazione STA 2
    mac.SetType ("ns3::StaWifiMac",
                 "Ssid", SsidValue (ssid2), "ActiveProbing", BooleanValue(false));
    NetDeviceContainer sta2Devices = wifi.Install (phy, mac, wifiStaNodes.Get (1));

    // Posizionamento dei nodi in modo che i due AP creino interferenza tra loro, mentre i due STA siano collegati a ciascun AP
    MobilityHelper mobility;

    mobility.SetPositionAllocator("ns3::GridPositionAllocator",
                                  "MinX",
                                  DoubleValue(0.0),
                                  "MinY",
                                  DoubleValue(0.0),
                                  "DeltaX",
                                  DoubleValue(5.0),
                                  "DeltaY",
                                  DoubleValue(10.0),
                                  "GridWidth",
                                  UintegerValue(3),
                                  "LayoutType",
                                  StringValue("RowFirst"));

    mobility.SetMobilityModel("ns3::RandomWalk2dMobilityModel",
                              "Bounds",
                              RectangleValue(Rectangle(-50, 50, -50, 50)));
    mobility.Install(wifiStaNodes);

    mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
    mobility.Install(wifiApNodes);
    
    // Creazione della connessione CSMA tra i due AP
    NodeContainer csmaNodes;
    csmaNodes.Add (wifiApNodes);

    CsmaHelper csma;
    csma.SetChannelAttribute ("DataRate", StringValue ("100Mbps"));
    csma.SetChannelAttribute ("Delay", TimeValue (NanoSeconds (6560)));
    NetDeviceContainer csmaDevices = csma.Install (csmaNodes);

    Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

    // Installazione dello stack Internet
    InternetStackHelper stack;
    stack.Install (wifiApNodes);
    stack.Install (wifiStaNodes);

    // Assegnazione degli indirizzi IP
    Ipv4AddressHelper address;

    // Assegnazione indirizzi per WiFi
    address.SetBase ("192.168.1.0", "255.255.255.0");
    Ipv4InterfaceContainer ap1Interfaces = address.Assign (ap1Devices);
    Ipv4InterfaceContainer sta1Interfaces = address.Assign (sta1Devices);

    address.SetBase ("192.168.2.0", "255.255.255.0");
    Ipv4InterfaceContainer ap2Interfaces = address.Assign (ap2Devices);
    Ipv4InterfaceContainer sta2Interfaces = address.Assign (sta2Devices);

    // Assegnazione indirizzi per CSMA
    address.SetBase ("10.1.1.0", "255.255.255.0");
    Ipv4InterfaceContainer csmaInterfaces = address.Assign (csmaDevices);

    // Installazione dell'applicazione di traffico sul primo AP e STA
    uint16_t port = 9;

    UdpEchoServerHelper server1 (port);
    ApplicationContainer serverApp1 = server1.Install (wifiApNodes.Get (0));
    serverApp1.Start (Seconds (1.0));
    serverApp1.Stop (Seconds (10.0));

    UdpEchoClientHelper client1 (ap1Interfaces.GetAddress (0), port);
    client1.SetAttribute ("MaxPackets", UintegerValue (320));
    client1.SetAttribute ("Interval", TimeValue (Seconds (0.05)));
    client1.SetAttribute ("PacketSize", UintegerValue (1024));

    ApplicationContainer clientApp1 = client1.Install (wifiStaNodes.Get (0));
    clientApp1.Start (Seconds (2.0));
    clientApp1.Stop (Seconds (10.0));

    // Installazione dell'applicazione di traffico sul secondo AP e STA
    UdpEchoServerHelper server2 (port);
    ApplicationContainer serverApp2 = server2.Install (wifiApNodes.Get (1));
    serverApp2.Start (Seconds (1.0));
    serverApp2.Stop (Seconds (10.0));

    UdpEchoClientHelper client2 (ap2Interfaces.GetAddress (0), port);
    client2.SetAttribute ("MaxPackets", UintegerValue (320));
    client2.SetAttribute ("Interval", TimeValue (Seconds (0.05)));
    client2.SetAttribute ("PacketSize", UintegerValue (1024));

    ApplicationContainer clientApp2 = client2.Install (wifiStaNodes.Get (1));
    clientApp2.Start (Seconds (2.0));
    clientApp2.Stop (Seconds (10.0));

    // Configurazione del monitoraggio del flusso
    FlowMonitorHelper flowmon;
    Ptr<FlowMonitor> monitor = flowmon.InstallAll ();

    Simulator::Stop (Seconds (10.0));

    // Misurazione del tempo di esecuzione, preferibile tramite flowmonitor
    /*
    clock_t start, end;
    start = clock();
    
    end = clock();
    double time_taken = double(end - start) / double(CLOCKS_PER_SEC);
    std::cout << "Simulation time: " << time_taken << " seconds" << std::endl;
    */
    Simulator::Run ();
    monitor->SerializeToXmlFile("scratch/sec.xml", true, true);

    Simulator::Destroy();
    return 0;
}

    