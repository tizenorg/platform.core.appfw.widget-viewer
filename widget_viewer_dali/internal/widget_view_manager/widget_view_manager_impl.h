#ifndef __DALI_WIDGET_VIEW_INTERNAL_WIDGET_VIEW_MANAGER_H__
#define __DALI_WIDGET_VIEW_INTERNAL_WIDGET_VIEW_MANAGER_H__

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
#include <public_api/widget_view_manager/widget_view_manager.h>

// EXTERNAL INCLUDES
#include <dali/public-api/object/base-object.h>
#include <pepper-dali/public-api/compositor/compositor.h>
#include <map>

namespace Dali
{

namespace WidgetView
{

namespace Internal
{

class WidgetViewManager;
typedef IntrusivePtr<WidgetViewManager> WidgetViewManagerPtr;

class WidgetViewManager : public BaseObject, public ConnectionTracker
{
public:

  /**
   * @copydoc Dali::WidgetView::WidgetViewManager::New()
   */
  static WidgetViewManagerPtr New( Application application, const std::string& name );

  /**
   * @copydoc Dali::WidgetView::WidgetViewManager::AddWidget()
   */
  Dali::WidgetView::WidgetView AddWidget( const std::string& widgetId, const char* bundleData, int width, int height );

private:

  /**
   * Construct a new WidgetViewManager.
   */
  WidgetViewManager();

  /**
   * A reference counted object may only be deleted by calling Unreference()
   */
  virtual ~WidgetViewManager();

  /**
   * Initialize the WidgetViewManager
   */
  int Initialize( Application application, const std::string& name );

  void OnObjectViewAdded( Pepper::Compositor compositor, Pepper::ObjectView objectView );
  void OnObjectViewDeleted( Pepper::Compositor compositor, Pepper::ObjectView objectView );

private:

  // Undefined
  WidgetViewManager( const WidgetViewManager& );

  // Undefined
  WidgetViewManager& operator= ( const WidgetViewManager& );

private:

  Pepper::Compositor mCompositor;

  typedef std::map<std::string, Dali::WidgetView::WidgetView> WidgetViewContainer;
  typedef WidgetViewContainer::iterator WidgetViewIter;

  WidgetViewContainer mWidgetViewContainer;
};

} // namespace Internal

// Helpers for public-api forwarding methods

inline Internal::WidgetViewManager& GetImplementation( WidgetViewManager& obj )
{
  DALI_ASSERT_ALWAYS( obj );

  Dali::BaseObject& handle = obj.GetBaseObject();

  return static_cast<Internal::WidgetViewManager&>( handle );
}

inline const Internal::WidgetViewManager& GetImplementation( const WidgetViewManager& obj )
{
  DALI_ASSERT_ALWAYS( obj );

  const Dali::BaseObject& handle = obj.GetBaseObject();

  return static_cast<const Internal::WidgetViewManager&>( handle );
}

} // namespace WidgetView

} // namespace Dali

#endif // __DALI_WIDGET_VIEW_INTERNAL_WIDGET_VIEW_MANAGER_H__
