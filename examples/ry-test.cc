/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2012 University of California, Los Angeles
 *
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
 *
 * Author: Alexander Afanasyev <alexander.afanasyev@ucla.edu>
 */

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/ndnSIM-module.h"

#include "ns3/wifi-module.h"
#include "ns3/mobility-module.h"

using namespace ns3;

/***edited by ry***/
class PcapWriter
{
public:
  PcapWriter (const std::string &file)
  {
    PcapHelper helper;
    m_pcap = helper.CreateFile (file, std::ios::out, PcapHelper::DLT_PPP);
  }

  void
  TracePacket1 (Ptr<const Packet> packet)
  {
      NS_LOG_UNCOND ("receive: " << Simulator::Now ().GetSeconds ());
      m_pcap->Write (Simulator::Now (), packet);
  }
 
  void
  TracePacket2 (Ptr<const Packet> packet)
  {
      m_pcap->Write (Simulator::Now (), packet);
  }

private:
  Ptr<PcapFileWrapper> m_pcap;
};
/******************/

/**
 * This scenario simulates a very simple network topology:
 *
 *
 *      +----------+     1Mbps      +--------+     1Mbps      +----------+
 *      | consumer | <------------> | router | <------------> | producer |
 *      +----------+         10ms   +--------+          10ms  +----------+
 *
 *
 * Consumer requests data from producer with frequency 10 interests per second
 * (interests contain constantly increasing sequence number).
 *
 * For every received interest, producer replies with a data packet, containing
 * 1024 bytes of virtual payload.
 *
 * Simulation time is 20 seconds, unless --finish parameter is specified
 *
 * To run scenario and see what is happening, use the following command:
 *
 *     NS_LOG=ndn.Simple:ndn.Consumer ./waf --run=ndn-simple
 */

NS_LOG_COMPONENT_DEFINE ("ndn.Simple");

int 
main (int argc, char *argv[])
{
  Config::SetDefault ("ns3::PointToPointNetDevice::DataRate", StringValue ("1000Mbps"));
  Config::SetDefault ("ns3::PointToPointChannel::Delay", StringValue ("4ms"));
//  Config::SetDefault ("ns3::DropTailQueue::MaxPackets", StringValue ("20"));
//  Config::SetDefault ("ns3::ndn::Producer::SignatureBits", StringValue ("1"));
 
  NodeContainer p2pnodes;
  p2pnodes.Create (3);

  // Connecting nodes using two links
  PointToPointHelper p2p;
  p2p.Install (p2pnodes.Get (0), p2pnodes.Get (1));

  Config::SetDefault ("ns3::PointToPointChannel::Delay", StringValue ("10ms"));
  p2p.Install (p2pnodes.Get (1), p2pnodes.Get (2));

  NodeContainer sta;
  NodeContainer ap;
  NetDeviceContainer ApDev;
  NetDeviceContainer staDev;

//  aps.Create(2);
  sta.Create(1);
  ap.Add(p2pnodes.Get(0));

  MobilityHelper mobility;
  Ptr<ListPositionAllocator> positionAlloc;
  positionAlloc = CreateObject<ListPositionAllocator> ();
  positionAlloc->Add (Vector (0.0, 0.0, 0.0));
  mobility.SetPositionAllocator (positionAlloc);
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.Install (ap);

  Ptr<ListPositionAllocator> staPositionAlloc;
  staPositionAlloc = CreateObject<ListPositionAllocator> ();
  staPositionAlloc->Add (Vector (0.0, 20.0, 0.0));
  mobility.SetPositionAllocator (positionAlloc);
  mobility.SetMobilityModel ("ns3::ConstantVelocityMobilityModel");
  mobility.Install (sta);


  Ssid ssid = Ssid("CCNAP");
  YansWifiPhyHelper wifiPhy = YansWifiPhyHelper::Default ();
//  wifiPhy.SetPcapDataLinkType (YansWifiPhyHelper::DLT_IEEE802_11_RADIO);
  wifiPhy.SetPcapDataLinkType (YansWifiPhyHelper::DLT_IEEE802_11);

  WifiHelper wifi = WifiHelper::Default ();
  NqosWifiMacHelper wifiMac = NqosWifiMacHelper::Default ();
  YansWifiChannelHelper wifiChannel = YansWifiChannelHelper::Default ();
  wifiPhy.SetChannel (wifiChannel.Create ());

//  wifiMac.SetType ("ns3::ApWifiMac","Ssid", SsidValue (ssid),"BeaconGeneration", BooleanValue (true),"BeaconInterval", TimeValue (MilliSeconds (150)));
  wifiMac.SetType ("ns3::ApWifiMac","Ssid", SsidValue (ssid));
  ApDev = wifi.Install (wifiPhy, wifiMac, ap);
  
  wifiMac.SetType ("ns3::StaWifiMac",
	               "Ssid", SsidValue (ssid),
	               "ActiveProbing", BooleanValue (false));
  staDev = wifi.Install (wifiPhy, wifiMac, sta);
  
  // Install Ndn stack on all nodes
//  NS_LOG_INFO ("Installing Ndn stack");
  ndn::StackHelper ndnHelper;
  ndnHelper.SetDefaultRoutes (true);
  ndnHelper.InstallAll ();

  NS_LOG_INFO ("Installing Ndn applications");

  ndn::AppHelper consumerHelper ("ns3::ndn::ConsumerCbr");
//  Consumer will request /prefix/0, /prefix/1, ...
  consumerHelper.SetPrefix ("/prefix");
  consumerHelper.SetAttribute ("Frequency", StringValue ("500")); // 10 interests a second
  consumerHelper.SetAttribute ("StartSeq",StringValue ("95"));

  consumerHelper.SetAttribute ("RetxTimer",StringValue ("100ms"));
  ApplicationContainer consumers01 = consumerHelper.Install (sta.Get(0));
  consumers01.Start (Seconds (2.1));

 
  ndn::AppHelper producerHelper ("ns3::ndn::Producer");
  // Producer will reply to all requests starting with /prefix
  producerHelper.SetPrefix ("/prefix");
  producerHelper.SetAttribute ("PayloadSize", StringValue("1024"));
  producerHelper.Install (p2pnodes.Get (2));
  
  PcapWriter trace2 ("ry-test-3.pcap");
  PcapWriter trace3 ("ry-test-0.pcap");

  Config::ConnectWithoutContext ("/NodeList/3/$ns3::ndn::L3Protocol/FaceList/*/NdnRx",MakeCallback (&PcapWriter::TracePacket1, &trace2));
 Config::ConnectWithoutContext ("/NodeList/0/$ns3::ndn::L3Protocol/FaceList/*/NdnRx",MakeCallback (&PcapWriter::TracePacket2, &trace3));

  Simulator::Stop (Seconds (2.2));
  Simulator::Run ();
  Simulator::Destroy ();
//  NS_LOG_INFO ("Done!");
    
  return 0;
}
