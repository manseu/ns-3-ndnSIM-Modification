/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2011 University of California, Los Angeles
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
 * Author: Ilya Moiseenko <iliamo@cs.ucla.edu>
 *         Alexander Afanasyev <alexander.afanasyev@ucla.edu>
 */
 /**
  * Modified by Tang, <tangjianqiang@bjtu.edu.cn>
  * National Engineering Lab for Next Generation Internet Interconnection Devices,
  * School of Electronics and Information Engineering,
  * Beijing Jiaotong Univeristy, Beijing 100044, China.
**/

#include "ndn-producer-agent.h"
#include "ns3/log.h"
#include "ns3/ndn-interest-header.h"
#include "ns3/ndn-content-object-header.h"
#include "ns3/string.h"
#include "ns3/uinteger.h"
#include "ns3/packet.h"
#include "ns3/simulator.h"

#include "ns3/ndn-app-face.h"
#include "ns3/ndn-fib.h"
// #include "../model/ndn-fib-impl.h"

#include <boost/ref.hpp>
#include <boost/lambda/lambda.hpp>
#include <boost/lambda/bind.hpp>
//add by tang
#include "ns3/ptr.h"
#include "ns3/callback.h"
#include "ns3/string.h"
#include "ns3/boolean.h"
#include "ns3/double.h"

#include <boost/lexical_cast.hpp>
#include "ns3/names.h"

#include "ns3/unused.h"
#include "../helper/ndn-encoding-helper.h"
#include "../helper/ndn-decoding-helper.h"

namespace ll = boost::lambda;

NS_LOG_COMPONENT_DEFINE ("ndn.ProducerAgent");

namespace ns3 {
namespace ndn {

NS_OBJECT_ENSURE_REGISTERED (ProducerAgent);
    
TypeId
ProducerAgent::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::ndn::ProducerAgent")
    .SetGroupName ("Ndn")
    .SetParent<App> ()
    .AddConstructor<ProducerAgent> ()
    .AddAttribute ("Prefix","Prefix, for which producer has the data",
                   StringValue ("/"),
                   MakeNameComponentsAccessor (&ProducerAgent::m_prefix),
                   MakeNameComponentsChecker ())
    .AddAttribute ("Locator","Locator, for which locator the mobile producer has attached",
                   StringValue ("/"),
                   MakeNameComponentsAccessor (&ProducerAgent::m_locatorName),
                   MakeNameComponentsChecker ())
    ;
        
  return tid;
}
    
ProducerAgent::ProducerAgent ()
	:m_isfromAgent (1)
{
  // NS_LOG_FUNCTION_NOARGS ();
}

// inherited from Application base class.
void
ProducerAgent::StartApplication ()
{
  NS_LOG_FUNCTION_NOARGS ();
  NS_ASSERT (GetNode ()->GetObject<Fib> () != 0);

  App::StartApplication ();

  NS_LOG_DEBUG ("NodeID: " << GetNode ()->GetId ());
  
  Ptr<Fib> fib = GetNode ()->GetObject<Fib> ();
  
  Ptr<fib::Entry> fibEntry = fib->Add (m_prefix, m_face, 0);

  fibEntry->UpdateStatus (m_face, fib::FaceMetric::NDN_FIB_GREEN);
  
  // // make face green, so it will be used primarily
  // StaticCast<fib::FibImpl> (fib)->modify (fibEntry,
  //                                        ll::bind (&fib::Entry::UpdateStatus,
  //                                                  ll::_1, m_face, fib::FaceMetric::NDN_FIB_GREEN));
}

void
ProducerAgent::StopApplication ()
{
  NS_LOG_FUNCTION_NOARGS ();
  NS_ASSERT (GetNode ()->GetObject<Fib> () != 0);

  App::StopApplication ();
}

void
ProducerAgent::OnInterest (const Ptr<const InterestHeader> &interest, Ptr<Packet> origPacket)
{
  App::OnInterest (interest, origPacket); // tracing inside

  NS_LOG_FUNCTION (this << interest);

  if (!m_active) return;

  //we can do something here, check the next node, or forward to the new locator.
  if(((interest->GetName().cut(1) == m_prefix))&&(m_locatorName.size()>0)) 
  {
      InterestHeader interestHeader;
      interestHeader.SetNonce              (m_rand.GetValue ());
      interestHeader.SetName               (Create<NameComponents> (interest->GetName ()));
      interestHeader.SetLocator            (Create<NameComponents> (m_locatorName));
 
      interestHeader.SetInterestLifetime    (interest->GetInterestLifetime ());

      interestHeader.SetChildSelector       (interest->IsEnabledChildSelector());
      if (interest->IsEnabledExclude()&&(interest->GetExclude().size ()>0))
      {
          interestHeader.SetExclude (Create<NameComponents> (interest->GetExclude ()));
      }

      interestHeader.SetAgent(m_isfromAgent);

      interestHeader.SetMaxSuffixComponents (interest->GetMaxSuffixComponents());
      interestHeader.SetMinSuffixComponents (interest->GetMinSuffixComponents ());
 

      Ptr<Packet> packet = Create<Packet> ();
      packet->AddHeader (interestHeader);
      NS_LOG_DEBUG ("Interest packet size: " << packet->GetSize ());

      m_protocolHandler (packet);

  }
  else
  {
      //NS_LOG_UNCOND("at producer agent: "<< interest->GetName() <<"\n");
      return;
  }

  
}

} // namespace ndn
} // namespace ns3
