
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
#include "ns3/multi-model-spectrum-channel.h"

#include <iomanip>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("WirelessInterferenceSimulation");

int main (int argc, char *argv[])
{   
    // Config parameters
    std::string wifiType{"ns3::YansWifiPhy"};
    uint16_t APnum{2};
    std::string errorModelType{"ns3::NistErrorRateModel"};

    // Command line arguments
    CommandLine cmd;
    cmd.AddValue("wifiType", "Type of wifi phy to use", wifiType);
    cmd.AddValue("APnum", "Number of APs", APnum);
    cmd.Parse(argc, argv);

    // Check if the wifi type is valid
    if (wifiType != "ns3::SpectrumWifiPhy" && wifiType != "ns3::YansWifiPhy")
    {
        std::cout << "Invalid wifi type" << std::endl;
        return 1;
    }

    // Check if the number of APs is valid
    if (APnum < 1 || APnum > 2)
    {
        std::cout << "Invalid number of APs" << std::endl;
        return 1;
    }    

    Time::SetResolution (Time::NS);

    //LogComponentEnable("UdpEchoClientApplication", LOG_LEVEL_INFO);
    //LogComponentEnable("UdpEchoServerApplication", LOG_LEVEL_INFO);

    // Nodes creation
    NodeContainer wifiApNodes;
    wifiApNodes.Create (APnum);
    NodeContainer wifiStaNodes;
    wifiStaNodes.Create (2);

    //CSMA nodes
    NodeContainer csmaNodes;
    csmaNodes.Add(wifiApNodes);

    CsmaHelper csma;
    csma.SetChannelAttribute("DataRate", StringValue("100Mbps"));
    csma.SetChannelAttribute("Delay", TimeValue(NanoSeconds(6560)));

    // Channel configuration
    YansWifiPhyHelper phy, phy2;
    SpectrumWifiPhyHelper spectrumPhy, spectrumPhy2;

    if (wifiType == "ns3::YansWifiPhy")
    {   YansWifiChannelHelper channel;
        channel.AddPropagationLoss("ns3::FriisPropagationLossModel",
                                   "Frequency",
                                   DoubleValue(5.180*1e6));
        channel.SetPropagationDelay("ns3::ConstantSpeedPropagationDelayModel");
        phy.SetChannel(channel.Create());
        phy.Set("ChannelSettings",
                    StringValue(std::string("{36, 0, BAND_5GHZ, 0}")));
        phy.Set("TxPowerStart", DoubleValue(1)); // dBm (1.26 mW)
        phy.Set("TxPowerEnd", DoubleValue(1));
        if(APnum == 2){
            phy2.SetChannel(channel.Create());
            phy2.Set("ChannelSettings",
                    StringValue(std::string("{38, 0, BAND_5GHZ, 0}")));
            phy2.Set("TxPowerStart", DoubleValue(1)); // dBm (1.26 mW)
            phy2.Set("TxPowerEnd", DoubleValue(1));
        }
    }
    else if (wifiType == "ns3::SpectrumWifiPhy")
    {   Ptr<MultiModelSpectrumChannel> spectrumChannel = CreateObject<MultiModelSpectrumChannel>();
        Ptr<FriisPropagationLossModel> lossModel = CreateObject<FriisPropagationLossModel>();
        lossModel->SetFrequency(5.180*1e6);
        spectrumChannel->AddPropagationLossModel(lossModel);

        Ptr<ConstantSpeedPropagationDelayModel> delayModel = CreateObject<ConstantSpeedPropagationDelayModel>();
        spectrumChannel->SetPropagationDelayModel(delayModel);
        spectrumPhy.SetChannel(spectrumChannel);
        spectrumPhy.Set("ChannelSettings",
                    StringValue(std::string("{36, 0, BAND_5GHZ, 0}")));
        spectrumPhy.SetErrorRateModel(errorModelType);
        spectrumPhy.Set("TxPowerStart", DoubleValue(1)); // dBm  (1.26 mW)
        spectrumPhy.Set("TxPowerEnd", DoubleValue(1));

        if(APnum == 2){
            spectrumPhy2.SetChannel(spectrumChannel);
            spectrumPhy2.Set("ChannelSettings",
                    StringValue(std::string("{38, 0, BAND_5GHZ, 0}")));
            spectrumPhy2.SetErrorRateModel(errorModelType);
            spectrumPhy2.Set("TxPowerStart", DoubleValue(1)); // dBm  (1.26 mW)
            spectrumPhy2.Set("TxPowerEnd", DoubleValue(1));
        }
    }

    // Standard WiFi 802.11n
    WifiHelper wifi;
    wifi.SetStandard (WIFI_STANDARD_80211n);

    // Mac Configuration
    WifiMacHelper mac;
    Ssid ssid1 = Ssid ("ns-3-ssid-1");
    Ssid ssid2 = Ssid ("ns-3-ssid-2");

    // Wifi installation
    NetDeviceContainer ap1Devices;
    NetDeviceContainer ap2Devices;
    NetDeviceContainer sta1Devices;
    NetDeviceContainer sta2Devices;

    if (wifiType == "ns3::YansWifiPhy"){
        mac.SetType("ns3::ApWifiMac",
                     "Ssid", SsidValue(ssid1));
        ap1Devices = wifi.Install(phy, mac, wifiApNodes.Get(0));
        mac.SetType("ns3::StaWifiMac",
                     "Ssid", SsidValue(ssid1));
        sta1Devices = wifi.Install(phy, mac, wifiStaNodes.Get(0));
        if(APnum == 2){
            mac.SetType("ns3::ApWifiMac",
                        "Ssid", SsidValue(ssid2));
            ap2Devices = wifi.Install(phy2, mac, wifiApNodes.Get(1));
            mac.SetType("ns3::StaWifiMac",
                        "Ssid", SsidValue(ssid2));
            sta2Devices = wifi.Install(phy2, mac, wifiStaNodes.Get(1));
        } else {
            sta2Devices = wifi.Install(phy, mac, wifiStaNodes.Get(1));
        }
    } else if (wifiType == "ns3::SpectrumWifiPhy"){
        mac.SetType("ns3::ApWifiMac",
                     "Ssid", SsidValue(ssid1));
        ap1Devices = wifi.Install(spectrumPhy, mac, wifiApNodes.Get(0));
        mac.SetType("ns3::StaWifiMac",
                     "Ssid", SsidValue(ssid1));
        sta1Devices = wifi.Install(spectrumPhy, mac, wifiStaNodes.Get(0));
        if(APnum == 2){
            mac.SetType("ns3::ApWifiMac",
                        "Ssid", SsidValue(ssid2));
            ap2Devices = wifi.Install(spectrumPhy2, mac, wifiApNodes.Get(1));
            mac.SetType("ns3::StaWifiMac",
                        "Ssid", SsidValue(ssid2));
            sta2Devices = wifi.Install(spectrumPhy2, mac, wifiStaNodes.Get(1));
        } else {
            sta2Devices = wifi.Install(spectrumPhy, mac, wifiStaNodes.Get(1));
        }
    }

    /*Ptr<WifiNetDevice> ap1NetDevice = DynamicCast<WifiNetDevice>(ap1Devices.Get(0));
    Ptr<WifiPhy> ap1Phy = DynamicCast<WifiPhy>(ap1NetDevice->GetPhy());
    ap1Phy->SetOperatingChannel(WifiPhy::ChannelTuple{1, 22, WIFI_PHY_BAND_2_4GHZ, 0});

    if(APnum == 2){
        Ptr<WifiNetDevice> ap2NetDevice = DynamicCast<WifiNetDevice>(ap2Devices.Get(0));
        Ptr<WifiPhy> ap2Phy = DynamicCast<WifiPhy>(ap2NetDevice->GetPhy());
        ap2Phy->SetOperatingChannel(WifiPhy::ChannelTuple{2, 22, WIFI_PHY_BAND_2_4GHZ, 0});
    }*/

    // Node mobility and position configuration
    MobilityHelper mobility;
    Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator>();

    positionAlloc->Add(Vector(0.0, 0.0, 0.0));
    positionAlloc->Add(Vector(5.0, 0.0, 0.0));
    positionAlloc->Add(Vector(0.0, 5.0, 0.0));
    positionAlloc->Add(Vector(5.0, 5.0, 0.0));

    mobility.SetPositionAllocator(positionAlloc);

    mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");

    mobility.Install(wifiApNodes);
    mobility.Install(wifiStaNodes);

    // Internet stack configuration
    InternetStackHelper stack;
    stack.Install(wifiApNodes);
    stack.Install(wifiStaNodes);

    // IP address configuration
    Ipv4AddressHelper address;

    // Wifi IP address
    address.SetBase ("192.168.1.0", "255.255.255.0");
    Ipv4InterfaceContainer ap1Interfaces = address.Assign (ap1Devices);
    Ipv4InterfaceContainer sta1Interfaces = address.Assign (sta1Devices);

    Ipv4InterfaceContainer ap2Interfaces;
    Ipv4InterfaceContainer sta2Interfaces;
    if (APnum == 2){
        address.SetBase ("192.168.2.0", "255.255.255.0");
        ap2Interfaces = address.Assign (ap2Devices);
        sta2Interfaces = address.Assign (sta2Devices);
    } else {
        sta2Interfaces = address.Assign (sta2Devices);
    }

    // CSMA IP address
    address.SetBase ("10.1.1.0", "255.255.255.0");
    Ipv4InterfaceContainer csmaInterfaces = address.Assign (csma.Install (csmaNodes));

    Ptr<Ipv4> ipv4A1 = wifiApNodes.Get(0)->GetObject<Ipv4>();
    Ptr<Ipv4> ipv4S1 = wifiStaNodes.Get(0)->GetObject<Ipv4>();
    Ptr<Ipv4> ipv4S2 = wifiStaNodes.Get(1)->GetObject<Ipv4>();
    Ptr<Ipv4> ipv4A2;
    if(APnum == 2){
        ipv4A2= wifiApNodes.Get(1)->GetObject<Ipv4>();
    }


    // Application configuration (UDP)
    uint16_t port = 9;
    UdpEchoServerHelper server(port);
    ApplicationContainer serverApp = server.Install(wifiApNodes.Get(0));
    serverApp.Start(Seconds(0.0));
    serverApp.Stop(Seconds(30.0));

    UdpEchoClientHelper client1 (ap1Interfaces.GetAddress(0), port);
    client1.SetAttribute("MaxPackets", UintegerValue(10000));
    client1.SetAttribute("Interval", TimeValue(Seconds(0.0001)));
    client1.SetAttribute("PacketSize", UintegerValue(1024));
    ApplicationContainer clientApp1 = client1.Install(wifiStaNodes.Get(0));
    clientApp1.Start(Seconds(0.0));
    clientApp1.Stop(Seconds(30.0));

    if (APnum == 2){
        UdpEchoServerHelper server2(port);
        ApplicationContainer serverApp2 = server2.Install(wifiApNodes.Get(1));
        serverApp2.Start(Seconds(0.0));
        serverApp2.Stop(Seconds(30.0));

        UdpEchoClientHelper client2 (ap2Interfaces.GetAddress(0), port);
        client2.SetAttribute("MaxPackets", UintegerValue(10000));
        client2.SetAttribute("Interval", TimeValue(Seconds(0.0001)));
        client2.SetAttribute("PacketSize", UintegerValue(1024));
        ApplicationContainer clientApp2 = client2.Install(wifiStaNodes.Get(1));
        clientApp2.Start(Seconds(0.0));
        clientApp2.Stop(Seconds(30.0));
    } else {
        UdpEchoClientHelper client2 (ap1Interfaces.GetAddress(0), port);
        client2.SetAttribute("MaxPackets", UintegerValue(10000));
        client2.SetAttribute("Interval", TimeValue(Seconds(0.0001)));
        client2.SetAttribute("PacketSize", UintegerValue(1024));
        ApplicationContainer clientApp2 = client2.Install(wifiStaNodes.Get(1));
        clientApp2.Start(Seconds(0.0));
        clientApp2.Stop(Seconds(30.0));
    }
    

    
    // Flow monitor
    Ptr<FlowMonitor> monitor;
    FlowMonitorHelper flowmon;
    monitor = flowmon.InstallAll();

    // Simulation start
    Simulator::Stop(Seconds(30.0));

    auto start = std::chrono::high_resolution_clock::now();


    Simulator::Run ();


    monitor->CheckForLostPackets ();

    if(wifiType == "ns3::SpectrumWifiPhy")
        if(APnum == 2)
            monitor->SerializeToXmlFile("scratch/ns-3-second-assignment/second-assignment-spectrum-2ap.xml", true, true);
        else
            monitor->SerializeToXmlFile("scratch/ns-3-second-assignment/second-assignment-spectrum-1ap.xml", true, true);
    else
        if(APnum == 2)
            monitor->SerializeToXmlFile("scratch/ns-3-second-assignment/second-assignment-yans-2ap.xml", true, true);
        else
            monitor->SerializeToXmlFile("scratch/ns-3-second-assignment/second-assignment-yans-1ap.xml", true, true);

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;
    std::cout << "Simulation time: " << elapsed.count() << "s" << std::endl;

    Simulator::Destroy();
    return 0;
}

    