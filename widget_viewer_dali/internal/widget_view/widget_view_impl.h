#ifndef __DALI_WIDGET_VIEW_INTERNAL_WIDGET_VIEW_H__
#define __DALI_WIDGET_VIEW_INTERNAL_WIDGET_VIEW_H__

/*
 * Copyright (c) 2015 Samsung Electronics Co., Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

// INTERNAL INCLUDES
#include <public_api/widget_view/widget_view.h>

// EXTERNAL INCLUDES
#include <dali-toolkit/public-api/controls/control-impl.h>
#include <pepper-dali/public-api/object-view/object-view.h>

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
  static Dali::WidgetView::WidgetView New();

  /**
   * @copydoc Dali::WidgetView::WidgetView::GetPid
   */
  pid_t GetPid() const;

  /**
   * @copydoc Dali::WidgetView::WidgetView::GetTitle
   */
  std::string GetTitle() const;

  /**
   * @copydoc Dali::WidgetView::WidgetView::GetAppId
   */
  std::string GetAppId() const;

  void SetObjectView( Pepper::ObjectView objectView );

protected:

  /**
   * Construct a new WidgetView.
   */
  WidgetView();

  /**
   * A reference counted object may only be deleted by calling Unreference()
   */
  virtual ~WidgetView();

private:

  // Undefined
  WidgetView( const WidgetView& );

  // Undefined
  WidgetView& operator= ( const WidgetView& );

private:

  Pepper::ObjectView mObjectView;
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
