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

// EXTERNAL INCLUDES
#include <dali/devel-api/adaptor-framework/singleton-service.h>

// INTERNAL INCLUDES

namespace Dali
{

namespace WidgetView
{

namespace Internal
{

namespace
{

} // unnamed namespace

WidgetView::WidgetViewManager WidgetViewManager::Get()
{
  WidgetView::WidgetViewManager manager;

  SingletonService singletonService( SingletonService::Get() );
  if ( singletonService )
  {
    // Check whether the widget view manager is already created
    Dali::BaseHandle handle = singletonService.GetSingleton( typeid( WidgetView::WidgetViewManager ) );
    if( handle )
    {
      // If so, downcast the handle of singleton to widget view manager
      manager = WidgetView::WidgetViewManager( dynamic_cast< WidgetViewManager* >( handle.GetObjectPtr() ) );
    }

    if( !manager )
    {
      // If not, create the widget view manager and register it as a singleton
      WidgetViewManager* internalManager = new WidgetViewManager();
      manager = WidgetView::WidgetViewManager( internalManager );
//      internalManager->Initialise();
      singletonService.Register( typeid(manager), manager );
    }
  }

  return manager;
}

WidgetViewManager::WidgetViewManager()
{
}

WidgetViewManager::~WidgetViewManager()
{
}

} // namespace Internal

} // namespace WidgetView

} // namespace Dali
