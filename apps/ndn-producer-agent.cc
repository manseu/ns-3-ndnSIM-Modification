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
#include <boost/foreach.hpp>
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
    .AddAttribute ("HandoffTime","the handoff time for mobile producer",
                   StringValue ("0"),
                   MakeTimeAccessor  (&ProducerAgent::m_handoffTime),
                   MakeTimeChecker ())
    .AddAttribute ("IsOpenCache","the handoff time for mobile producer",
                   BooleanValue(false),
                   MakeBooleanAccessor(&ProducerAgent::m_isopenCache),
                   MakeBooleanChecker())
    ;
        
  return tid;
}
    
ProducerAgent::ProducerAgent ()
	:m_isfromAgent (1)
	,m_forwardtime (0)
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

  SetForwardTime(Simulator::Now () + m_handoffTime);
  
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

bool
ProducerAgent::IsOpenCache () const
{
  return m_isopenCache;
}

void
ProducerAgent::SetForwardTime(Time forwardTimer)
{
   m_forwardtime=forwardTimer;
}

Time
ProducerAgent::GetForwardTime () const
{
  return m_forwardtime;
}

void
ProducerAgent::SetCacheInterest (bool value)
{
  m_iscached=value;
}

bool
ProducerAgent::IsCachedInterest () const
{
  return m_iscached;
}

void
ProducerAgent::OnInterest (const Ptr<const InterestHeader> &interest, Ptr<Packet> origPacket)
{
  App::OnInterest (interest, origPacket); // tracing inside

  NS_LOG_FUNCTION (this << interest);

  if (!m_active) return;

  if((interest->GetName().cut(1) == m_prefix))
  {
    Time timeNow=Simulator::Now ();	  
    if ((m_locatorName.size()>0) && (timeNow >= GetForwardTime()))
    {
      InterestHeader cachedHeader;
      cachedHeader.SetNonce              (m_rand.GetValue ());
      cachedHeader.SetLocator            (Create<NameComponents> (m_locatorName));
 
      cachedHeader.SetInterestLifetime    (interest->GetInterestLifetime ());
      cachedHeader.SetChildSelector       (interest->IsEnabledChildSelector());
      if (interest->IsEnabledExclude()&&(interest->GetExclude().size ()>0))
      {
        cachedHeader.SetExclude (Create<NameComponents> (interest->GetExclude ()));
      }
      cachedHeader.SetAgent(m_isfromAgent);
      cachedHeader.SetMaxSuffixComponents (interest->GetMaxSuffixComponents());
      cachedHeader.SetMinSuffixComponents (interest->GetMinSuffixComponents ());
		  
	if(IsCachedInterest())
	{
          BOOST_FOREACH(const uint32_t t_seq, intr_container)
          {
            Ptr<NameComponents> nameWithSequence = Create<NameComponents> (m_prefix);
            (*nameWithSequence) (t_seq);
            cachedHeader.SetName(nameWithSequence);
            Ptr<Packet> packet = Create<Packet> ();
            packet->AddHeader (cachedHeader);
            NS_LOG_DEBUG ("Interest packet size: " << packet->GetSize ());

            m_protocolHandler (packet);
          }
          intr_container.clear();
          SetCacheInterest(false);
	}

      cachedHeader.SetName (Create<NameComponents> (interest->GetName ()));
      Ptr<Packet> packet = Create<Packet> ();
      packet->AddHeader (cachedHeader);
      NS_LOG_DEBUG ("Interest packet size: " << packet->GetSize ());
      m_protocolHandler (packet);

    }
    else
    {
      if(IsOpenCache())
      	{
        uint32_t seq = boost::lexical_cast<uint32_t> (interest->GetName ().GetComponents ().back ());
        intr_container.insert(seq);
        SetCacheInterest(true);
      	}
	return;      
    }
  }

}

} // namespace ndn
} // namespace ns3
