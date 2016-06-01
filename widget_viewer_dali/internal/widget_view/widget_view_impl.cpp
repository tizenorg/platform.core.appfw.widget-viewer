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
#include <internal/widget_view/widget_view_impl.h>

// INTERNAL INCLUDES

// EXTERNAL INCLUDES
#include <dali/public-api/object/type-registry.h>
#include <dali/devel-api/object/type-registry-helper.h>
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

BaseHandle Create()
{
  return Dali::WidgetView::WidgetView::New( "", "", 0, 0, 0.0 );
}

// Setup properties, signals and actions using the type-registry.
DALI_TYPE_REGISTRATION_BEGIN( Dali::WidgetView::WidgetView, Toolkit::Control, Create );

// Register Event-Side Properties
DALI_PROPERTY_REGISTRATION( Dali::WidgetView, WidgetView, "previewEnabled",     BOOLEAN, PREVIEW_ENABLED    )
DALI_PROPERTY_REGISTRATION( Dali::WidgetView, WidgetView, "stateTextEnabled",   BOOLEAN, STATE_TEXT_ENABLED )
DALI_PROPERTY_REGISTRATION( Dali::WidgetView, WidgetView, "permanentDelete",    FLOAT,   PERMANENT_DELETE   )

// Register Signals
DALI_SIGNAL_REGISTRATION(   Dali::WidgetView, WidgetView, "widgetAdded",               SIGNAL_WIDGET_ADDED                 )
DALI_SIGNAL_REGISTRATION(   Dali::WidgetView, WidgetView, "widgetDeleted",             SIGNAL_WIDGET_DELETED               )
DALI_SIGNAL_REGISTRATION(   Dali::WidgetView, WidgetView, "widgetCreationAborted",     SIGNAL_WIDGET_CREATION_ABORTED      )
DALI_SIGNAL_REGISTRATION(   Dali::WidgetView, WidgetView, "widgetResized",             SIGNAL_WIDGET_RESIZED               )
DALI_SIGNAL_REGISTRATION(   Dali::WidgetView, WidgetView, "widgetContentUpdated",      SIGNAL_WIDGET_CONTENT_UPDATED       )
DALI_SIGNAL_REGISTRATION(   Dali::WidgetView, WidgetView, "widgetExtraInfoUpdated",    SIGNAL_WIDGET_EXTRA_INFO_UPDATED    )
DALI_SIGNAL_REGISTRATION(   Dali::WidgetView, WidgetView, "widgetUpdatePeriodChanged", SIGNAL_WIDGET_UPDATE_PERIOD_CHANGED )
DALI_SIGNAL_REGISTRATION(   Dali::WidgetView, WidgetView, "widgetFaulted",             SIGNAL_WIDGET_FAULTED               )

DALI_TYPE_REGISTRATION_END()

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

bool WidgetView::PauseWidget()
{
  int ret = widget_instance_pause( mWidgetId.c_str(), mInstanceId.c_str() );
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
  int ret = widget_instance_resume( mWidgetId.c_str(), mInstanceId.c_str() );
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
  bundle* bundle = NULL;
  bundle_raw* contentInfo = NULL;
  int contentLength = 0;

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

  if( widget_instance_get_content( instance, &bundle ) < 0 )
  {
    DALI_LOG_INFO( gWidgetViewLogging, Debug::Verbose, "WidgetView::GetContentInfo: Failed to get content of widget. [%s]\n", mInstanceId.c_str() );
    return mContentInfo;
  }

  if( !bundle )
  {
    DALI_LOG_INFO( gWidgetViewLogging, Debug::Verbose, "WidgetView::GetContentInfo: Cotent of widget [%s] is invalid.\n", mInstanceId.c_str() );
    return mContentInfo;
  }

  if( bundle_encode( bundle, &contentInfo, &contentLength ) < 0 )
  {
    DALI_LOG_INFO( gWidgetViewLogging, Debug::Verbose, "WidgetView::GetContentInfo: bundle_encode is failed. [%s]\n", mInstanceId.c_str() );
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
    mPid = widget_instance_launch( mWidgetId.c_str(), mInstanceId.c_str(), mBundle, mWidth, mHeight );
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

bool WidgetView::DoConnectSignal( BaseObject* object, ConnectionTrackerInterface* tracker, const std::string& signalName, FunctorDelegate* functor )
{
  Dali::BaseHandle handle( object );

  bool connected( true );
  Dali::WidgetView::WidgetView widgetView = Dali::WidgetView::WidgetView::DownCast( handle );

  if( 0 == strcmp( signalName.c_str(), SIGNAL_WIDGET_ADDED ) )
  {
    widgetView.WidgetAddedSignal().Connect( tracker, functor );
  }
  else if( 0 == strcmp( signalName.c_str(), SIGNAL_WIDGET_DELETED ) )
  {
    widgetView.WidgetDeletedSignal().Connect( tracker, functor );
  }
  else if( 0 == strcmp( signalName.c_str(), SIGNAL_WIDGET_CREATION_ABORTED ) )
  {
    widgetView.WidgetCreationAbortedSignal().Connect( tracker, functor );
  }
  else if( 0 == strcmp( signalName.c_str(), SIGNAL_WIDGET_RESIZED ) )
  {
    widgetView.WidgetResizedSignal().Connect( tracker, functor );
  }
  else if( 0 == strcmp( signalName.c_str(), SIGNAL_WIDGET_CONTENT_UPDATED ) )
  {
    widgetView.WidgetContentUpdatedSignal().Connect( tracker, functor );
  }
  else if( 0 == strcmp( signalName.c_str(), SIGNAL_WIDGET_EXTRA_INFO_UPDATED ) )
  {
    widgetView.WidgetExtraInfoUpdatedSignal().Connect( tracker, functor );
  }
  else if( 0 == strcmp( signalName.c_str(), SIGNAL_WIDGET_UPDATE_PERIOD_CHANGED ) )
  {
    widgetView.WidgetUpdatePeriodChangedSignal().Connect( tracker, functor );
  }
  else if( 0 == strcmp( signalName.c_str(), SIGNAL_WIDGET_FAULTED ) )
  {
    widgetView.WidgetFaultedSignal().Connect( tracker, functor );
  }
  else
  {
    // signalName does not match any signal
    connected = false;
  }

  return connected;
}

void WidgetView::SetProperty( BaseObject* object, Property::Index index, const Property::Value& value )
{
  Dali::WidgetView::WidgetView widgetView = Dali::WidgetView::WidgetView::DownCast( Dali::BaseHandle( object ) );

  if ( widgetView )
  {
    switch ( index )
    {
      case Dali::WidgetView::WidgetView::Property::PREVIEW_ENABLED:
      {
        GetImplementation( widgetView ).SetPreviewEnabled( value.Get< bool >() );
        break;
      }

      case Dali::WidgetView::WidgetView::Property::STATE_TEXT_ENABLED:
      {
        GetImplementation( widgetView ).SetStateTextEnabled( value.Get< bool >() );
        break;
      }

      case Dali::WidgetView::WidgetView::Property::PERMANENT_DELETE:
      {
        GetImplementation( widgetView ).SetPermanentDelete( value.Get< bool >() );
        break;
      }
    }
  }
}

Property::Value WidgetView::GetProperty( BaseObject* object, Property::Index propertyIndex )
{
  Property::Value value;

  Dali::WidgetView::WidgetView widgetView = Dali::WidgetView::WidgetView::DownCast( Dali::BaseHandle( object ) );

  if ( widgetView )
  {
    switch ( propertyIndex )
    {
      case Dali::WidgetView::WidgetView::Property::PREVIEW_ENABLED:
      {
        value = GetImplementation( widgetView ).mPreviewEnabled;
        break;
      }

      case Dali::WidgetView::WidgetView::Property::STATE_TEXT_ENABLED:
      {
        value = GetImplementation( widgetView ).mStateTextEnabled;
        break;
      }

      case Dali::WidgetView::WidgetView::Property::PERMANENT_DELETE:
      {
        value = GetImplementation( widgetView ).mPermanentDelete;
        break;
      }
    }
  }

  return value;
}

void WidgetView::OnInitialize()
{
  char* instanceId = NULL;
  char* previewPath = NULL;
  std::string previewImage;
  widget_size_type_e sizeType;

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
  mPid = widget_instance_launch( mWidgetId.c_str(), instanceId, mBundle, mWidth, mHeight );
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
  // Up call to Control
  Control::OnSizeSet( targetSize );

  if( mObjectView )
  {
    mObjectView.SetSize( targetSize );
  }
}

} // namespace Internal

} // namespace WidgetView

} // namespace Dali
