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

/**
 * This is a location agent for producer.
**/

#ifndef NDN_PRODUCER_AGENT_H
#define NDN_PRODUCER_AGENT_H

#include "ndn-app.h"

#include "ns3/ptr.h"
#include "ns3/ndn-name-components.h"
#include "ns3/ndn-content-object-header.h"

#include "ns3/nstime.h"
#include "ns3/random-variable.h"
#include "ns3/data-rate.h"
#include "../../internet/model/rtt-estimator.h"

#include <set>

#include <boost/multi_index_container.hpp>
#include <boost/multi_index/tag.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/member.hpp>

namespace ns3 {
namespace ndn {

/**
 * @brief A simple Interest-sink applia simple Interest-sink application
 *
 * A simple Interest-sink applia simple Interest-sink application,
 * which replying every incoming Interest with Data packet with a specified
 * size and name same as in Interest.cation, which replying every incoming Interest
 * with Data packet with a specified size and name same as in Interest.
 */
class ProducerAgent: public App
{
public: 
  typedef std::set< int32_t > seq_container;
  typedef seq_container::const_iterator interest_iterator;
  static TypeId
  GetTypeId (void);
        
  ProducerAgent ();

  // inherited from NdnApp
  void OnInterest (const Ptr<const InterestHeader> &interest, Ptr<Packet> packet);
  
  bool
  IsOpenCache () const;
  
  void
  SetForwardTime(Time forwardTimer);

  Time
  GetForwardTime () const;
  
  void
  SetCacheInterest (bool value);

  bool
  IsCachedInterest () const;

protected:
  // inherited from Application base class.
  virtual void
  StartApplication ();    // Called at time specified by Start

  virtual void
  StopApplication ();     // Called at time specified by Stop

  UniformVariable m_rand; ///< @brief nonce generator

  seq_container intr_container;

private:
  NameComponents m_prefix;
  NameComponents m_locatorName;
  int8_t m_isfromAgent;
  bool m_isopenCache;
  bool m_iscached;
  Time m_handoffTime;
  Time m_forwardtime;
  /*
  uint32_t m_virtualPayloadSize;
  
  uint32_t m_signatureBits;
  // ContentObjectHeader::SignedInfo m_signedInfo;

  SeqTimeoutsContainer m_seqTimeouts;       ///< \brief multi-index for the set of SeqTimeout structs
  SeqTimeoutsContainer m_seqLifetimes;
  */
};

} // namespace ndn
} // namespace ns3

#endif // NDN_PRODUCER_Agent_H
