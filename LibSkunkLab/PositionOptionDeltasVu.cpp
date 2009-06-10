#include "StdAfx.h"
#include "PositionOptionDeltasVu.h"

#include "PositionOptionDeltasMacros.h"

#include "Log.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNCREATE(CPositionOptionDeltasVu, CListCtrl)

CPositionOptionDeltasVu::CPositionOptionDeltasVu() 
: CListCtrl()
{
  
}

CPositionOptionDeltasVu::~CPositionOptionDeltasVu(void) {
}

BEGIN_MESSAGE_MAP(CPositionOptionDeltasVu, CListCtrl)
  ON_WM_DESTROY( )  // may need reflection
  ON_WM_CLOSE( )    // may need reflection
  ON_WM_CREATE( )   // may need reflection
  ON_NOTIFY_REFLECT ( NM_CUSTOMDRAW, OnCustomDraw )
  ON_NOTIFY_REFLECT ( LVN_GETDISPINFO, OnGetDispInfo )
END_MESSAGE_MAP()


afx_msg int CPositionOptionDeltasVu::OnCreate( LPCREATESTRUCT lpCreateStruct ) {

  int i  = CListCtrl::OnCreate( lpCreateStruct );

  BOOL b = CListCtrl::SetItemCountEx( 0, LVSICF_NOINVALIDATEALL | LVSICF_NOSCROLL );

  int ix = 0;
  BOOST_PP_REPEAT( BOOST_PP_ARRAY_SIZE( COLHDR_DELTAS_ARRAY ), COLHDR_DELTAS_EMIT_InsertColumn, ix )
  // InsertColumn( ix++, "UndSym", LVCFMT_LEFT, 50 );

  return i;
}

afx_msg void CPositionOptionDeltasVu::OnDestroy( ) {
  CListCtrl::OnDestroy();
}

afx_msg void CPositionOptionDeltasVu::OnClose( ) { // isn't called
  CListCtrl::OnClose();  
}

afx_msg void CPositionOptionDeltasVu::OnCustomDraw( NMHDR* pNMHDR, LRESULT* pResult ) {
  int i[] = {CDDS_POSTERASE, CDDS_POSTPAINT,  CDIS_CHECKED,  LVCDI_ITEM, LVCDI_GROUP};
  NMLVCUSTOMDRAW* pLVCD = reinterpret_cast<NMLVCUSTOMDRAW*>( pNMHDR );
  LOG << "in CPositionOptionDeltasVu::OnCustomDraw " 
    << pLVCD->nmcd.dwDrawStage
    << ", " << pLVCD->dwItemType
    << ", " << pLVCD->iSubItem
    << ", " << pLVCD->nmcd.uItemState
    ;
  *pResult = CDRF_DODEFAULT;
}

/*
dwDrawStage
Current drawing stage. This value is one of the following. 
Global Values:
  CDDS_POSTERASE
After the erasing cycle is complete.
  CDDS_POSTPAINT
After the painting cycle is complete.
  CDDS_PREERASE
Before the erasing cycle begins.
  CDDS_PREPAINT
Before the painting cycle begins.
Item-specific Values:
  CDDS_ITEM
Indicates that the dwItemSpec, uItemState, and lItemlParam members are valid.
  CDDS_ITEMPOSTERASE
After an item has been erased.
  CDDS_ITEMPOSTPAINT
After an item has been drawn.
  CDDS_ITEMPREERASE
Before an item is erased.
  CDDS_ITEMPREPAINT
Before an item is drawn.
  CDDS_SUBITEM
*/

afx_msg void CPositionOptionDeltasVu::OnGetDispInfo( NMHDR* pNMHDR, LRESULT* pResult ) {
  //NMLVDISPINFO *pdi = (NMLVDISPINFO*)lParam
  NMLVDISPINFO* pDI = reinterpret_cast<NMLVDISPINFO*>( pNMHDR );
  //pDI->item.
  *pResult = 0;
}

void CPositionOptionDeltasVu::SetModel( CPositionOptionDeltasModel *pModel ) {
  assert( NULL != pModel );
  m_pModel = pModel;
  pModel->SetOnInstrumentAdded( MakeDelegate( this, &CPositionOptionDeltasVu::HandleInstrumentAdded ) );
}

void CPositionOptionDeltasVu::HandleInstrumentAdded( CPositionOptionDeltasModel::vDeltaRows_t::size_type cnt ) {
  BOOL b = CListCtrl::SetItemCountEx( cnt, LVSICF_NOINVALIDATEALL | LVSICF_NOSCROLL );
}

//void CPositionOptionDeltasVu::HandleRowUpdate( CPositionOptionDeltasModel::vDeltaRows_t::size_type ixRow ) {  // ix starts at 0
//}

void CPositionOptionDeltasVu::HandlePeriodicRefresh(CGeneratePeriodicRefresh *refresh ) {
}


/* Hidden columns in Report View still get LVN_GETDISPINFO called for them, 
even though the data may not ever get displayed. 
You can optimize expensive draw/lookup operations 
away by checking the RECT returned from ListView_GetSubItemRect() 
to make sure RECT.right - RECT.left > 0.
*/