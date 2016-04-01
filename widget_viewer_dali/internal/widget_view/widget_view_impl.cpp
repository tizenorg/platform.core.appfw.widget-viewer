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

Dali::WidgetView::WidgetView WidgetView::New( const std::string& widgetId, const std::string& contentInfo, int width, int height, double period )
{
  // Create the implementation, temporarily owned on stack
  IntrusivePtr< WidgetView > internalWidgetView = new WidgetView( widgetId, contentInfo, width, height, period );

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
  mContentInfo(),
  mTitle(),
  mBundle( NULL ),
  mWidth( 0 ),
  mHeight( 0 ),
  mPid( 0 ),
  mPeriod( 0.0 ),
  mPermanentDelete( true )
{
}

WidgetView::WidgetView( const std::string& widgetId, const std::string& contentInfo, int width, int height, double period )
: Control( ControlBehaviour( REQUIRES_TOUCH_EVENTS ) ),
  mWidgetId( widgetId ),
  mInstanceId(),
  mContentInfo( contentInfo ),
  mTitle(),
  mBundle( NULL ),
  mWidth( width ),
  mHeight( height ),
  mPid( 0 ),
  mPeriod( period ),
  mPermanentDelete( true )
{
}

WidgetView::~WidgetView()
{
  if( !mWidgetId.empty() && !mInstanceId.empty() )
  {
    widget_instance_terminate( mWidgetId.c_str(), mInstanceId.c_str() );

    if( mPermanentDelete )
    {
      widget_instance_destroy( mWidgetId.c_str(), mInstanceId.c_str() );
    }
  }

  if( mBundle )
  {
    bundle_free( mBundle );
  }
}

const std::string& WidgetView::GetWidgetId() const
{
  return mWidgetId;
}

const std::string& WidgetView::GetInstanceId() const
{
  return mInstanceId;
}

const std::string& WidgetView::GetContentInfo() const
{
  return mContentInfo;
}

const std::string& WidgetView::GetTitle() const
{
  return mTitle;
}

double WidgetView::GetPeriod() const
{
  return mPeriod;
}

void WidgetView::ActivateFaultedWidget()
{
  if( mPid < 0 )
  {
    // launch widget again
    mPid = widget_instance_launch( mWidgetId.c_str(), mInstanceId.c_str(), mBundle, mWidth, mHeight );
    if( mPid < 0)
    {
      DALI_LOG_INFO( gWidgetViewLogging, Debug::Verbose, "WidgetView::ActivateFaultedWidget: widget_instance_launch is failed. [%s]\n", mWidgetId.c_str() );
      return;
    }

    DALI_LOG_INFO( gWidgetViewLogging, Debug::Verbose, "WidgetView::ActivateFaultedWidget: widget_instance_launch is called. [%s, mPid = %d]\n", mWidgetId.c_str(), mPid );
  }
}

bool WidgetView::IsWidgetFaulted()
{
  return mPid < 0 ? true : false;
}

void WidgetView::SetPermanentDelete( bool permanentDelete )
{
  mPermanentDelete = permanentDelete;
}

void WidgetView::AddObjectView( Pepper::ObjectView objectView )
{
  mObjectView = objectView;

  mObjectView.SetParentOrigin( ParentOrigin::CENTER );
  mObjectView.SetAnchorPoint( AnchorPoint::CENTER );

  Self().Add( mObjectView );
  Self().SetResizePolicy( ResizePolicy::FIT_TO_CHILDREN, Dimension::ALL_DIMENSIONS );

  mTitle = mObjectView.GetTitle();

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

  if( !mContentInfo.empty() )
  {
    DALI_LOG_INFO( gWidgetViewLogging, Debug::Verbose, "WidgetView::OnInitialize: decode bundle\n" );

    mBundle = bundle_decode( reinterpret_cast< const bundle_raw* >( mContentInfo.c_str() ), mContentInfo.length() );
    if( !mBundle )
    {
      DALI_LOG_INFO( gWidgetViewLogging, Debug::Verbose, "WidgetView::OnInitialize: Invalid bundle data.\n" );
      return;
    }

    bundle_get_str( mBundle, WIDGET_K_INSTANCE, &instanceId );
  }

  if( !instanceId )
  {
    int ret = widget_instance_create( mWidgetId.c_str(), &instanceId );
    if( ret < 0 || !instanceId )
    {
      DALI_LOG_INFO( gWidgetViewLogging, Debug::Verbose, "WidgetView::OnInitialize: widget_instance_create is failed [%s].\n", mWidgetId.c_str() );
      return;
    }

    DALI_LOG_INFO( gWidgetViewLogging, Debug::Verbose, "WidgetView::OnInitialize: widget_instance_create is called. [widget id = %s, instance id = %s]\n",
                   mWidgetId.c_str(), instanceId );
  }

  mInstanceId = instanceId;

  // launch widget
  mPid = widget_instance_launch( mWidgetId.c_str(), instanceId, mBundle, mWidth, mHeight );
  if( mPid < 0)
  {
    DALI_LOG_INFO( gWidgetViewLogging, Debug::Verbose, "WidgetView::OnInitialize: widget_instance_launch is failed. [%s]\n", mWidgetId.c_str() );
    return;
  }

  DALI_LOG_INFO( gWidgetViewLogging, Debug::Verbose, "WidgetView::OnInitialize: widget_instance_launch is called. [%s, mPid = %d]\n", mWidgetId.c_str(), mPid );
}

} // namespace Internal

} // namespace WidgetView

} // namespace Dali
