/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
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
 * Author: Alexander Afanasyev <alexander.afanasyev@ucla.edu>
 *         Ilya Moiseenko <iliamo@cs.ucla.edu>
 */

#include "ndn-name-components.h"
#include <boost/foreach.hpp>
#include "ns3/log.h"

#include <iostream>

using namespace std;

NS_LOG_COMPONENT_DEFINE ("ndn.NameComponents");

namespace ns3 {
namespace ndn {

ATTRIBUTE_HELPER_CPP (NameComponents);

NameComponents::NameComponents (/* root */)
{
}

NameComponents::NameComponents (const std::list<boost::reference_wrapper<const std::string> > &components)
{
  BOOST_FOREACH (const boost::reference_wrapper<const std::string> &component, components)
    {
      Add (component.get ());
    }
}

NameComponents::NameComponents (const std::string &prefix)
{
  istringstream is (prefix);
  is >> *this;
}

NameComponents::NameComponents (const char *prefix)
{
  NS_ASSERT (prefix != 0);
  
  istringstream is (prefix);
  is >> *this;
}

const std::list<std::string> &
NameComponents::GetComponents () const
{
  return m_prefix;
}

std::string
NameComponents::GetLastComponent () const
{
  if (m_prefix.size () == 0)
    {
      return "";
    }

  return m_prefix.back ();
}

std::list<boost::reference_wrapper<const std::string> >
NameComponents::GetSubComponents (size_t num) const
{
  NS_ASSERT_MSG (0<=num && num<=m_prefix.size (), "Invalid number of subcomponents requested");
  
  std::list<boost::reference_wrapper<const std::string> > subComponents;
  std::list<std::string>::const_iterator component = m_prefix.begin();
  for (size_t i=0; i<num; i++, component++)
    {
      subComponents.push_back (boost::ref (*component));
    }
    
  return subComponents;
}

NameComponents
NameComponents::cut (size_t minusComponents) const
{
  NameComponents retval;
  std::list<std::string>::const_iterator component = m_prefix.begin (); 
  for (uint32_t i = 0; i < m_prefix.size () - minusComponents; i++, component++)
    {
      retval.Add (*component);
    }

  return retval;
}

void
NameComponents::Print (std::ostream &os) const
{
  for (const_iterator i=m_prefix.begin(); i!=m_prefix.end(); i++)
    {
      os << "/" << *i;
    }
  if (m_prefix.size ()==0) os << "/";
}
  
std::ostream &
operator << (std::ostream &os, const NameComponents &components)
{
  components.Print (os);
  return os;
}

std::istream &
operator >> (std::istream &is, NameComponents &components)
{
  istream_iterator<char> eos; // end of stream
  
  std::string component = "";
  istream_iterator<char> it (is);
  for (; it != eos; it++)
    {
      if (*it == '/')
        {
          if (component != "")
              components.Add (component);
          component = "";
        }
      else
        component.push_back (*it);
    }
  if (component != "")
      components.Add (component);

  is.clear (); 
  // NS_LOG_ERROR (components << ", bad: " << is.bad () <<", fail: " << is.fail ());
  
  return is;
}

} // ndn
} // ns3
