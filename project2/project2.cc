/* copyright */
/* cse 5344 */
/* Anuraag venkatapuram sreenivas */
/* 1001716458 */

#include <string>
#include <fstream>
#include <cstdlib>
#include <map>
#include "ns3/core-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/internet-module.h"
#include "ns3/applications-module.h"
#include "ns3/network-module.h"
#include "ns3/packet-sink.h"
#include "ns3/flow-monitor-module.h"
#include "src/internet/model/mp-tcp-socket-base.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("Project2");

Ptr<PacketSink> sink_Data;
Ptr<PacketSink> sink_Data2;

uint64_t last_Total_Rx [1000] ;
uint64_t last_Total_Tx [1000] ;
double totalthroughput = 0;
Ptr<FlowMonitor> monitor;
FlowMonitorHelper flowmon;
vector<pair<double, double> > dataRateTracer[1000];
vector<pair<double, double> > throughputTracer[1000];
GnuplotCollection gnu;

void averageThroughput()
{
        Time now = Simulator::Now ();
        monitor->CheckForLostPackets ();
        Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier> (flowmon.GetClassifier ());
        std::map<FlowId, FlowMonitor::FlowStats> stats = monitor->GetFlowStats ();
        for (std::map<FlowId, FlowMonitor::FlowStats>::const_iterator i = stats.begin (); i != stats.end (); ++i)
        {
                Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow (i->first);
                std::cout << "Flow " << i->first  << " (" << t.sourceAddress << " -> " << t.destinationAddress << ")\n";
                std::cout << "  Tx Packets: " << i->second.txPackets << "\n";
                std::cout << "  Tx Bytes:   " << i->second.txBytes << "\n";
                std::cout << "  TxOffered:  " << i->second.txBytes * 8.0 / (i->second.timeLastRxPacket.GetSeconds() - i->second.timeFirstTxPacket.GetSeconds()) / 1000 / 1000  << " Mbps\n";
                std::cout << "  Rx Packets: " << i->second.rxPackets << "\n";
                std::cout << "  Rx Bytes:   " << i->second.rxBytes << "\n";
                std::cout << "  Throughput: " << i->second.rxBytes * 8.0 / (i->second.timeLastRxPacket.GetSeconds() - i->second.timeFirstTxPacket.GetSeconds()) / 1000 / 1000  << " Mbps\n";
                totalthroughput = totalthroughput + ( i->second.rxBytes * 8.0 / (i->second.timeLastRxPacket.GetSeconds() - i->second.timeFirstTxPacket.GetSeconds()) / 1000 / 1000 ) ;
        }
        monitor->SerializeToXmlFile("stats.flowmon", false, false);
}



void CalculateThroughput()
{
        Time now = Simulator::Now ();


        monitor->CheckForLostPackets ();
        Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier> (flowmon.GetClassifier ());
        std::map<FlowId, FlowMonitor::FlowStats> stats = monitor->GetFlowStats ();
        for (std::map<FlowId, FlowMonitor::FlowStats>::const_iterator i = stats.begin (); i != stats.end (); ++i)
        {

                double cur = (i->second.txBytes - last_Total_Tx[i->first - 1]) * 8.0 / 1e6;

                last_Total_Tx[i->first - 1] = i->second.txBytes; 
                // you may need to modify the if i->first condition to capture the flows you want to monitor
                if ( ((i->first) == 1) || ((i->first) == 2)|| ((i->first) == 5))
                        dataRateTracer[i->first - 1].push_back(make_pair(now.GetSeconds(), cur));


                cur = (i->second.rxBytes - last_Total_Rx[i->first - 1]) * 4.0 / 1e6;
                last_Total_Rx[i->first - 1] = i->second.rxBytes;

                 if (((i->first) == 1) || ((i->first) == 2))
                        throughputTracer[i->first - 1].push_back(make_pair(now.GetSeconds(), cur));// you may need to modify the if i->first condition to capture the flows you want to monitor

        }
        monitor->SerializeToXmlFile("stats.flowmon", false, false);
        Simulator::Schedule (MilliSeconds (1000), &CalculateThroughput);
}

void GenerateDataRateTracer()
{
        Gnuplot throughputGraph;
        throughputGraph.AppendExtra(
                       "set terminal postscript eps enhanced color solid font 'Times-Bold,15'\n"
                       "set output \"Throughput.eps\"\n"
                       "set xlabel \"Time (s)\" offset 0,-1\n"
                       "set ylabel \"Throughput (Mbps)\" offset 0,0 \n"
                       "set grid\n"
                       "set lmargin 10.0\n"
                       "set rmargin 5.0\n"
                       "set key bmargin center horizontal Left reverse noenhanced autotitles columnhead nobox\n");
        throughputGraph.SetTitle("Throughput vs Time\\n\\n");
        for (uint16_t idx = 0; idx < 26; idx++)
        {      
                Gnuplot2dDataset dataSet;
                dataSet.SetStyle(Gnuplot2dDataset::LINES_POINTS);

                std::stringstream title;
                title << "Flow " << idx+1;

                dataSet.SetTitle(title.str());
                vector<pair<double, double> >::iterator it = throughputTracer[idx].begin();

                while (it != throughputTracer[idx].end())
                {
                        dataSet.Add(it->first, it->second);
                        it++;
                }
                if (throughputTracer[idx].size() > 0)
                        throughputGraph.AddDataset(dataSet);
        }

        Gnuplot dataRateGraph;
        dataRateGraph.AppendExtra(
                       "set terminal postscript eps enhanced color solid font 'Times-Bold,15'\n"
                       "set output \"DataRate.eps\"\n"
                       "set xlabel \"Time (s)\" offset 0,-1\n"
                       "set ylabel \"DataRate (Mbps)\" offset 0,0 \n"
                       "set grid\n"
                       "set lmargin 10.0\n"
                       "set rmargin 5.0\n"
                       "set key bmargin center horizontal Left reverse noenhanced autotitles columnhead nobox\n");
        dataRateGraph.SetTitle("DataRate vs Time\\n\\n");
        for (uint16_t idx = 0; idx < 26; idx++)
        {      
                Gnuplot2dDataset dataSet;
                dataSet.SetStyle(Gnuplot2dDataset::LINES_POINTS);

                std::stringstream title;
                title << "Flow " << idx+1;

                dataSet.SetTitle(title.str());
                vector<pair<double, double> >::iterator it = dataRateTracer[idx].begin();

                while (it != dataRateTracer[idx].end())
                {
                        dataSet.Add(it->first, it->second);
                        it++;
                }
                if (dataRateTracer[idx].size() > 0)
                        dataRateGraph.AddDataset(dataSet);
        }
        gnu.AddPlot(dataRateGraph);
        gnu.AddPlot(throughputGraph);
}
void GeneratePlotsOutput()
{
        stringstream oss;
        oss << "Semi_Coupled"; 
        string tmp = oss.str();
        oss.clear();

        Ptr<OutputStreamWrapper> stream = Create<OutputStreamWrapper>(tmp.c_str(), std::ios::out);
        ostream* os = stream->GetStream();
        gnu.GenerateOutput(*os);
}





int
main(int argc, char *argv[])
{

        LogComponentEnable("MpTcpSocketBase", LOG_INFO);

        Config::SetDefault("ns3::Ipv4GlobalRouting::FlowEcmpRouting", BooleanValue(true));
        Config::SetDefault("ns3::TcpSocket::SegmentSize", UintegerValue(536));
        Config::SetDefault("ns3::TcpSocket::DelAckCount", UintegerValue(0));
        Config::SetDefault("ns3::DropTailQueue::Mode", StringValue("QUEUE_MODE_PACKETS"));
        Config::SetDefault("ns3::DropTailQueue::MaxPackets", UintegerValue(100));
        Config::SetDefault("ns3::TcpL4Protocol::SocketType", TypeIdValue(MpTcpSocketBase::GetTypeId()));
        Config::SetDefault("ns3::MpTcpSocketBase::MaxSubflows", UintegerValue(8)); // Sink


//Algorithm selecting


       Config::SetDefault("ns3::MpTcpSocketBase::CongestionControl", StringValue("Semi_Coupled"));



        gnu.SetOutFile("allPlots.pdf");

//todo: Topology build up
        
         //generate nodes
        NodeContainer nodes;
        nodes.Create (4);

        //connect nodes
        NodeContainer n0n2;
        n0n2.Add(nodes.Get(0));
        n0n2.Add(nodes.Get(2));

        NodeContainer n1n2;
        n1n2.Add(nodes.Get(1));
        n1n2.Add(nodes.Get(2));

        NodeContainer n1n3;
        n1n3.Add(nodes.Get(1));
        n1n3.Add(nodes.Get(3));

        NodeContainer n2n3;
        n2n3.Add(nodes.Get(2));
        n2n3.Add(nodes.Get(3));

        //Link Capacity Setting

        PointToPointHelper pointToPoint1;
        pointToPoint1.SetDeviceAttribute ("DataRate", StringValue ("100Mbps"));
        pointToPoint1.SetChannelAttribute ("Delay", StringValue ("0.5ms"));

        NetDeviceContainer d0d2 = pointToPoint1.Install(n0n2);
        NetDeviceContainer d1d2 = pointToPoint1.Install(n1n2);
        NetDeviceContainer d1d3 = pointToPoint1.Install(n1n3);
        NetDeviceContainer d2d3 = pointToPoint1.Install(n2n3);

        InternetStackHelper stack;
        stack.Install (nodes);
  
        //assign address
        Ipv4AddressHelper address;
        Ipv4InterfaceContainer i0i2;
        Ipv4InterfaceContainer i1i2;
        Ipv4InterfaceContainer i1i3;
        Ipv4InterfaceContainer i2i3;


        address.SetBase ("10.0.1.0", "255.255.255.0");
        i0i2 = address.Assign(d0d2);

        address.SetBase ("10.1.2.0", "255.255.255.0");
        i1i2 = address.Assign(d1d2);

        address.SetBase ("10.1.3.0", "255.255.255.0");
        i1i3 = address.Assign(d1d3);

        address.SetBase ("10.0.4.0", "255.255.255.0");
        i2i3 = address.Assign(d2d3);

        Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

//example of how to set up mptcp sender and sink application
//todo: set the applications correct to your topology
        uint16_t port = 9;
        double start =0.0;
        double end =30.0;
        MpTcpPacketSinkHelper sink("ns3::TcpSocketFactory", InetSocketAddress(Ipv4Address::GetAny(), port));
        ApplicationContainer sinkApps = sink.Install(nodes.Get(3));

        sink_Data = StaticCast<PacketSink> (sinkApps.Get(0));
        sinkApps.Start(Seconds(0.0));
        sinkApps.Stop(Seconds(end));

  
        MpTcpBulkSendHelper source("ns3::TcpSocketFactory", InetSocketAddress(Ipv4Address(i1i3.GetAddress(1)), port));
        source.SetAttribute("MaxBytes", UintegerValue(0));
        ApplicationContainer sourceApps = source.Install(nodes.Get(1));
        sourceApps.Start(Seconds(0.0));
        sourceApps.Stop(Seconds(end));

        MpTcpBulkSendHelper source1("ns3::TcpSocketFactory", InetSocketAddress(Ipv4Address(i2i3.GetAddress(1)), port));
        source1.SetAttribute("MaxBytes", UintegerValue(0));
        ApplicationContainer sourceApps1 = source1.Install(nodes.Get(0));
        sourceApps.Start(Seconds(0.0));
        sourceApps.Stop(Seconds(end));        
        

        Simulator::Schedule(Seconds(0.0), &CalculateThroughput);

        Simulator::Schedule(Seconds(end), &averageThroughput);
        monitor = flowmon.InstallAll ();
        NS_LOG_INFO ("Run Simulation.");


        Simulator::Stop(Seconds(end));
        Simulator::Run();

        Simulator::Destroy();
        GenerateDataRateTracer();
        GeneratePlotsOutput();
        NS_LOG_INFO ("Done.");
}
