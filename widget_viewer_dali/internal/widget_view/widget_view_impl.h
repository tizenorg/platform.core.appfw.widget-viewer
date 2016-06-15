#ifndef __DALI_WIDGET_VIEW_INTERNAL_WIDGET_VIEW_H__
#define __DALI_WIDGET_VIEW_INTERNAL_WIDGET_VIEW_H__

/*
 * Samsung API
 * Copyright (c) 2016 Samsung Electronics Co., Ltd.
 *
 * Licensed under the Flora License, Version 1.1 (the License);
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://floralicense.org/license/
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

// INTERNAL INCLUDES
#include <public_api/widget_view/widget_view.h>

// EXTERNAL INCLUDES
#include <dali-toolkit/public-api/controls/control-impl.h>
#include <dali-toolkit/public-api/controls/image-view/image-view.h>
#include <dali-toolkit/public-api/controls/text-controls/text-label.h>
#include <pepper-dali/public-api/object-view/object-view.h>
#include <bundle.h>

namespace Dali
{

namespace WidgetView
{

namespace Internal
{

class WidgetView : public Toolkit::Internal::Control
{
public:

  /**
   * @copydoc Dali::WidgetView::WidgetView::New
   */
  static Dali::WidgetView::WidgetView New( const std::string& widgetId, const std::string& contentInfo, int width, int height, double updatePeriod );

  /**
   * @copydoc Dali::WidgetView::WidgetView::PauseWidget
   */
  bool PauseWidget();

  /**
   * @copydoc Dali::WidgetView::WidgetView::ResumeWidget
   */
  bool ResumeWidget();

  /**
   * @copydoc Dali::WidgetView::WidgetView::GetWidgetId
   */
  const std::string& GetWidgetId() const;

  /**
   * @copydoc Dali::WidgetView::WidgetView::GetInstanceId
   */
  const std::string& GetInstanceId() const;

  /**
   * @copydoc Dali::WidgetView::WidgetView::GetContentInfo
   */
  const std::string& GetContentInfo();

  /**
   * @copydoc Dali::WidgetView::WidgetView::GetTitle
   */
  const std::string& GetTitle();

  /**
   * @copydoc Dali::WidgetView::WidgetView::GetUpdatePeriod
   */
  double GetUpdatePeriod() const;

  /**
   * @copydoc Dali::WidgetView::WidgetView::Show
   */
  void Show();

  /**
   * @copydoc Dali::WidgetView::WidgetView::Hide
   */
  void Hide();

  /**
   * @copydoc Dali::WidgetView::WidgetView::CancelTouchEvent
   */
  bool CancelTouchEvent();

  /**
   * @copydoc Dali::WidgetView::WidgetView::SetPreviewEnabled
   */
  void SetPreviewEnabled( bool enabled );

  /**
   * @copydoc Dali::WidgetView::WidgetView::GetPreviewEnabled
   */
  bool GetPreviewEnabled() const;

  /**
   * @copydoc Dali::WidgetView::WidgetView::SetStateTextEnabled
   */
  void SetStateTextEnabled( bool enabled );

  /**
   * @copydoc Dali::WidgetView::WidgetView::GetStateTextEnabled
   */
  bool GetStateTextEnabled() const;

  /**
   * @copydoc Dali::WidgetView::WidgetView::ActivateFaultedWidget
   */
  void ActivateFaultedWidget();

  /**
   * @copydoc Dali::WidgetView::WidgetView::IsWidgetFaulted
   */
  bool IsWidgetFaulted();

  /**
   * @copydoc Dali::WidgetView::WidgetView::SetPermanentDelete
   */
  void SetPermanentDelete( bool permanentDelete );

  void AddObjectView( Pepper::ObjectView objectView );
  void RemoveObjectView();

  void SendWidgetEvent( int event );

public: //Signals

  /**
   * @copydoc Dali::WidgetView::WidgetView::WidgetAddedSignal
   */
  Dali::WidgetView::WidgetView::WidgetViewSignalType& WidgetAddedSignal();

  /**
   * @copydoc Dali::WidgetView::WidgetView::WidgetDeletedSignal
   */
  Dali::WidgetView::WidgetView::WidgetViewSignalType& WidgetDeletedSignal();

  /**
   * @copydoc Dali::WidgetView::WidgetView::WidgetCreationAbortedSignal
   */
  Dali::WidgetView::WidgetView::WidgetViewSignalType& WidgetCreationAbortedSignal();

  /**
   * @copydoc Dali::WidgetView::WidgetView::WidgetResizedSignal
   */
  Dali::WidgetView::WidgetView::WidgetViewSignalType& WidgetResizedSignal();

  /**
   * @copydoc Dali::WidgetView::WidgetView::WidgetContentUpdatedSignal
   */
  Dali::WidgetView::WidgetView::WidgetViewSignalType& WidgetContentUpdatedSignal();

  /**
   * @copydoc Dali::WidgetView::WidgetView::WidgetExtraInfoUpdatedSignal
   */
  Dali::WidgetView::WidgetView::WidgetViewSignalType& WidgetExtraInfoUpdatedSignal();

  /**
   * @copydoc Dali::WidgetView::WidgetView::WidgetUpdatePeriodChangedSignal
   */
  Dali::WidgetView::WidgetView::WidgetViewSignalType& WidgetUpdatePeriodChangedSignal();

  /**
   * @copydoc Dali::WidgetView::WidgetView::WidgetFaultedSignal
   */
  Dali::WidgetView::WidgetView::WidgetViewSignalType& WidgetFaultedSignal();

protected:

  /**
   * Construct a new WidgetView.
   */
  WidgetView();

  /**
   * Construct a new WidgetView.
   */
  WidgetView( const std::string& widgetId, const std::string& contentInfo, int width, int height, double updatePeriod );

  /**
   * A reference counted object may only be deleted by calling Unreference()
   */
  virtual ~WidgetView();

private: // From Control

  /**
   * @copydoc Toolkit::Control::OnInitialize()
   */
  virtual void OnInitialize();

private: // From CustomActorImpl

  /**
   * @copydoc CustomActorImpl::OnSizeSet( const Vector3& targetSize )
   */
  virtual void OnSizeSet( const Vector3& targetSize );

private:

  // Undefined
  WidgetView( const WidgetView& );

  // Undefined
  WidgetView& operator= ( const WidgetView& );

private:

  Pepper::ObjectView mObjectView;     ///< Widget content
  Toolkit::ImageView mPreviewImage;   ///< Preview image
  Toolkit::TextLabel mStateText;      ///< State text

  std::string mWidgetId;
  std::string mInstanceId;
  std::string mContentInfo;
  std::string mTitle;

  bundle* mBundle;

  int mWidth;
  int mHeight;
  int mPid;
  double mUpdatePeriod;

  bool mPreviewEnabled;
  bool mStateTextEnabled;
  bool mPermanentDelete;

  // Signals
  Dali::WidgetView::WidgetView::WidgetViewSignalType mWidgetAddedSignal;
  Dali::WidgetView::WidgetView::WidgetViewSignalType mWidgetDeletedSignal;
  Dali::WidgetView::WidgetView::WidgetViewSignalType mWidgetCreationAbortedSignal;
  Dali::WidgetView::WidgetView::WidgetViewSignalType mWidgetResizedSignal;
  Dali::WidgetView::WidgetView::WidgetViewSignalType mWidgetContentUpdatedSignal;
  Dali::WidgetView::WidgetView::WidgetViewSignalType mWidgetExtraInfoUpdatedSignal;
  Dali::WidgetView::WidgetView::WidgetViewSignalType mWidgetUpdatePeriodChangedSignal;
  Dali::WidgetView::WidgetView::WidgetViewSignalType mWidgetFaultedSignal;
};

} // namespace Internal

// Helpers for public-api forwarding methods

inline Internal::WidgetView& GetImplementation( WidgetView& widgetView )
{
  DALI_ASSERT_ALWAYS( widgetView );

  Dali::RefObject& handle = widgetView.GetImplementation();

  return static_cast<Internal::WidgetView&>( handle );
}

inline const Internal::WidgetView& GetImplementation( const WidgetView& widgetView )
{
  DALI_ASSERT_ALWAYS( widgetView );

  const Dali::RefObject& handle = widgetView.GetImplementation();

  return static_cast<const Internal::WidgetView&>( handle );
}

} // namespace WidgetView

} // namespace Dali

#endif // __DALI_WIDGET_VIEW_INTERNAL_WIDGET_VIEW_H__
