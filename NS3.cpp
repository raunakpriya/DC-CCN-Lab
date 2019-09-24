#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/traffic-control-module.h"
#include "ns3/flow-monitor-module.h"

// This simple example shows how to use TrafficControlHelper to install a
// QueueDisc on a device.
//
// The default QueueDisc is a pfifo_fast with a capacity of 1000 packets (as in
// Linux). However, in this example, we install a RedQueueDisc with a capacity
// of 10000 packets.
//
// Network topology
//
//       10.1.1.0
// n0 -------------- n1
//    point-to-point
//
// The output will consist of all the traced changes in the length of the RED
// internal queue and in the length of the netdevice queue:
//
//    DevicePacketsInQueue 0 to 1
//    TcPacketsInQueue 7 to 8
//    TcPacketsInQueue 8 to 9
//    DevicePacketsInQueue 1 to 0
//    TcPacketsInQueue 9 to 8
//
// plus some statistics collected at the network layer (by the flow monitor)
// and the application layer. Finally, the number of packets dropped by the
// queuing discipline, the number of packets dropped by the netdevice and
// the number of packets requeued by the queuing discipline are reported.
//
// If the size of the DropTail queue of the netdevice were increased from 1
// to a large number (e.g. 1000), one would observe that the number of dropped
// packets goes to zero, but the latency grows in an uncontrolled manner. This
// is the so-called bufferbloat problem, and illustrates the importance of
// having a small device queue, so that the standing queues build in the traffic
// control layer where they can be managed by advanced queue discs rather than
// in the device layer.

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("TrafficControlExample");

int main (int argc, char *argv[])
{
  double simulationTime = 5; //seconds
  std::string transportProt = "Udp";
  std::string socketType;
  CommandLine cmd;
  //cmd.Parse (argc, argv);

  if (transportProt.compare ("Tcp") == 0)
    {
      socketType = "ns3::TcpSocketFactory";
    }
  else
    {
      socketType = "ns3::UdpSocketFactory";
    }

  NodeContainer nodes;
  nodes.Create (3);

  PointToPointHelper pointToPoint;
  pointToPoint.SetDeviceAttribute ("DataRate", StringValue ("8Mbps"));
  pointToPoint.SetChannelAttribute ("Delay", StringValue ("16ms"));
  pointToPoint.SetQueue ("ns3::DropTailQueue", "Mode", StringValue ("QUEUE_MODE_PACKETS"), "MaxPackets", UintegerValue (1));

  NetDeviceContainer devices01=pointToPoint.Install (nodes.Get(0),nodes.Get(1));
  NetDeviceContainer devices12=pointToPoint.Install (nodes.Get(1),nodes.Get(2));

  InternetStackHelper stack;
  stack.Install (nodes);

  Ipv4AddressHelper address;
  address.SetBase ("10.1.1.0", "255.255.255.0");

  Ipv4InterfaceContainer interfaces01 = address.Assign (devices01);
  address.SetBase ("10.1.2.0", "255.255.255.0");
  Ipv4InterfaceContainer interfaces12 = address.Assign (devices12);
  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();
 
//Flow
  uint16_t port = 7;
  Address localAddress (InetSocketAddress (Ipv4Address::GetAny (), port));
 

  PacketSinkHelper packetSinkHelper (socketType, localAddress);
  ApplicationContainer sinkApp = packetSinkHelper.Install (nodes.Get (0));

  sinkApp.Start (Seconds (0.0));
  sinkApp.Stop (Seconds (simulationTime + 0.1));

  uint32_t payloadSize = 1448;
  Config::SetDefault ("ns3::TcpSocket::SegmentSize", UintegerValue (payloadSize));

  OnOffHelper onoff (socketType, Ipv4Address::GetAny ());
  onoff.SetAttribute ("OnTime",  StringValue ("ns3::ConstantRandomVariable[Constant=1]"));
  onoff.SetAttribute ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0]"));
  onoff.SetAttribute ("PacketSize", UintegerValue (payloadSize));
  onoff.SetAttribute ("DataRate", StringValue ("50Mbps")); //bit/s
  ApplicationContainer apps;

  AddressValue remoteAddress (InetSocketAddress (interfaces12.GetAddress (1), port));
  onoff.SetAttribute ("Remote", remoteAddress);
  apps.Add (onoff.Install (nodes.Get (0)));
  apps.Start (Seconds (1.0));
  apps.Stop (Seconds (simulationTime + 0.1));

  FlowMonitorHelper flowmon;
  Ptr<FlowMonitor> monitor = flowmon.InstallAll();

  Simulator::Stop (Seconds (simulationTime + 5));
  Simulator::Run ();

  Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier> (flowmon.GetClassifier ());
  std::map<FlowId, FlowMonitor::FlowStats> stats = monitor->GetFlowStats ();
  std::cout << std::endl << "*** Flow monitor statistics ***" << std::endl;
  std::cout << "  Tx Packets:   " << stats[1].txPackets << std::endl;
  std::cout << "  Tx Bytes:   " << stats[1].txBytes << std::endl;
  std::cout << "  Dropped Packets: " << stats[1].lostPackets<<std::endl;
Simulator::Destroy();
 

  return 0;
}
