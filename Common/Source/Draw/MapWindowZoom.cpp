/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id: MapWindowZoom.cpp,v 8.29 2011/01/06 02:07:52 root Exp root $
*/


#include "externs.h"
#include "Multimap.h"


MapWindow::Zoom::Zoom():
  _bMapScale (true),
  _inited(false),
  _autoZoom(false), _circleZoom(true), _bigZoom(false),
  _scale(0), _realscale(0),  _requestedScale(&_modeScale[SCALE_CRUISE]),
  _scaleOverDistanceModify(0),
  _resScaleOverDistanceModify(0)
{
  for(unsigned i=0; i<SCALE_NUM; i++)
    _modeScale[i] = 0;
}


/**
 * @brief Sets requested zoom scale for TARGET_PAN mode
 */
void MapWindow::Zoom::CalculateTargetPanZoom()
{
  // set scale exactly so that waypoint distance is the zoom factor across the screen
  *_requestedScale = LimitMapScale(TargetZoomDistance * DISTANCEMODIFY / 6.0);
}


/**
 * @brief Sets requested zoom scale for AUTO_ZOOM mode
 */
void MapWindow::Zoom::CalculateAutoZoom() {

  // Do not AutoZoom if we have CircleZoom enabled  and we are Circling
  if ( CircleZoom() && mode.Is(Mode::MODE_CIRCLING) )
    return;

  double wpd = DerivedDrawInfo.ZoomDistance;
  if ((wpd > 0) && wpd < AutoZoomThreshold) {
    double AutoZoomFactor = 4;
    if ((DisplayOrientation == NORTHTRACK) || DisplayOrientation == NORTHUP || DisplayOrientation == NORTHSMART ||
        ( (MapWindow::mode.autoNorthUP() || DisplayOrientation == NORTHCIRCLE ||
            DisplayOrientation == TARGETCIRCLE || DisplayOrientation == TARGETUP)
            && mode.Is(Mode::MODE_CIRCLING))) {
      AutoZoomFactor = 2.5;
    }
    _modeScale[SCALE_CRUISE] = LimitMapScale(wpd * DISTANCEMODIFY / AutoZoomFactor);
  } else {
    _modeScale[SCALE_CRUISE] = GetZoomInitValue(CruiseZoom) / 1.4;
  }

}

double MapWindow::Zoom::GetZoomInitValue(int parameter_number) const
{
  // Initial cruise/Climb zoom map scales. Parameter number equal to config dlg item index
  // Values are given in user units, km or mi what is selected.
  // These values used to select the best available mapscale from scalelist. See MapWindow::FillScaleListForEngineeringUnits(

  switch (Units::GetUserDistanceUnit()) {
    default:
        return ScaleListArrayMeters[parameter_number];
      break;
    case unStatuteMiles:
      return ScaleListArrayStatuteMiles[parameter_number];
      break;
    case unNauticalMiles:
      return ScaleListNauticalMiles[parameter_number];
      break;
  } //sw units


}

/**
 * @brief Resets Map Zoom to initial values
 */
void MapWindow::Zoom::Reset()
{
  const double SCALE_PANORAMA_INIT    = 10.0;

  _modeScale[SCALE_CRUISE]   = GetZoomInitValue(CruiseZoom);
  _modeScale[SCALE_CIRCLING] = GetZoomInitValue(ClimbZoom);
  _modeScale[SCALE_PANORAMA] = SCALE_PANORAMA_INIT;

  // Correct _modeScale[] values for internal use
  // You have to give values in user units (km,mi, what is selected), we need to divide it by 1.4
  // because of the size of the mapscale symbol
  _modeScale[SCALE_CRUISE]   /= 1.4;
  _modeScale[SCALE_CIRCLING] /= 1.4;
  _modeScale[SCALE_PANORAMA] /= 1.4;

  if(_autoZoom)
    _modeScale[SCALE_AUTO_ZOOM] = _modeScale[SCALE_CRUISE];

  _requestedScale = &_modeScale[SCALE_CRUISE];
  _scale = *_requestedScale;
  _scaleOverDistanceModify = *_requestedScale / DISTANCEMODIFY;
  _realscale = *_requestedScale/DISTANCEMODIFY/1000;

  _inited = true;
  SwitchMode();
}


/**
 * @brief Assigns proper zoom ratio for new Display Mode
 */
void MapWindow::Zoom::SwitchMode()
{
  if(!_inited)
    return;

  if((mode._mode & Mode::MODE_TARGET_PAN) && !(mode._lastMode & Mode::MODE_TARGET_PAN)) {
    // TARGET_PAN enabled
    _requestedScale = &_modeScale[SCALE_TARGET_PAN];
    CalculateTargetPanZoom();
  }
  else if(mode._mode & Mode::MODE_TARGET_PAN) {
    // do not change zoom for other mode changes while in TARGET_PAN mode
    return;
  }
  else if(mode._mode & Mode::MODE_PAN) {
    if(!(mode._lastMode & Mode::MODE_PAN))
      // PAN enabled - use current map scale if PAN enabled
      _modeScale[SCALE_PAN] = *_requestedScale;

    _requestedScale = &_modeScale[SCALE_PAN];

    // do not change zoom for other mode changes while in PAN mode
    return;
  }
  else if((mode._mode & Mode::MODE_PANORAMA) && !(mode._lastMode & Mode::MODE_PANORAMA)) {
    // PANORAMA enabled
    _requestedScale = &_modeScale[SCALE_PANORAMA];
  }
  else if(mode._mode & Mode::MODE_PANORAMA) {
    // do not change zoom for mode changes while in PANORAMA mode
    return;
  }
  else {
    if((mode._mode & Mode::MODE_CIRCLING) && _circleZoom) {
      _requestedScale = &_modeScale[SCALE_CIRCLING];
    }
    else {
      _requestedScale = &_modeScale[SCALE_CRUISE];

      if(_autoZoom)
        CalculateAutoZoom();
    }
  }
  *_requestedScale = LimitMapScale(*_requestedScale);

  RefreshMap();
}


/**
 * @brief Switches AutoZoom state
 *
 * @param vswitch Switch value:
 *               - -1 - switch current state
 *               -  0 - disable
 *               -  1 - enable
 */
void MapWindow::Zoom::EventAutoZoom(int vswitch)
{
  bool lastAutoZoom = _autoZoom;
  if(vswitch== -1)
    _autoZoom = !_autoZoom;
  else
    _autoZoom = vswitch;

  if(_autoZoom)
    // backup current zoom
    _modeScale[SCALE_AUTO_ZOOM] = _modeScale[SCALE_CRUISE];

  if(_autoZoom != lastAutoZoom)
    SwitchMode();
}


/**
 * @brief Sets provided value as current zoom
 *
 * @param value zoom ratio to set
 */
void MapWindow::Zoom::EventSetZoom(double value)
{
  double _lastRequestedScale = *_requestedScale;
  *_requestedScale = LimitMapScale(value);
  if(*_requestedScale != _lastRequestedScale) {
    RefreshMap();
  }
}


/**
 * @brief Modifies current zoom ratio
 *
 * @param vswitch Modifier value:
 */
void MapWindow::Zoom::EventScaleZoom(int vswitch)
{

  if (IsMultiMapNoMain() && !INPAN) {
	if (vswitch>0)
		LKevent=LKEVENT_UP;
	else
		LKevent=LKEVENT_DOWN;
	return;
  }

  // disable AutoZoom if possible
  if(_autoZoom &&
     mode.Special() == Mode::MODE_SPECIAL_NONE &&
     !(_circleZoom && mode.Is(Mode::MODE_CIRCLING))) {
    // Disable Auto Zoom only if not in Special or Circling Zoom
    DoStatusMessage(MsgToken(857)); // AutoZoom OFF
    _autoZoom = false;
  }

  // For best results, zooms should be multiples or roots of 2
  double value = *_requestedScale;
  if(ScaleListCount > 0) {
    value = FindMapScale(*_requestedScale);
    value = StepMapScale(-vswitch);
  }
  else {
    if (abs(vswitch)>=4) {
      if (vswitch==4)
        vswitch = 1;
      if (vswitch==-4)
        vswitch = -1;
    }
    if (vswitch==1) { // zoom in a little
      value /= 1.414;
    }
    if (vswitch== -1) { // zoom out a little
      value *= 1.414;
    }
    if (vswitch==2) { // zoom in a lot
      value /= 2.0;
    }
    if (vswitch== -2) { // zoom out a lot
      value *= 2.0;
    }
  }
  double _lastRequestedScale = *_requestedScale;
  *_requestedScale = LimitMapScale(value);
  if(*_requestedScale != _lastRequestedScale) {
    RefreshMap();
  }
}


/**
 * @brief Updates current map scale
 */
void MapWindow::Zoom::UpdateMapScale()
{

  if(mode.Is(Mode::MODE_TARGET_PAN)) {
    // update TARGET_PAN
    CalculateTargetPanZoom();
    if(_scale != *_requestedScale)
      ModifyMapScale();
    return;
  }


  if(_autoZoom &&
     mode.Special() == Mode::MODE_SPECIAL_NONE &&
     !(_circleZoom && mode.Is(Mode::MODE_CIRCLING))) {
    // Calculate Auto Zoom only if not in Special or Circling Zoom
    CalculateAutoZoom();
    if(_scale != *_requestedScale)
      ModifyMapScale();
    return;
  }

  // if there is user intervention in the scale
  if(_scale != *_requestedScale)
    ModifyMapScale();
}


/**
 * @brief Recalculates zoom parameters
 */



void MapWindow::Zoom::ModifyMapScale()
{
  // limit zoomed in so doesn't reach silly levels
  if(_bMapScale)
    *_requestedScale = LimitMapScale(*_requestedScale); // FIX VENTA remove limit
  _scaleOverDistanceModify = *_requestedScale / DISTANCEMODIFY;
  LKASSERT(_scaleOverDistanceModify!=0);
  _resScaleOverDistanceModify = GetMapResolutionFactor() / _scaleOverDistanceModify;
  _drawScale = _scaleOverDistanceModify;
  _drawScale = _drawScale / 111194;
  LKASSERT(_drawScale!=0);
  _drawScale = GetMapResolutionFactor() / _drawScale;
  _invDrawScale = 1.0 / _drawScale;
  _scale = *_requestedScale;
  _realscale = *_requestedScale/DISTANCEMODIFY/1000;
}


bool MapWindow::Zoom::GetInitMapScaleText(int init_parameter, TCHAR *out, size_t size) const
{
  double mapscale = GetZoomInitValue(init_parameter);
  //double mapscale = ScaleList[init_parameter];

  // Get nearest discrete value
  double ms = MapWindow::FindMapScale(mapscale/1.4)*1.4;
  return Units::FormatUserMapScale(NULL, Units::ToSysDistance(ms), out, size);
}


