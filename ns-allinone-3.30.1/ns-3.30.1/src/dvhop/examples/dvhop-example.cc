/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "ns3/dvhop-module.h"
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/mobility-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/wifi-module.h"
#include "ns3/netanim-module.h"
#include <iostream>
#include <cmath>
#include "ns3/udp-socket-factory.h"
#include "ns3/wifi-net-device.h"
#include "ns3/adhoc-wifi-mac.h"
#include "ns3/string.h"
#include "ns3/pointer.h"
#include "ns3/dvhop.h"
#include "ns3/node-list.h"
#include "ns3/names.h"
#include "ns3/ptr.h"
#include "ns3/ipv4-list-routing.h"
#include "ns3/node.h"
#include "stdlib.h"

#include "ns3/simulator.h"

#include "ns3/dvhop.h"

using namespace ns3;

/**
 * \brief Test script.
 *
 * This script creates 1-dimensional grid topology and then ping last node from the first one:
 *
 * [10.0.0.1] <-- step --> [10.0.0.2] <-- step --> [10.0.0.3] <-- step --> [10.0.0.4]
 *
 *
 */
class DVHopExample
{
public:
  DVHopExample ();
  /// Configure script parameters, \return true on successful configuration
  bool Configure (int argc, char **argv);
  /// Run simulation
  void Run ();
  /// Report results
  void Report (std::ostream & os);

private:
  ///\name parameters
  //\{
  /// Number of nodes
  uint32_t size;
  /// Distance between nodes, meters
  double step;
  /// Simulation time, seconds
  double totalTime;
  /// Write per-device PCAP traces if true
  bool pcap;
  /// Print routes if true
  bool printRoutes;

  uint32_t numNodes; //Number of nodes
  

  uint32_t numNodesToKill = 0;
  uint32_t beacons[3] = {1,7,14};
  //\}

  ///\name network
  //\{
  NodeContainer nodes;
  NetDeviceContainer devices;
  Ipv4InterfaceContainer interfaces;
  //\}

private:
  void CreateNodes ();
  void CreateDevices ();
  void InstallInternetStack ();
  void InstallApplications ();
  void CreateBeacons();
  void DeleteSomeNodes(int, uint32_t, uint32_t);
};

int main (int argc, char **argv)
{
  DVHopExample test;
  if (!test.Configure (argc, argv))
    NS_FATAL_ERROR ("Configuration failed. Aborted.");

  test.Run ();
  test.Report (std::cout);
  return 0;
}

//-----------------------------------------------------------------------------
DVHopExample::DVHopExample () :
  size (5),
  step (30),
  totalTime (10),
  //numNodes (20),
  pcap (true),
  printRoutes (false),
  numNodes (20)
{
}

bool
DVHopExample::Configure (int argc, char **argv)
{
  // Enable DVHop logs by default. Comment this if too noisy
  //LogComponentEnable("DVHopRoutingProtocol", LOG_LEVEL_ALL);

  SeedManager::SetSeed (12345);
  CommandLine cmd;

  cmd.AddValue ("pcap", "Write PCAP traces.", pcap);
  cmd.AddValue ("printRoutes", "Print routing table dumps.", printRoutes);
  cmd.AddValue ("size", "Number of nodes.", size);
  cmd.AddValue ("time", "Simulation time, s.", totalTime);
  cmd.AddValue ("step", "Grid step, m", step);

  cmd.Parse (argc, argv);
  return true;
}

void
DVHopExample::Run ()
{
//  Config::SetDefault ("ns3::WifiRemoteStationManager::RtsCtsThreshold", UintegerValue (1)); // enable rts cts all the time.
  CreateNodes ();
  CreateDevices ();
  InstallInternetStack ();

  CreateBeacons();

  std::cout << "Starting simulation for " << totalTime << " s ...\n";

  Simulator::Stop (Seconds (totalTime));

  AnimationInterface anim("animation.xml");
  //for(int i = 1; i <= 9; i+=2)
  //{
    srand(100);
    DVHopExample::DeleteSomeNodes(1, interfaces.Get((int)(rand() % (int)(numNodes))).second, numNodes);
  //}
  Simulator::Run ();
  Simulator::Destroy ();
}

void
DVHopExample::Report (std::ostream &)
{

}

    void
DVHopExample::CreateNodes ()
{
    srand(260);
    uint32_t nodesToKill[numNodesToKill];
    for(uint32_t i = 0; i < numNodesToKill; i++)
    {
        nodesToKill[i] = rand() % (int)(numNodes);
        for(int j = 0; j < 3; j++)
        {
            if(nodesToKill[i] == beacons[j] || nodesToKill[i] == 15)
            {
                if(i!=0)
                {
                    i--;//try this iteration of the for loop again
                }
                else
                {
                    nodesToKill[i] = 3;
                }
            }
        }
    }




  std::cout << "Creating " << (unsigned)numNodes<< " nodes " << step << " m apart.\n";
  nodes.Create (numNodes);
  // Name nodes
  for (uint32_t i = 0; i < numNodes; ++i)
    {
      std::ostringstream os;
      os << "node-" << i;
      std::cout << "Creating node: "<< os.str ()<< std::endl;
      Names::Add (os.str (), nodes.Get (i));
    }
  // Create static grid
  MobilityHelper mobility;
  mobility.SetPositionAllocator ("ns3::GridPositionAllocator",
                                 "MinX", DoubleValue (0.0),
                                 "MinY", DoubleValue (0.0),
                                 "DeltaX", DoubleValue (step),
                                 "DeltaY", DoubleValue (step),
                                 "GridWidth", UintegerValue (size),
                                 "LayoutType", StringValue ("RowFirst"));
  
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.Install (nodes);

  for(uint32_t i = 0; i < numNodes; i++)
  {
        for(uint32_t j = 0; j < numNodesToKill; j++)
        {
            if(i == nodesToKill[j])
            {
                Ptr<MobilityModel> nodeToYeet = nodes.Get(i)->GetObject<MobilityModel>();
                Vector crazyPosition = Vector(200, 0, 0);
                nodeToYeet->SetPosition(crazyPosition);
            }
        }
  }


}

void
DVHopExample::CreateBeacons ()
{
  Ptr<Ipv4RoutingProtocol> proto = nodes.Get (1)->GetObject<Ipv4>()->GetRoutingProtocol ();
  Ptr<dvhop::RoutingProtocol> dvhop = DynamicCast<dvhop::RoutingProtocol> (proto);
  dvhop->SetIsBeacon (true);
  //dvhop->SetPosition (123.42, 4534.452);
  dvhop->SetPosition(1 * step,0 * step);

  proto = nodes.Get (7)->GetObject<Ipv4>()->GetRoutingProtocol ();
  dvhop = DynamicCast<dvhop::RoutingProtocol> (proto);
  dvhop->SetIsBeacon (true);
  //dvhop->SetPosition (6663.42, 566.646);
  dvhop->SetPosition(2 * step,-1 * step);

  proto = nodes.Get (14)->GetObject<Ipv4>()->GetRoutingProtocol ();
  dvhop = DynamicCast<dvhop::RoutingProtocol> (proto);
  dvhop->SetIsBeacon (true);
  //dvhop->SetPosition (123.42, 9873.45);
  dvhop->SetPosition(4 * step,-2 * step);

}


void
DVHopExample::CreateDevices ()
{
  WifiMacHelper wifiMac = WifiMacHelper();
  wifiMac.SetType ("ns3::AdhocWifiMac");
  
  YansWifiPhyHelper wifiPhy = YansWifiPhyHelper::Default ();
  YansWifiChannelHelper wifiChannel = YansWifiChannelHelper::Default ();
  wifiPhy.SetChannel (wifiChannel.Create ());
  WifiHelper wifi = WifiHelper();
  wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager", "DataMode", StringValue ("OfdmRate6Mbps"), "RtsCtsThreshold", UintegerValue (0));
  devices = wifi.Install (wifiPhy, wifiMac, nodes);

  if (pcap)
    {
      wifiPhy.EnablePcapAll (std::string ("aodv"));
    }
}

void
DVHopExample::DeleteSomeNodes(int time, uint32_t interface, uint32_t totalNodes) 
{
    Ptr<Ipv4RoutingProtocol> proto = nodes.Get (0)->GetObject<Ipv4>()->GetRoutingProtocol ();
    Ptr<dvhop::RoutingProtocol> dvhop = DynamicCast<dvhop::RoutingProtocol> (proto);
    dvhop->DeleteSomeNodes(Seconds(time), interface, totalNodes);
    //Simulator::Schedule(Seconds(time), &DVHopExample::InstallInternetStack, this);
    //Simulator::Schedule(Seconds(time), 

    //InstallInternetStack();
    //CreateBeacons();
}


void
DVHopExample::InstallInternetStack ()
{
  DVHopHelper dvhop;
  // you can configure DVhop attributes here using aodv.Set(name, value)
  InternetStackHelper stack;
  stack.SetRoutingHelper (dvhop); // has effect on the next Install ()
  stack.Install (nodes);
  Ipv4AddressHelper address;
  address.SetBase ("10.0.0.0", "255.0.0.0");
  interfaces = address.Assign (devices);

  Ptr<OutputStreamWrapper> distStream = Create<OutputStreamWrapper>("dvhop.distances", std::ios::out);
  dvhop.PrintDistanceTableAllAt(Seconds(9), distStream);

  if (printRoutes)
    {
      Ptr<OutputStreamWrapper> routingStream = Create<OutputStreamWrapper> ("dvhop.routes", std::ios::out);
      dvhop.PrintRoutingTableAllAt (Seconds (8), routingStream);
    }
}
