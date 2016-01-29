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

} // unnamed namespace

WidgetViewManagerPtr WidgetViewManager::New( Application application, const std::string& name )
{
  WidgetViewManagerPtr impl = new WidgetViewManager();

  // Second-phase init of the implementation
  impl->Initialize( application, name );

  return impl;
}

WidgetViewManager::WidgetViewManager()
{
}

WidgetViewManager::~WidgetViewManager()
{
}

void WidgetViewManager::Initialize( Application application, const std::string& name )
{
  // create compositor
  mCompositor = Pepper::Compositor::New( application, name );

  mCompositor.ObjectViewAddedSignal().Connect( this, &WidgetViewManager::OnObjectViewAdded );
  mCompositor.ObjectViewDeletedSignal().Connect( this, &WidgetViewManager::OnObjectViewDeleted );
}

Dali::WidgetView::WidgetViewManager::WidgetViewManagerSignalType& WidgetViewManager::WidgetViewAddedSignal()
{
  return mWidgetViewAddedSignal;
}

Dali::WidgetView::WidgetViewManager::WidgetViewManagerSignalType& WidgetViewManager::WidgetViewDeletedSignal()
{
  return mWidgetViewDeletedSignal;
}

void WidgetViewManager::OnObjectViewAdded( Pepper::Compositor compositor, Pepper::ObjectView objectView )
{
  // Add a new widget view
  Dali::WidgetView::WidgetView widgetView = Dali::WidgetView::WidgetView::New();

  Dali::WidgetView::GetImplementation( widgetView ).SetObjectView( objectView );

  // Add to map
  mWidgetViewContainer.insert(std::pair<Pepper::ObjectView, Dali::WidgetView::WidgetView>(objectView, widgetView));

  // Emit signal
  Dali::WidgetView::WidgetViewManager handle( this );
  mWidgetViewAddedSignal.Emit( handle, widgetView );

  DALI_LOG_INFO( gWidgetViewManagerLogging, Debug::Verbose, "WidgetViewManager::OnObjectViewAdded: ObjectView is added!\n" );
}

void WidgetViewManager::OnObjectViewDeleted( Pepper::Compositor compositor, Pepper::ObjectView objectView )
{
  // Remove from map
  if( mWidgetViewContainer.size() > 0)
  {
    WidgetViewIter iter = mWidgetViewContainer.find( objectView );
    if( iter != mWidgetViewContainer.end() )
    {
      Dali::WidgetView::WidgetView widgetView = iter->second;

      // Emit signal
      Dali::WidgetView::WidgetViewManager handle( this );
      mWidgetViewAddedSignal.Emit( handle, widgetView );

      mWidgetViewContainer.erase( iter );
    }
  }

  DALI_LOG_INFO( gWidgetViewManagerLogging, Debug::Verbose, "WidgetViewManager::OnObjectViewDeleted: ObjectView is deleted!\n" );
}

} // namespace Internal

} // namespace WidgetView

} // namespace Dali
