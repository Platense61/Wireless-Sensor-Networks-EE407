/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

//introduction ******^
//imports        ******v

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/csma-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/ipv4-global-routing-helper.h"

// Default Network Topology
//
//       10.1.1.0
// n0 -------------- n1   n2   n3   n4				
//    point-to-point  |    |    |    |
//                    ================
//                      LAN 10.1.2.0

//the network topology shown is simply a point to point network, with a bunch of LAN (CSMA)
// nodes attached on n1

using namespace ns3;					//to prevent having to write ns3 every ns3 function 

NS_LOG_COMPONENT_DEFINE ("SecondScriptExample");

int 
main (int argc, char *argv[])
{
  bool verbose = true;					//Sets up logging components
  uint32_t nCsma = 3;	

  CommandLine cmd;					//sets up how the program interacts with the command line
  cmd.AddValue ("nCsma", "Number of \"extra\" CSMA nodes/devices", nCsma);
  cmd.AddValue ("verbose", "Tell echo applications to log if true", verbose);

  cmd.Parse (argc,argv); 			// this parses command line arguments (int argc, char *argv[])
			   
  if (verbose)					//if logging is on
    {
      LogComponentEnable ("UdpEchoClientApplication", LOG_LEVEL_INFO);			//For logging
      LogComponentEnable ("UdpEchoServerApplication", LOG_LEVEL_INFO);
    }

  nCsma = nCsma == 0 ? 1 : nCsma; //ternary expression, used for setting nCsma to 0 or 1

  NodeContainer p2pNodes;								//Creating a container for notes
  p2pNodes.Create (2);									//Creating 2 nodes to connect via p2p link

  NodeContainer csmaNodes;								//Declaring a NodeContainer to hold the CSMA nodes
  csmaNodes.Add (p2pNodes.Get (1));							//attaches csma nodes to n1 node
  csmaNodes.Create (nCsma);								//Adds the first node to the CSMA container

//the following PointToPointHelper section is similar to first.cc
  PointToPointHelper pointToPoint;							//pointToPoint is used in order to set up a point to point 
											//link. This requires a lot of low-level work, but is  
											//automated by the PointToPointHelper class
											//a point to point link in real life is 2 cards, and a cable	

  pointToPoint.SetDeviceAttribute ("DataRate", StringValue ("5Mbps"));			//sets point to point link to 
									      		//use a data rate of 5Mbps

  pointToPoint.SetChannelAttribute ("Delay", StringValue ("2ms"));			//same as before, but with delay setting (2ms)

  NetDeviceContainer p2pDevices; 							//final stages in using pointToPointHelper
  p2pDevices = pointToPoint.Install (p2pNodes);

//the following CsmaHelper section diverges from first.cc
  CsmaHelper csma;									//similar to PointToPointHelper, but corresponds to CSMA
											//CSMA is the LAN network shown above

  csma.SetChannelAttribute ("DataRate", StringValue ("100Mbps"));			//like before, sets data rate to 100Mbps
										
  csma.SetChannelAttribute ("Delay", TimeValue (NanoSeconds (6560)));			//like before, sets delay (6560 ns)
 

  NetDeviceContainer csmaDevices;							//final stages in using CsmaHelper
  csmaDevices = csma.Install (csmaNodes);

  InternetStackHelper stack;								//used to install protocol stacks on our p2p 
											//and csma nodes these protocols are tcp, udp, ip, etc
											
  stack.Install (p2pNodes.Get (0));                 					//final steps in using InternetStackHelper
				                        				//with these, all the nodes are completely set up   
  stack.Install (csmaNodes);

  Ipv4AddressHelper address;								//assigning IP address to our point-to-point devices
  address.SetBase ("10.1.1.0", "255.255.255.0");	
  Ipv4InterfaceContainer p2pInterfaces;
  p2pInterfaces = address.Assign (p2pDevices);

  address.SetBase ("10.1.2.0", "255.255.255.0");					//assigning IP address to CSMA devices
  Ipv4InterfaceContainer csmaInterfaces;
  csmaInterfaces = address.Assign (csmaDevices);

  UdpEchoServerHelper echoServer (9);							//setting server port number

  ApplicationContainer serverApps = echoServer.Install (csmaNodes.Get (nCsma));
  serverApps.Start (Seconds (1.0));							//start time for Server
  serverApps.Stop (Seconds (10.0));							//stop time for Server

  UdpEchoClientHelper echoClient (csmaInterfaces.GetAddress (nCsma), 9);
  echoClient.SetAttribute ("MaxPackets", UintegerValue (1));				//Set attributes how many packets sent out

  echoClient.SetAttribute ("Interval", TimeValue (Seconds (1.0)));			//How often packets are sent
  echoClient.SetAttribute ("PacketSize", UintegerValue (1024));				//How big the packets are

  ApplicationContainer clientApps = echoClient.Install (p2pNodes.Get (0));		//Sets up routing
  clientApps.Start (Seconds (2.0));							//When the client node starts
  clientApps.Stop (Seconds (10.0));							//When the client node stops

  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();					//Creates table for all routing traffic and fills it out

  pointToPoint.EnablePcapAll ("second");						//Enables PCap files output
  csma.EnablePcap ("second", csmaDevices.Get (1), true);

  Simulator::Run ();									//Runs Simulation
  Simulator::Destroy ();								//Cleans up Simulation
  return 0;										//Returns 0 if program runs properly
}
