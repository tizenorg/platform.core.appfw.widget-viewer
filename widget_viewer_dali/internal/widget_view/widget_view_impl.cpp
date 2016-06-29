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

// CLASS HEADER
#include <internal/widget_view/widget_view_impl.h>

// INTERNAL INCLUDES

// EXTERNAL INCLUDES
#include <dali/integration-api/debug.h>
#include <string.h>
#include <widget_service.h>
#include <widget_instance.h>
#include <tzplatform_config.h>

namespace Dali
{

namespace WidgetView
{

namespace Internal
{

namespace
{

#define WIDGET_VIEW_RESOURCE_DEFAULT_IMG "/widget_viewer_dali/images/unknown.png"

#if defined(DEBUG_ENABLED)
Integration::Log::Filter* gWidgetViewLogging  = Integration::Log::Filter::New( Debug::Verbose, false, "LOG_WIDGET_VIEW" );
#endif

} // unnamed namespace

Dali::WidgetView::WidgetView WidgetView::New( const std::string& widgetId, const std::string& contentInfo, int width, int height, double updatePeriod )
{
  // Create the implementation, temporarily owned on stack
  IntrusivePtr< WidgetView > internalWidgetView = new WidgetView( widgetId, contentInfo, width, height, updatePeriod );

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
  mUpdatePeriod( 0.0 ),
  mPreviewEnabled( true ),
  mStateTextEnabled( true ),
  mPermanentDelete( true )
{
}

WidgetView::WidgetView( const std::string& widgetId, const std::string& contentInfo, int width, int height, double updatePeriod )
: Control( ControlBehaviour( REQUIRES_TOUCH_EVENTS ) ),
  mWidgetId( widgetId ),
  mInstanceId(),
  mContentInfo( contentInfo ),
  mTitle(),
  mBundle( NULL ),
  mWidth( width ),
  mHeight( height ),
  mPid( 0 ),
  mUpdatePeriod( updatePeriod ),
  mPreviewEnabled( true ),
  mStateTextEnabled( true ),
  mPermanentDelete( true )
{
}

WidgetView::~WidgetView()
{
  if( !mWidgetId.empty() && !mInstanceId.empty() )
  {
    widget_instance_terminate( mInstanceId.c_str() );

    if( mPermanentDelete )
    {
      widget_instance_destroy( mInstanceId.c_str() );
    }
  }

  if( mBundle )
  {
    bundle_free( mBundle );
  }
}

bool WidgetView::PauseWidget()
{
  int ret = widget_instance_pause( mInstanceId.c_str() );
  if( ret < 0 )
  {
    DALI_LOG_INFO( gWidgetViewLogging, Debug::Verbose, "WidgetView::PauseWidget: Fail to pause widget(%s, %s) [%d]\n", mWidgetId.c_str(), mInstanceId.c_str(), ret );
    return false;
  }

  DALI_LOG_INFO( gWidgetViewLogging, Debug::Verbose, "WidgetView::PauseWidget: Widget is paused (%s, %s)\n", mWidgetId.c_str(), mInstanceId.c_str() );

  return true;
}

bool WidgetView::ResumeWidget()
{
  int ret = widget_instance_resume( mInstanceId.c_str() );
  if( ret < 0 )
  {
    DALI_LOG_INFO( gWidgetViewLogging, Debug::Verbose, "WidgetView::ResumeWidget: Fail to resume widget(%s, %s) [%d]\n", mWidgetId.c_str(), mInstanceId.c_str(), ret );
    return false;
  }

  DALI_LOG_INFO( gWidgetViewLogging, Debug::Verbose, "WidgetView::ResumeWidget: Widget is resumed (%s, %s)\n", mWidgetId.c_str(), mInstanceId.c_str() );

  return true;
}

const std::string& WidgetView::GetWidgetId() const
{
  return mWidgetId;
}

const std::string& WidgetView::GetInstanceId() const
{
  return mInstanceId;
}

const std::string& WidgetView::GetContentInfo()
{
  widget_instance_h instance;
  char* contentInfo = NULL;

  mContentInfo.clear();

  if( mWidgetId.empty() || mInstanceId.empty() )
  {
    DALI_LOG_INFO( gWidgetViewLogging, Debug::Verbose, "WidgetView::GetContentInfo: Widget id (%s) or instance id (%s) is invalid.\n", mWidgetId.c_str(), mInstanceId.c_str() );
    return mContentInfo;
  }

  instance = widget_instance_get_instance( mWidgetId.c_str(), mInstanceId.c_str() );
  if( !instance )
  {
    DALI_LOG_INFO( gWidgetViewLogging, Debug::Verbose, "WidgetView::GetContentInfo: widget_instance_get_instance is failed. [%s]\n", mInstanceId.c_str() );
    return mContentInfo;
  }

  if( widget_instance_get_content( instance, &contentInfo ) < 0 )
  {
    DALI_LOG_INFO( gWidgetViewLogging, Debug::Verbose, "WidgetView::GetContentInfo: Failed to get content of widget. [%s]\n", mInstanceId.c_str() );
    return mContentInfo;
  }

  mContentInfo = reinterpret_cast< char* >( contentInfo );

  return mContentInfo;
}

const std::string& WidgetView::GetTitle()
{
  if( mObjectView )
  {
    mTitle = mObjectView.GetTitle();
    if( mTitle.empty() )
    {
      mTitle = widget_service_get_name( mWidgetId.c_str(), NULL );
    }
  }

  DALI_LOG_INFO( gWidgetViewLogging, Debug::Verbose, "WidgetView::GetTitle: title = %s\n", mTitle.c_str() );

  return mTitle;
}

double WidgetView::GetUpdatePeriod() const
{
  return mUpdatePeriod;
}

void WidgetView::Show()
{
  if( mObjectView )
  {
    mObjectView.Show();
  }
}

void WidgetView::Hide()
{
  if( mObjectView )
  {
    mObjectView.Hide();
  }
}

bool WidgetView::CancelTouchEvent()
{
  if( mObjectView )
  {
    return mObjectView.CancelTouchEvent();
  }

  return false;
}

void WidgetView::SetPreviewEnabled( bool enabled )
{
  mPreviewEnabled = enabled;

  if( mPreviewImage )
  {
    mPreviewImage.SetVisible( enabled );
  }
}

bool WidgetView::GetPreviewEnabled() const
{
  return mPreviewEnabled;
}

void WidgetView::SetStateTextEnabled( bool enabled )
{
  mStateTextEnabled = enabled;

  if( mStateText )
  {
    mStateText.SetVisible( enabled );
  }
}

bool WidgetView::GetStateTextEnabled() const
{
  return mStateTextEnabled;
}

void WidgetView::ActivateFaultedWidget()
{
  if( mPid < 0 )
  {
    // Esable preview and text
    if( mPreviewEnabled )
    {
      mPreviewImage.SetVisible( true );
    }

    if( mStateTextEnabled )
    {
      mStateText.SetVisible( true );
    }

    // launch widget again
    mPid = widget_instance_launch( mInstanceId.c_str(), (char *)mContentInfo.c_str(), mWidth, mHeight );
    if( mPid < 0)
    {
      DALI_LOG_INFO( gWidgetViewLogging, Debug::Verbose, "WidgetView::ActivateFaultedWidget: widget_instance_launch is failed. [%s]\n", mWidgetId.c_str() );

      // Emit signal
      Dali::WidgetView::WidgetView handle( GetOwner() );
      mWidgetCreationAbortedSignal.Emit( handle );

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

  // Disable preview and text
  if( mPreviewEnabled )
  {
    mPreviewImage.SetVisible( false );
  }

  if( mStateTextEnabled )
  {
    mStateText.SetVisible( false );
  }

  // Emit signal
  Dali::WidgetView::WidgetView handle( GetOwner() );
  mWidgetAddedSignal.Emit( handle );

  DALI_LOG_INFO( gWidgetViewLogging, Debug::Verbose, "WidgetView::AddObjectView: ObjectView is added.\n" );
}

void WidgetView::RemoveObjectView()
{
  // Enable preview and text
  if( mPreviewEnabled )
  {
    mPreviewImage.SetVisible( true );
  }

  if( mStateTextEnabled )
  {
    mStateText.SetVisible( true );
  }

  // Emit signal
  Dali::WidgetView::WidgetView handle( GetOwner() );
  mWidgetDeletedSignal.Emit( handle );

  mObjectView.Reset();

  DALI_LOG_INFO( gWidgetViewLogging, Debug::Verbose, "WidgetView::RemoveObjectView: ObjectView is removed.\n" );
}

void WidgetView::SendWidgetEvent( int event )
{
  Dali::WidgetView::WidgetView handle( GetOwner() );

  DALI_LOG_INFO( gWidgetViewLogging, Debug::Verbose, "WidgetView::SendWidgetEvent: event = %d widget = %s\n", event,  mWidgetId.c_str() );

  // Emit signal
  switch( event )
  {
    case WIDGET_INSTANCE_EVENT_UPDATE:
    {
      mWidgetContentUpdatedSignal.Emit( handle );
      break;
    }
    case WIDGET_INSTANCE_EVENT_PERIOD_CHANGED:
    {
      mWidgetUpdatePeriodChangedSignal.Emit( handle );
      break;
    }
    case WIDGET_INSTANCE_EVENT_SIZE_CHANGED:
    {
      mWidgetResizedSignal.Emit( handle );
      break;
    }
    case WIDGET_INSTANCE_EVENT_EXTRA_UPDATED:
    {
      mWidgetExtraInfoUpdatedSignal.Emit( handle );
      break;
    }
    case WIDGET_INSTANCE_EVENT_FAULT:
    {
      mWidgetFaultedSignal.Emit( handle );
      break;
    }
    default:
    {
      break;
    }
  }
}

Dali::WidgetView::WidgetView::WidgetViewSignalType& WidgetView::WidgetAddedSignal()
{
  return mWidgetAddedSignal;
}

Dali::WidgetView::WidgetView::WidgetViewSignalType& WidgetView::WidgetDeletedSignal()
{
  return mWidgetDeletedSignal;
}

Dali::WidgetView::WidgetView::WidgetViewSignalType& WidgetView::WidgetCreationAbortedSignal()
{
  return mWidgetCreationAbortedSignal;
}

Dali::WidgetView::WidgetView::WidgetViewSignalType& WidgetView::WidgetResizedSignal()
{
  return mWidgetResizedSignal;
}

Dali::WidgetView::WidgetView::WidgetViewSignalType& WidgetView::WidgetContentUpdatedSignal()
{
  return mWidgetContentUpdatedSignal;
}

Dali::WidgetView::WidgetView::WidgetViewSignalType& WidgetView::WidgetExtraInfoUpdatedSignal()
{
  return mWidgetExtraInfoUpdatedSignal;
}

Dali::WidgetView::WidgetView::WidgetViewSignalType& WidgetView::WidgetUpdatePeriodChangedSignal()
{
  return mWidgetUpdatePeriodChangedSignal;
}

Dali::WidgetView::WidgetView::WidgetViewSignalType& WidgetView::WidgetFaultedSignal()
{
  return mWidgetFaultedSignal;
}

void WidgetView::OnInitialize()
{
  char* instanceId = NULL;
  char* previewPath = NULL;
  std::string previewImage;
  widget_size_type_e sizeType;

  int ret = widget_instance_create( mWidgetId.c_str(), &instanceId );
  if( ret < 0 || !instanceId )
  {
    DALI_LOG_INFO( gWidgetViewLogging, Debug::Verbose, "WidgetView::OnInitialize: widget_instance_create is failed [%s].\n", mWidgetId.c_str() );
    return;
  }

  DALI_LOG_INFO( gWidgetViewLogging, Debug::Verbose, "WidgetView::OnInitialize: widget_instance_create is called. [widget id = %s, instance id = %s]\n",
                 mWidgetId.c_str(), instanceId );

  mInstanceId = instanceId;

  // Preview image
  widget_service_get_size_type( mWidth, mHeight, &sizeType );

  previewPath = widget_service_get_preview_image_path( mWidgetId.c_str(), sizeType );
  if( previewPath )
  {
    previewImage = previewPath;
    free( previewPath );
  }
  else
  {
    previewImage = tzplatform_getenv( TZ_SYS_SHARE );
    previewImage.append( WIDGET_VIEW_RESOURCE_DEFAULT_IMG );
  }

  DALI_LOG_INFO( gWidgetViewLogging, Debug::Verbose, "WidgetView::OnInitialize: preview image path = %s\n", previewImage.c_str() );

  mPreviewImage = Toolkit::ImageView::New( previewImage );

  mPreviewImage.SetParentOrigin( ParentOrigin::CENTER );
  mPreviewImage.SetAnchorPoint( AnchorPoint::CENTER );

  if( !previewPath )
  {
    mPreviewImage.SetSize( mWidth, mHeight );
  }

  Self().SetResizePolicy( ResizePolicy::FIT_TO_CHILDREN, Dimension::ALL_DIMENSIONS );
  Self().Add( mPreviewImage );

  // State text
  // TODO: use po files
  mStateText = Toolkit::TextLabel::New( "Loading..." );

  mStateText.SetParentOrigin( ParentOrigin::CENTER );
  mStateText.SetAnchorPoint( AnchorPoint::CENTER );
  mStateText.SetProperty( Toolkit::TextLabel::Property::HORIZONTAL_ALIGNMENT, "CENTER" );
  mStateText.SetProperty( Toolkit::TextLabel::Property::VERTICAL_ALIGNMENT, "CENTER" );

  mPreviewImage.Add( mStateText );

  // launch widget
  mPid = widget_instance_launch( instanceId, (char *)mContentInfo.c_str(), mWidth, mHeight );
  if( mPid < 0)
  {
    DALI_LOG_INFO( gWidgetViewLogging, Debug::Verbose, "WidgetView::OnInitialize: widget_instance_launch is failed. [%s]\n", mWidgetId.c_str() );

    // Emit signal
    Dali::WidgetView::WidgetView handle( GetOwner() );
    mWidgetCreationAbortedSignal.Emit( handle );

    return;
  }

  DALI_LOG_INFO( gWidgetViewLogging, Debug::Verbose, "WidgetView::OnInitialize: widget_instance_launch is called. [%s, mPid = %d]\n", mWidgetId.c_str(), mPid );
}

void WidgetView::OnSizeSet( const Vector3& targetSize )
{
  if( mObjectView )
  {
    mObjectView.SetSize( targetSize );
  }
}

} // namespace Internal

} // namespace WidgetView

} // namespace Dali
