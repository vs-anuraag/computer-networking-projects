/* Copy Right */
/* CSE 5344 */
/* NAME: ANURAAG VENKATAPURAM SREENIVAS */
/*Uta ID: 1001716458*/
/*
Topology
sender         receiver
 
  n4 -------------
                 |
 n2-----n1------n0
          |
     n3----

*/

#include "ns3/flow-monitor.h"
#include "ns3/flow-monitor-helper.h"
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/netanim-module.h"
using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("FirstScriptExample");

int
main (int argc, char *argv[])
{
        //generate nodes
        NodeContainer nodes;
        nodes.Create (5);

        //connect nodes
        NodeContainer n0n1;
        n0n1.Add(nodes.Get(0));
        n0n1.Add(nodes.Get(1));

        NodeContainer n1n2;
        n1n2.Add(nodes.Get(1));
        n1n2.Add(nodes.Get(2));

        NodeContainer n1n3;
        n1n3.Add(nodes.Get(1));
        n1n3.Add(nodes.Get(3));

        NodeContainer n0n4;
        n0n4.Add(nodes.Get(0));
        n0n4.Add(nodes.Get(4));


        //Link Capacity Setting

        PointToPointHelper pointToPoint1;
        pointToPoint1.SetDeviceAttribute ("DataRate", StringValue ("100Mbps"));
        pointToPoint1.SetChannelAttribute ("Delay", StringValue ("0.5ms"));

        NetDeviceContainer d0d1 = pointToPoint1.Install(n0n1);
        NetDeviceContainer d1d2 = pointToPoint1.Install(n1n2);
        NetDeviceContainer d1d3 = pointToPoint1.Install(n1n3);
        NetDeviceContainer d0d4 = pointToPoint1.Install(n0n4);


        InternetStackHelper stack;
        stack.Install (nodes);

        //assign address
        Ipv4AddressHelper address;
        Ipv4InterfaceContainer i0i1;
        Ipv4InterfaceContainer i1i2;
        Ipv4InterfaceContainer i1i3;
        Ipv4InterfaceContainer i0i4;


        address.SetBase ("10.1.1.0", "255.255.255.0");
        i0i1 = address.Assign(d0d1);

        address.SetBase ("10.1.2.0", "255.255.255.0");
        i1i2 = address.Assign(d1d2);

        address.SetBase ("10.1.3.0", "255.255.255.0");
        i1i3 = address.Assign(d1d3);

        address.SetBase ("10.1.4.0", "255.255.255.0");
        i0i4 = address.Assign(d0d4);



        //generate routing table
        Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

        //sink deploy
        uint16_t port = 9;
        PacketSinkHelper sink("ns3::TcpSocketFactory", InetSocketAddress(Ipv4Address::GetAny(), port));
        ApplicationContainer sinkApps = sink.Install(nodes.Get(0));

        sinkApps.Start(Seconds(0.0));
        sinkApps.Stop(Seconds(5.0));



        //flow 
        BulkSendHelper source("ns3::TcpSocketFactory", InetSocketAddress(Ipv4Address(i0i1.GetAddress(0)), port));
        source.SetAttribute("MaxBytes", UintegerValue(0));
        ApplicationContainer sourceApps = source.Install(nodes.Get(2));
        sourceApps.Start(Seconds(0.0));
        sourceApps.Stop(Seconds(5.0));

        ApplicationContainer sourceApps1 = source.Install(nodes.Get(3));
        sourceApps1.Start(Seconds(0.0));
        sourceApps1.Stop(Seconds(5.0));

        ApplicationContainer sourceApps2 = source.Install(nodes.Get(4));
        sourceApps2.Start(Seconds(0.0));
        sourceApps2.Stop(Seconds(5.0));

	
	Ptr<FlowMonitor> flowMonitor;
	FlowMonitorHelper flowHelper;
	flowMonitor = flowHelper.InstallAll();
        
        
        //animation generate
        AnimationInterface anim ("project1.xml");                   // output .xml name
	anim.SetConstantPosition(nodes.Get(0),5.0, 20.0);       // node number, x-value, y-value
	anim.SetConstantPosition(nodes.Get(1),35.0, 20.0);
	anim.SetConstantPosition(nodes.Get(2),65.0, 20.0);
        anim.SetConstantPosition(nodes.Get(3),65.0, 10.0);
        anim.SetConstantPosition(nodes.Get(4),65.0, 30.0);


  Simulator::Stop (Seconds (5));
  Simulator::Run ();

	flowMonitor->CheckForLostPackets();
    std::map<FlowId, FlowMonitor::FlowStats> stats = flowMonitor->GetFlowStats();
    uint64_t txPacketsum = 0;
    uint64_t rxPacketsum = 0;
    uint64_t DropPacketsum = 0;
    uint64_t LostPacketsum = 0;
    double Delaysum = 0;

  //  int j = 0;
    for (std::map<FlowId, FlowMonitor::FlowStats>::const_iterator i = stats.begin(); i != stats.end(); ++i) {
        txPacketsum = i->second.txPackets;
        rxPacketsum = i->second.rxPackets;
        LostPacketsum = i->second.lostPackets;
        DropPacketsum = i->second.packetsDropped.size();
        Delaysum = i->second.delaySum.GetSeconds();
	Time simulationtime = Simulator::Now ();
    
    NS_LOG_UNCOND(std::endl << "  SIMULATION STATISTICS");	
	NS_LOG_UNCOND("Flow " << i->first - 0);
	
    NS_LOG_UNCOND(" Tx Packets: " << txPacketsum);
    NS_LOG_UNCOND(" Rx Packets: " << rxPacketsum);
    NS_LOG_UNCOND(" Delay: " << Delaysum / txPacketsum);
    NS_LOG_UNCOND(" Lost Packets: " << LostPacketsum);
    NS_LOG_UNCOND(" Drop Packets: " << DropPacketsum);
    NS_LOG_UNCOND(" Rx Bytes:   " << i->second.rxBytes);
    NS_LOG_UNCOND("  Tx Bytes:   " << i->second.txBytes);
    NS_LOG_UNCOND("  Throughput:   " << i->second.rxBytes* 8.0 / simulationtime.GetSeconds() * 10.0 / 1000 / 1000 << " Mbytes");
    
    NS_LOG_UNCOND("  Packets Delivery Ratio: " << ((rxPacketsum * 100) / txPacketsum) << "%");
    NS_LOG_UNCOND("  Packets Lost Ratio: " << ((LostPacketsum * 100) / txPacketsum) << "%");
    }
	//flowMonitor->SerializeToXmlFile("flow.xml", true, true);	

  Simulator::Destroy ();
  return 0;
}