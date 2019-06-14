/************************************************************************
 * Copyright(c) 2019, One Unified. All rights reserved.                 *
 * email: info@oneunified.net                                           *
 *                                                                      *
 * This file is provided as is WITHOUT ANY WARRANTY                     *
 *  without even the implied warranty of                                *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.                *
 *                                                                      *
 * This software may not be used nor distributed without proper license *
 * agreement.                                                           *
 *                                                                      *
 * See the file LICENSE.txt for redistribution information.             *
 ************************************************************************/

/* 
 * File:    Strangle.cpp
 * Author:  raymond@burkholder.net
 * Project: TFOptions
 * Created on May 25, 2019, 10:56 PM
 */

#include "Strangle.h"

namespace ou { // One Unified
namespace tf { // TradeFrame
namespace option { // options

using pInstrument_t = ou::tf::Instrument::pInstrument_t;
using pOption_t = Option::pOption_t;

Strangle::Strangle()
: Combo()
{
}

Strangle::Strangle( Strangle&& rhs )
: Combo( std::move( rhs ) )
{
}

Strangle::~Strangle() {
}

void Strangle::Tick( bool bInTrend, double dblPriceUnderlying, ptime dt ) {
  Combo::Tick( bInTrend, dblPriceUnderlying, dt ); // first or last in sequence?
//  if ( m_vLeg.empty() ) {
//    ChooseStrikes( dblPriceUnderlying );
//  }
}

// TODO: need to fix this if other legs present.  Need to limit to the active legs.
//   maybe vector of inactive legs
// NOTE: if volatility drops, then losses occur on premium
void Strangle::PlaceOrder( ou::tf::OrderSide::enumOrderSide side ) { 
  switch ( m_state ) {
    case State::Positions: // doesn't confirm both put/call are available
    case State::Watching:
      for ( Leg& leg: m_vLeg ) {
        leg.PlaceOrder( side, 1 );
      }
      m_state = State::Executing;
      break;
  }
}

// TODO: functional, three/four sections:
//   choose option type, and strike price
//   check if different from existing trackers
//   construct options, and place into tracker
//   retrieve options after validation, and reset validator

bool Strangle::ValidateSpread( ConstructionTools& tools, double price, size_t nDuration ) {

  double strikeOtmCall {};
  double strikeOtmPut {};

  m_SpreadValidation.SetLegCount( 2 );
  const std::string& sUnderlying( tools.m_pWatchUnderlying->GetInstrument()->GetInstrumentName() );

  double bStrikesFound( false );

  try {
    strikeOtmCall = tools.m_chains.Call_Otm( price );
    assert( 0.0 <= ( strikeOtmCall - price ) );
    if ( ( 0.20 * 0.50 ) > ( strikeOtmCall - price ) ) { // within edge of range
      strikeOtmCall = tools.m_chains.Call_Otm( strikeOtmCall ); // choose a further out strike
    }
    strikeOtmPut = tools.m_chains.Put_Otm( price );
    assert( 0.0 <= ( price - strikeOtmPut ) );
    if ( ( 0.20 * 0.50 ) > ( price - strikeOtmPut ) ) { // within edge of range
      strikeOtmPut = tools.m_chains.Put_Otm( strikeOtmPut ); // choose a further out strike
    }
    assert( strikeOtmCall > strikeOtmPut );
    const double dblStrikeDelta = strikeOtmCall - strikeOtmPut;
    if ( m_dblMaxStrangleDelta > dblStrikeDelta ) {
      const double dblExclusionRange = 0.5 * ( ( 1.0 - 0.20 ) * dblStrikeDelta );  // enter in middle 20% only
      if (
        ( price < ( strikeOtmCall - dblExclusionRange ) ) &&
        ( price > ( strikeOtmPut  + dblExclusionRange ) )
      ) {
        bStrikesFound = true;
      }
    }
  }
  catch ( std::runtime_error& e ) {
    std::cout
      << sUnderlying
      << " found no strike for mid-point " << price
      << " expiry " << tools.m_dateExpiry
//        << " for quote " << m_QuoteUnderlyingLatest.DateTime().date()
      << " [" << e.what() << "]"
      << std::endl;
    throw e;
  }

  bool bBuildOptions( false );

  if ( bStrikesFound ) {
    if ( !m_SpreadValidation.IsActive() ) {
      bBuildOptions = true;
    }
    else {
      if ( ( strikeOtmCall != boost::dynamic_pointer_cast<ou::tf::option::Option>( m_SpreadValidation.GetWatch( 0 ) )->GetStrike() )
        || ( strikeOtmPut  != boost::dynamic_pointer_cast<ou::tf::option::Option>( m_SpreadValidation.GetWatch( 1 ) )->GetStrike() )
      ) {
        m_SpreadValidation.ResetOptions();
        bBuildOptions = true;
      }
    }
  }

  if ( bBuildOptions ) {
    std::cout
      << sUnderlying
      << ": strangle -> quote=" << price
      << ",otm call=" << strikeOtmCall
      << ",otm put=" << strikeOtmPut
      << std::endl;
    pInstrument_t pInstrumentUnderlying = tools.m_pWatchUnderlying->GetInstrument();
    tools.m_fConstructOption( 
      tools.m_chains.GetIQFeedNameCall( strikeOtmCall),
      pInstrumentUnderlying,
      [this]( pOption_t pOptionCall ){
        m_SpreadValidation.SetWatch( 0, pOptionCall );
      } );
    tools.m_fConstructOption( 
      tools.m_chains.GetIQFeedNamePut( strikeOtmPut),
      pInstrumentUnderlying,
      [this]( pOption_t pOptionPut ){
        m_SpreadValidation.SetWatch( 1, pOptionPut );
      } );
  } // bBuildOptions

  bool bValidated( false );
  if ( m_SpreadValidation.IsActive() ) {
    bValidated = m_SpreadValidation.Validate( nDuration );
  }

  return bValidated;
}

Strangle::pOptionPair_t Strangle::ValidatedOptions() {
  assert( m_SpreadValidation.IsActive() );
  pOptionPair_t pair(
    boost::dynamic_pointer_cast<ou::tf::option::Option>( m_SpreadValidation.GetWatch( 0 ) ),
    boost::dynamic_pointer_cast<ou::tf::option::Option>( m_SpreadValidation.GetWatch( 1 ) )
    );
  m_SpreadValidation.ResetOptions();
  return pair;
}

// applicable when running a long strangle strategy
void Strangle::CloseItmLegForProfit( double price, EOrderSide defaultOrderSide, fBuildLeg_t&& f ) {
  for ( Leg& leg: m_vLeg ) {
    if ( leg.CloseItmForProfit( price ) ) {
      EOptionSide side = leg.GetPosition()->GetInstrument()->GetOptionSide(); // assumes an option
      f( m_pPortfolio->Id(), side, price,
        [this,defaultOrderSide](pPosition_t pPosition, pChartDataView_t pChartDataView, EColour colour ){
          AddPosition( pPosition, pChartDataView, colour );
          for ( Leg& leg: m_vLeg ) {
            if ( leg.GetPosition()->Id() == pPosition->Id() ) {
              leg.PlaceOrder( defaultOrderSide, 1 );
              break;
            }
          }
        } );
    }
  }
}

double Strangle::GetNet( double price ) {
  double dblNet {};
  double dblConstructedValue {};
  double strikeUpper {};
  double strikeLower {};
  double multiplier {};
  for ( Leg& leg: m_vLeg ) {
    dblNet += leg.GetNet( price );
    double dblLegConstructedValue = leg.ConstructedValue();
    std::cout << ",constructed@" << dblLegConstructedValue;
    dblConstructedValue += dblLegConstructedValue;
    std::cout << std::endl;
    double strike = leg.GetPosition()->GetInstrument()->GetStrike();
    multiplier = leg.GetPosition()->GetInstrument()->GetRow().nMultiplier;
    if ( 0.0 == strikeUpper ) {
      strikeUpper = strikeLower = strike;
    }
    else {
      if ( strike > strikeUpper ) strikeUpper = strike;
      else {
        if ( strike < strikeLower ) strikeLower = strike;
        else std::cout << "don't know what happened" << std::endl;
      }
    }
  }

  //double adjustment = dblConstructedValue / multiplier;
  double adjustment = dblConstructedValue;
  double lower = strikeLower - adjustment;
  double upper = strikeUpper + adjustment;
  double profit = adjustment;
  double profitTotal {};

  std::string status;
  if ( ( price > strikeLower ) && ( price < strikeUpper ) ) {
    status = "expires";
  }
  else {
    if ( ( price > lower ) && ( price < upper ) ) {
      status = "profit";
      if ( price <= strikeLower ) profit = price - lower;
      if ( price >= strikeUpper ) profit = upper - price;
    }
    else {
      status = "loss";
      if ( price <= lower ) profit = price - lower;
      if ( price >= upper ) profit = upper - price;
    }
  }

  profitTotal += profit;

  std::cout
    << "  constructed: " << adjustment
    << ",lowerBE: " << lower
    << ",upperBE: " << upper
    << "=>" << status
    << "@" << profit
    << std::endl;
  return profitTotal;
}

void Strangle::Init() {
}

} // namespace option
} // namespace tf
} // namespace ou
