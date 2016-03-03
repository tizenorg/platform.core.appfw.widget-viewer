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
#include <internal/widget_view/widget_view_impl.h>

// INTERNAL INCLUDES

// EXTERNAL INCLUDES
#include <dali/integration-api/debug.h>
#include <string.h>
#include <widget_service.h>
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
Integration::Log::Filter* gWidgetViewLogging  = Integration::Log::Filter::New( Debug::Verbose, false, "LOG_WIDGET_VIEW" );
#endif

} // unnamed namespace

Dali::WidgetView::WidgetView WidgetView::New( const std::string& widgetId, const char* bundleData, int width, int height )
{
  // Create the implementation, temporarily owned on stack
  IntrusivePtr< WidgetView > internalWidgetView = new WidgetView( widgetId, bundleData, width, height );

  // Pass ownership to CustomActor
  Dali::WidgetView::WidgetView widgetView( *internalWidgetView );

  // Second-phase init of the implementation
  // This can only be done after the CustomActor connection has been made...
  internalWidgetView->Initialize();

  return widgetView;
}

WidgetView::WidgetView()
: Control( ControlBehaviour( REQUIRES_TOUCH_EVENTS ) ),
  mWidgetId(),
  mInstanceId(),
  mBundleData( NULL ),
  mBundle( NULL ),
  mWidth( 0 ),
  mHeight( 0 ),
  mPid( 0 )
{
}

WidgetView::WidgetView( const std::string& widgetId, const char* bundleData, int width, int height )
: Control( ControlBehaviour( REQUIRES_TOUCH_EVENTS ) ),
  mWidgetId( widgetId ),
  mInstanceId(),
  mBundle( NULL ),
  mWidth( width ),
  mHeight( height ),
  mPid( 0 )
{
  mBundleData = strdup( bundleData );
}

WidgetView::~WidgetView()
{
  if( !mWidgetId.empty() && !mInstanceId.empty() )
  {
    widget_instance_terminate( mWidgetId.c_str(), mInstanceId.c_str() );

    // TODO: permanent_delete?
    widget_instance_destroy( mWidgetId.c_str(), mInstanceId.c_str() );
  }

  if( mBundleData )
  {
    free( mBundleData );
  }

  if( mBundle )
  {
    bundle_free( mBundle );
  }
}

const std::string& WidgetView::GetInstanceId() const
{
  return mInstanceId;
}

void WidgetView::AddObjectView( Pepper::ObjectView objectView )
{
  mObjectView = objectView;

  mObjectView.SetParentOrigin( ParentOrigin::CENTER );
  mObjectView.SetAnchorPoint( AnchorPoint::CENTER );

  Self().Add( mObjectView );
  Self().SetResizePolicy( ResizePolicy::FIT_TO_CHILDREN, Dimension::ALL_DIMENSIONS );

  // Emit signal
  Dali::WidgetView::WidgetView handle( GetOwner() );
  mWidgetAddedSignal.Emit( handle );
}

void WidgetView::RemoveObjectView()
{
  // Emit signal
  Dali::WidgetView::WidgetView handle( GetOwner() );
  mWidgetDeletedSignal.Emit( handle );

  mObjectView.Reset();
}

Dali::WidgetView::WidgetView::WidgetViewSignalType& WidgetView::WidgetAddedSignal()
{
  return mWidgetAddedSignal;
}

Dali::WidgetView::WidgetView::WidgetViewSignalType& WidgetView::WidgetDeletedSignal()
{
  return mWidgetDeletedSignal;
}

void WidgetView::OnInitialize()
{
  char* instanceId = NULL;

  if( mBundleData )
  {
    mBundle = bundle_decode( reinterpret_cast< const bundle_raw* >( mBundleData ), strlen( mBundleData ) );
    if( !mBundle )
    {
      DALI_LOG_INFO( gWidgetViewLogging, Debug::Verbose, "WidgetView::OnInitialize: Invalid bundle data.\n" );
      return;
    }

    bundle_get_str( mBundle, WIDGET_K_INSTANCE, &instanceId );

    if( !instanceId )
    {
      int ret = widget_instance_create( mWidgetId.c_str(), &instanceId );
      if( ret < 0 || !instanceId )
      {
        DALI_LOG_INFO( gWidgetViewLogging, Debug::Verbose, "WidgetView::OnInitialize: widget_instance_create is failed [%s].\n", mWidgetId.c_str() );
        return;
      }
    }

    mInstanceId = instanceId;

    // launch widget
    mPid = widget_instance_launch( mWidgetId.c_str(), instanceId, mBundle, mWidth, mHeight );
  }
}

} // namespace Internal

} // namespace WidgetView

} // namespace Dali
