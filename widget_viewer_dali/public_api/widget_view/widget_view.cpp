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
#include <public_api/widget_view/widget_view.h>

// INTERNAL INCLUDES
#include <internal/widget_view/widget_view_impl.h>

namespace Dali
{

namespace WidgetView
{

WidgetView WidgetView::New( const std::string& widgetId, const std::string& contentInfo, int width, int height, double period )
{
  return Internal::WidgetView::New( widgetId, contentInfo, width, height, period );
}

WidgetView WidgetView::DownCast( BaseHandle handle )
{
  return Toolkit::Control::DownCast<WidgetView, Internal::WidgetView>( handle );
}

WidgetView::WidgetView()
{
}

WidgetView::WidgetView( const WidgetView& handle )
: Toolkit::Control( handle )
{
}

WidgetView& WidgetView::operator=( const WidgetView& handle )
{
  if( &handle != this )
  {
    Control::operator=( handle );
  }
  return *this;
}

WidgetView::~WidgetView()
{
}

const std::string& WidgetView::GetWidgetId() const
{
  return Dali::WidgetView::GetImplementation( *this ).GetWidgetId();
}

const std::string& WidgetView::GetInstanceId() const
{
  return Dali::WidgetView::GetImplementation( *this ).GetInstanceId();
}

const std::string& WidgetView::GetContentInfo() const
{
  return Dali::WidgetView::GetImplementation( *this ).GetContentInfo();
}

const std::string& WidgetView::GetTitle() const
{
  return Dali::WidgetView::GetImplementation( *this ).GetTitle();
}

double WidgetView::GetPeriod() const
{
  return Dali::WidgetView::GetImplementation( *this ).GetPeriod();
}

void WidgetView::ActivateFaultedWidget()
{
  return Dali::WidgetView::GetImplementation( *this ).ActivateFaultedWidget();
}

bool WidgetView::IsWidgetFaulted()
{
  return Dali::WidgetView::GetImplementation( *this ).IsWidgetFaulted();
}

void WidgetView::SetPermanentDelete( bool permanentDelete )
{
  return Dali::WidgetView::GetImplementation( *this ).SetPermanentDelete( permanentDelete );
}

WidgetView::WidgetViewSignalType& WidgetView::WidgetAddedSignal()
{
  return Dali::WidgetView::GetImplementation(*this).WidgetAddedSignal();
}

WidgetView::WidgetViewSignalType& WidgetView::WidgetDeletedSignal()
{
  return Dali::WidgetView::GetImplementation(*this).WidgetDeletedSignal();
}

WidgetView::WidgetView( Internal::WidgetView& implementation )
: Control( implementation )
{
}

WidgetView::WidgetView( Dali::Internal::CustomActor* internal )
: Control( internal )
{
  VerifyCustomActorPointer<Internal::WidgetView>( internal );
}

} // namespace WidgetView

} // namespace Dali