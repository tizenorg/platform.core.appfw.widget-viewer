/*
 * Copyright (c) 2016 Samsung Electronics Co., Ltd.
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
#include <cynara-client.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
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

#define SMACK_LABEL_LENGTH 255

#if defined(DEBUG_ENABLED)
Integration::Log::Filter* gWidgetViewManagerLogging  = Integration::Log::Filter::New( Debug::Verbose, false, "LOG_WIDGET_VIEW_MANAGER" );
#endif

static bool IsWidgetFeatureEnabled()
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

static bool CheckPrivilege( const char* privilege )
{
  cynara* cynara;
  int fd = 0;
  int ret = 0;
  char subjectLabel[SMACK_LABEL_LENGTH + 1] = "";
  char uid[10] = { 0, };
  const char* clientSession = "";

  ret = cynara_initialize( &cynara, NULL );
  if( ret != CYNARA_API_SUCCESS )
  {
    return false;
  }

  fd = open( "/proc/self/attr/current", O_RDONLY );
  if( fd < 0 )
  {
    cynara_finish( cynara );
    return false;
  }

  ret = read( fd, subjectLabel, SMACK_LABEL_LENGTH );
  if( ret < 0 )
  {
    close( fd );
    cynara_finish( cynara );
    return false;
  }

  close( fd );

  snprintf( uid, 10, "%d", getuid() );

  ret = cynara_check( cynara, subjectLabel, clientSession, uid, privilege );
  if( ret != CYNARA_API_ACCESS_ALLOWED )
  {
    cynara_finish( cynara );
    return false;
  }

  cynara_finish( cynara );

  return true;
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
  widget_instance_unlisten_event( WidgetViewManager::WidgetEventCallback );
  widget_instance_fini();
}

int WidgetViewManager::Initialize( Application application, const std::string& name )
{
  if( !IsWidgetFeatureEnabled() )
  {
    DALI_LOG_INFO( gWidgetViewManagerLogging, Debug::Verbose, "WidgetViewManager::Initialize: Widget feature is not enabled.\n" );
    return WIDGET_ERROR_NOT_SUPPORTED;
  }

  if( !CheckPrivilege( "http://tizen.org/privilege/widget.viewer" ) )
  {
    DALI_LOG_INFO( gWidgetViewManagerLogging, Debug::Verbose, "WidgetViewManager::Initialize: Privilege error.\n" );
    return WIDGET_ERROR_PERMISSION_DENIED;
  }

  // create compositor
  mCompositor = Pepper::Compositor::New( application, name );

  mCompositor.ObjectViewAddedSignal().Connect( this, &WidgetViewManager::OnObjectViewAdded );
  mCompositor.ObjectViewDeletedSignal().Connect( this, &WidgetViewManager::OnObjectViewDeleted );

  // init widget service
  widget_instance_init( name.c_str() );
  widget_instance_listen_event( WidgetViewManager::WidgetEventCallback, this );

  DALI_LOG_INFO( gWidgetViewManagerLogging, Debug::Verbose, "WidgetViewManager::Initialize: success.\n" );

  setenv("WAYLAND_DISPLAY", mCompositor.GetName().c_str(), 1);

  return WIDGET_ERROR_NONE;
}

Dali::WidgetView::WidgetView WidgetViewManager::AddWidget( const std::string& widgetId, const std::string& contentInfo, int width, int height, double updatePeriod )
{
  // Add a new widget view
  Dali::WidgetView::WidgetView widgetView = Dali::WidgetView::WidgetView::New( widgetId, contentInfo, width, height, updatePeriod );

  std::string instanceId = widgetView.GetInstanceId();

  if( !instanceId.empty() )
  {
    // Add to map
    mWidgetViewContainer.insert( std::pair<std::string, Dali::WidgetView::WidgetView>( instanceId, widgetView ) );
  }

  DALI_LOG_INFO( gWidgetViewManagerLogging, Debug::Verbose, "WidgetViewManager::AddWidget: success [%s]\n", widgetId.c_str() );

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

int WidgetViewManager::WidgetEventCallback( const char* widgetId, const char* instanceId, int event, void* data )
{
  WidgetViewManager* widgetViewManager = static_cast< WidgetViewManager* >( data );

  if( widgetViewManager->mWidgetViewContainer.size() > 0)
  {
    WidgetViewIter iter = widgetViewManager->mWidgetViewContainer.find( std::string( instanceId ) );
    if( iter != widgetViewManager->mWidgetViewContainer.end() )
    {
      Dali::WidgetView::WidgetView widgetView = iter->second;

      Dali::WidgetView::GetImplementation( widgetView ).SendWidgetEvent( event );

      return 0;
    }
  }

  DALI_LOG_INFO( gWidgetViewManagerLogging, Debug::Verbose, "WidgetViewManager::WidgetEventCallback: WidgetView is not found! [%s, %s]\n", widgetId, instanceId );

  return 0;
}

} // namespace Internal

} // namespace WidgetView

} // namespace Dali
