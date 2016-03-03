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

// EXTERNAL INCLUDES

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

Dali::WidgetView::WidgetView WidgetView::New()
{
  // Create the implementation, temporarily owned on stack
  IntrusivePtr< WidgetView > internalWidgetView = new WidgetView();

  // Pass ownership to CustomActor
  Dali::WidgetView::WidgetView widgetView( *internalWidgetView );

  // Second-phase init of the implementation
  // This can only be done after the CustomActor connection has been made...
  internalWidgetView->Initialize();

  return widgetView;
}

WidgetView::WidgetView()
: Control( ControlBehaviour( REQUIRES_TOUCH_EVENTS ) )
{
}

WidgetView::~WidgetView()
{
}

pid_t WidgetView::GetPid() const
{
  if( mObjectView )
  {
    return mObjectView.GetPid();
  }

  return 0;
}

std::string WidgetView::GetTitle() const
{
  if( mObjectView )
  {
    return mObjectView.GetTitle();
  }

  return NULL;
}

std::string WidgetView::GetAppId() const
{
  if( mObjectView )
  {
    return mObjectView.GetAppId();
  }

  return NULL;
}

void WidgetView::SetObjectView( Pepper::ObjectView objectView )
{
  mObjectView = objectView;

  mObjectView.SetParentOrigin( ParentOrigin::CENTER );
  mObjectView.SetAnchorPoint( AnchorPoint::CENTER );

  Self().Add( mObjectView );
  Self().SetResizePolicy( ResizePolicy::FIT_TO_CHILDREN, Dimension::ALL_DIMENSIONS );
}

} // namespace Internal

} // namespace WidgetView

} // namespace Dali
