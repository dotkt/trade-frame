/************************************************************************
 * Copyright(c) 2009, One Unified. All rights reserved.                 *
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

#include "StdAfx.h"

#include <stdexcept>
#include <cassert>

#include <OUCommon/TimeSource.h>

#include "Order.h"

namespace ou { // One Unified
namespace tf { // TradeFrame

const std::string COrder::m_sTableName = "orders";

COrder::COrder(void) {
}

COrder::COrder( // market order
  CInstrument::pInstrument_cref instrument,
  OrderType::enumOrderType eOrderType,
  OrderSide::enumOrderSide eOrderSide, 
  unsigned long nOrderQuantity,
  idPosition_t idPosition,
  ptime dtOrderSubmitted
  ) 
:
  m_row( idPosition, eOrderType, eOrderSide, nOrderQuantity, dtOrderSubmitted ),
  m_pInstrument( instrument ),
  m_bOutsideRTH( false ),
  m_dblPriceXQuantity( 0 ), 
  m_nNextExecutionId ( 0 )
{
  ConstructOrder();
}

COrder::COrder( // limit or stop
  CInstrument::pInstrument_cref instrument,
  OrderType::enumOrderType eOrderType,
  OrderSide::enumOrderSide eOrderSide, 
  unsigned long nOrderQuantity,
  double dblPrice1,
  idPosition_t idPosition,
  ptime dtOrderSubmitted
  ) 
:
  m_row( idPosition, eOrderType, eOrderSide, nOrderQuantity, dblPrice1, dtOrderSubmitted ),
  m_pInstrument( instrument ),
  m_bOutsideRTH( false ),
  m_dblPriceXQuantity( 0 ),
  m_nNextExecutionId ( 0 )
{
  ConstructOrder();
}

COrder::COrder( // limit and stop
  CInstrument::pInstrument_cref instrument,
  OrderType::enumOrderType eOrderType,
  OrderSide::enumOrderSide eOrderSide, 
  unsigned long nOrderQuantity,
  double dblPrice1, double dblPrice2,
  idPosition_t idPosition,
  ptime dtOrderSubmitted
  ) 
:
  m_row( idPosition, eOrderType, eOrderSide, nOrderQuantity, dblPrice1, dblPrice2, dtOrderSubmitted ),
  m_pInstrument( instrument ), 
  m_bOutsideRTH( false ),
  m_dblPriceXQuantity( 0 ), 
  m_nNextExecutionId ( 0 )
{
  ConstructOrder();
}

COrder::COrder( const TableRowDef& row  ) 
: m_row( row ),
// need something for the pInstrument construction
  m_bOutsideRTH( false ),
  m_dblPriceXQuantity( 0 ), 
  m_nNextExecutionId ( 0 )
{
}

COrder::~COrder(void) {
}

void COrder::ConstructOrder() {
//  try {

  // need to do something with idInstrument, into and out of the database

  m_row.dtOrderCreated = ou::CTimeSource::Instance().Internal();
  assert( NULL != m_pInstrument.get() );
//  m_nOrderId = m_persistedorderid.GetNextOrderId();
//  }
//  catch (...) {
//    bOrderIdOk = false;
//    std::cout << "CIBTWS::PlaceOrder: Couldn't get the next order key." << std::endl;
//  }
}

void COrder::SetSendingToProvider() {
  assert( OrderStatus::Created == m_row.eOrderStatus );
  m_row.eOrderStatus = OrderStatus::SendingToProvider;
  m_row.dtOrderSubmitted = ou::CTimeSource::Instance().Internal();
}

OrderStatus::enumOrderStatus COrder::ReportExecution(const CExecution &exec) { 
  // need to worry about fill after cancel
  assert( exec.GetOrderSide() == m_row.eOrderSide );
  bool bOverDone = false;
  if ( 0 == m_row.nQuantityRemaining ) {
    // yes this has happened, 2008/07/09 vmw
    std::cout << "Order " << m_row.idOrder << " overfilled with +" << exec.GetSize() << std::endl;
    m_row.eOrderStatus = OrderStatus::OverFilled;
    bOverDone = true;
  }
  else {
    m_row.nQuantityRemaining -= exec.GetSize();
  }
  m_row.nQuantityFilled += exec.GetSize();
  if ( m_row.nQuantityFilled > m_row.nOrderQuantity ) {
    std:: cout << "Order " << m_row.idOrder << " overfilled with +" << exec.GetSize() << std::endl;
    bOverDone = true;
  }
  if ( !bOverDone ) {
    m_dblPriceXQuantity += exec.GetPrice() * exec.GetSize();
    m_row.dblAverageFillPrice = m_dblPriceXQuantity / m_row.nQuantityFilled;
    if ( 0 == m_row.nQuantityRemaining ) {
      m_row.eOrderStatus = OrderStatus::Filled;
      m_row.dtOrderClosed = ou::CTimeSource::Instance().Internal();
      OnOrderFilled( *this );
    }
    else {
      switch ( m_row.eOrderStatus ) {
      case OrderStatus::SendingToProvider:
      case OrderStatus::Submitted:
      case OrderStatus::Filling:
      case OrderStatus::PreSubmission:
        m_row.eOrderStatus = OrderStatus::Filling;
        break;
      case OrderStatus::Cancelled:
      case OrderStatus::CancelSubmitted:
      case OrderStatus::FillingDuringCancel:
      case OrderStatus::CancelledWithPartialFill:
        m_row.eOrderStatus = OrderStatus::FillingDuringCancel;
        break;
      case OrderStatus::OverFilled:
        break;
      default:
        std::cout << "COrder::ReportExecution " << static_cast<char>( m_row.eOrderStatus ) << std::endl;
        break;
      }
      OnPartialFill( *this );
    }
  }
  OnExecution( std::pair<const COrder&, const CExecution&>( *this, exec ) );
  return m_row.eOrderStatus;
}

void COrder::ActOnError(OrderErrors::enumOrderErrors eError) {
  switch( eError ) {
    case OrderErrors::Cancelled:
      m_row.eOrderStatus = OrderStatus::Cancelled;
      break;
    case OrderErrors::Rejected:
    case OrderErrors::InstrumentNotFound:
      m_row.eOrderStatus = OrderStatus::Rejected;
      break;
    case OrderErrors::NotCancellable:
      break;
  }
}

void COrder::SetCommission( double dblCommission ) { 
  m_row.dblCommission = dblCommission; 
  OnCommission( *this );
}

void COrder::SetOrderId( idOrder_t id ) {
  assert( 0 != id );
  assert( m_row.idOrder == 0 );
  m_row.idOrder = id;
}

} // namespace tf
} // namespace ou
