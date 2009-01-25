#pragma once

// class DT: primitive data type (self-assignment not checked)
// class GUI:  GUI type to be used for drawing

// 
// contains a single data point
// emits event when data point changes
// provides ability to draw data point in owner draw device context
// 

// to reduce code size, some common methods could be factored out into a 
//  base class and inherited

#include <sstream>

#include "FastDelegate.h"
using namespace fastdelegate;

#include <boost/utility.hpp>

#include "DrawingPrimitives.h"

template<class DT, class GUI> class CVisibleItem: boost::noncopyable {
public:
  CVisibleItem<DT,GUI>( GUI* pWin, 
    DT dtBlank, // initialized with this, GUI shows blank with this value
    CDrawingPrimitives* pPrimitives
    );
  ~CVisibleItem<DT,GUI>(void);

  DT& operator=( const DT& rhs );
  DT& Value( void ) { return m_dtItem; };

  typedef FastDelegate0<> OnUpdateHandler;
  void SetOnUpdate( OnUpdateHandler function ) {
    OnUpdate = function;
  }

  void Paint( CPaintDC* pdc );

protected:
  OnUpdateHandler OnUpdate;
private:
  GUI* m_pWin;  // destroyed elsewhere
  DT m_dtItem;
  DT m_dtBlank;
  CDrawingPrimitives* m_pPrimitives;
  COLORREF colourBackground;
  COLORREF colourForeground;
  //std::string m_sText;
  std::stringstream m_ss;
};

template<class DT, class GUI> CVisibleItem<DT,GUI>::CVisibleItem( 
  GUI *pWin, 
  DT dtBlank,
  CDrawingPrimitives* pPrimitives
  ) 
: m_pWin( pWin ), m_dtBlank( dtBlank ), m_dtItem( dtBlank ), m_pPrimitives( pPrimitives )
{
  // pre-constructed window on to which we'll draw our stuff
  assert( NULL != pWin );
}

template<class DT, class GUI> CVisibleItem<DT,GUI>::~CVisibleItem() {
}

template<class DT, class GUI> DT& CVisibleItem<DT,GUI>::operator=( const DT& rhs ) {
  if ( m_dtItem != rhs ) {
    m_dItem = rhs;
    m_ss.str( "" );
    m_ss << m_dtItem;
    if ( NULL != OnUpdate ) OnUpdate();
  }
  return m_dtItem;
}

template<class DT, class GUI> void Paint( CPaintDC* pdc ) {
  // should be called from within Win's OnPaint handler
  // commands as they are used in CVisibleElement
  // OnEraseBkgnd should be no-op'd
  CRect rc;
  pWin->GetClientRect( &rc );
  pdc->SelectObject( m_pPrimitives->Brush() );
  pdc->SelectObject( m_pPrimitives->Pen() );
  pdc->FillSolidRect( &rc, colourBackground );
  // possibly draw a focus rectangle:
  // pdc->Rectangle( &rc );
  pdc->SelectObject( m_pPrimitives->Font() );
  pdc->SetBkMode( TRANSPARENT );
  pdc->DrawText( m_ss.str().c_str(), m_ss.str().size(), &rc, DT_CENTER|DT_SINGLELINE|DT_VCENTER|DT_NOCLIP );
}