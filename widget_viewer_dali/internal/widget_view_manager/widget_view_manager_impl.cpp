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

// CLASS HEADER
#include <internal/widget_view_manager/widget_view_manager_impl.h>

// INTERNAL INCLUDES
#include <internal/widget_view/widget_view_impl.h>

// EXTERNAL INCLUDES
#include <dali/integration-api/debug.h>
#include <system_info.h>
#include <widget_errno.h>
#include <widget_instance.h>

namespace Dali
{

namespace WidgetView
{

namespace Internal
{

namespace
{

#if defined(DEBUG_ENABLED)
Integration::Log::Filter* gWidgetViewManagerLogging  = Integration::Log::Filter::New( Debug::Verbose, false, "LOG_WIDGET_VIEW_MANAGER" );
#endif

static inline bool IsWidgetFeatureEnabled()
{
  static bool feature = false;
  static bool retrieved = false;
  int ret;

  if( retrieved == true )
    return feature;

  ret = system_info_get_platform_bool( "http://tizen.org/feature/shell.appwidget", &feature );
  if( ret != SYSTEM_INFO_ERROR_NONE )
  {
    return false;
  }

  retrieved = true;

  return feature;
}

} // unnamed namespace

WidgetViewManagerPtr WidgetViewManager::New( Application application, const std::string& name )
{
  WidgetViewManagerPtr impl = new WidgetViewManager();

  // Second-phase init of the implementation
  if( impl->Initialize( application, name ) != WIDGET_ERROR_NONE )
  {
    DALI_LOG_INFO( gWidgetViewManagerLogging, Debug::Verbose, "WidgetViewManager::New: Fail to create WidgetViewManager.\n" );
    return NULL;
  }

  return impl;
}

WidgetViewManager::WidgetViewManager()
{
}

WidgetViewManager::~WidgetViewManager()
{
  widget_instance_fini();
}

int WidgetViewManager::Initialize( Application application, const std::string& name )
{
  if( !IsWidgetFeatureEnabled() )
  {
    DALI_LOG_INFO( gWidgetViewManagerLogging, Debug::Verbose, "WidgetViewManager::Initialize: Widget feature is not enabled.\n" );
    return WIDGET_ERROR_NOT_SUPPORTED;
  }

  // create compositor
  mCompositor = Pepper::Compositor::New( application, name );

  mCompositor.ObjectViewAddedSignal().Connect( this, &WidgetViewManager::OnObjectViewAdded );
  mCompositor.ObjectViewDeletedSignal().Connect( this, &WidgetViewManager::OnObjectViewDeleted );

  // init widget service
  widget_instance_init( name.c_str() );

  return WIDGET_ERROR_NONE;
}

Dali::WidgetView::WidgetView WidgetViewManager::AddWidget( const std::string& widgetId, const char* bundleData, int width, int height )
{
  // Add a new widget view
  Dali::WidgetView::WidgetView widgetView = Dali::WidgetView::WidgetView::New( widgetId, bundleData, width, height );

  std::string instanceId = widgetView.GetInstanceId();

  if( !instanceId.empty() )
  {
    // Add to map
    mWidgetViewContainer.insert( std::pair<std::string, Dali::WidgetView::WidgetView>( instanceId, widgetView ) );
  }

  return widgetView;
}

void WidgetViewManager::OnObjectViewAdded( Pepper::Compositor compositor, Pepper::ObjectView objectView )
{
  std::string appId = objectView.GetAppId();  // widget instance id

  if( mWidgetViewContainer.size() > 0)
  {
    WidgetViewIter iter = mWidgetViewContainer.find( appId );
    if( iter != mWidgetViewContainer.end() )
    {
      Dali::WidgetView::WidgetView widgetView = iter->second;

      Dali::WidgetView::GetImplementation( widgetView ).AddObjectView( objectView );
    }
  }

  DALI_LOG_INFO( gWidgetViewManagerLogging, Debug::Verbose, "WidgetViewManager::OnObjectViewAdded: ObjectView is added!\n" );
}

void WidgetViewManager::OnObjectViewDeleted( Pepper::Compositor compositor, Pepper::ObjectView objectView )
{
  std::string appId = objectView.GetAppId();  // widget instance id

  // Remove from map
  if( mWidgetViewContainer.size() > 0)
  {
    WidgetViewIter iter = mWidgetViewContainer.find( appId );
    if( iter != mWidgetViewContainer.end() )
    {
      Dali::WidgetView::WidgetView widgetView = iter->second;

      Dali::WidgetView::GetImplementation( widgetView ).RemoveObjectView();

      mWidgetViewContainer.erase( iter );
    }
  }

  DALI_LOG_INFO( gWidgetViewManagerLogging, Debug::Verbose, "WidgetViewManager::OnObjectViewDeleted: ObjectView is deleted!\n" );
}

} // namespace Internal

} // namespace WidgetView

} // namespace Dali
