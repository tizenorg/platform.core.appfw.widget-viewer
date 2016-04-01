#ifndef __DALI_WIDGET_VIEW_WIDGET_VIEW_MANAGER_H__
#define __DALI_WIDGET_VIEW_WIDGET_VIEW_MANAGER_H__

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

// INTERNAL INCLUDES
#include <public_api/widget_view/widget_view.h>

// EXTERNAL INCLUDES
#include <dali/public-api/object/base-handle.h>
#include <dali/public-api/adaptor-framework/application.h>

namespace Dali
{

namespace WidgetView
{

namespace Internal DALI_INTERNAL
{
class WidgetViewManager;
}

/**
 * @addtogroup dali_widget_view
 * @{
 */

/**
 * @brief WidgetViewManager manages addition of WidgetView controls.
 *
 * This class provides the functionality of adding the widget views and controlling the widgets.
 *
 * @since_tizen 3.0
 */
class DALI_IMPORT_API WidgetViewManager : public BaseHandle
{
public:

  /**
   * @brief Create widget view manager.
   *
   * @since_tizen 3.0
   * @param[in] application Application class for the widget view manager.
   * @param[in] name Widget view manager name. It is used for socket name internally.
   * @return A handle to WidgetViewManager.
   */
  static WidgetViewManager New( Application application, const std::string& name );

  /**
   * @brief Downcast a handle to WidgetViewManager handle.
   *
   * If the BaseHandle points is a WidgetViewManager the downcast returns a valid handle.
   * If not the returned handle is left empty.
   *
   * @since_tizen 3.0
   * @param[in] handle Handle to an object
   * @return handle to a WidgetViewManager or an empty handle
   */
  static WidgetViewManager DownCast( BaseHandle handle );

  /**
   * @brief Creates an WidgetViewManager handle.
   *
   * Calling member functions with an uninitialised handle is not allowed.
   * @since_tizen 3.0
   */
  WidgetViewManager();

  /**
   * @brief Copy constructor.
   *
   * @since_tizen 3.0
   * @param[in] handle The handle to copy from.
   */
  WidgetViewManager( const WidgetViewManager& handle );

  /**
   * @brief Assignment operator.
   *
   * @since_tizen 3.0
   * @param[in] handle The handle to copy from.
   * @return A reference to this.
   */
  WidgetViewManager& operator=( const WidgetViewManager& handle );

  /**
   * @brief Destructor
   *
   * This is non-virtual since derived Handle types must not contain data or virtual methods.
   * @since_tizen 3.0
   */
  ~WidgetViewManager();

  /**
   * @brief Creates a new widget view object
   *
   * @since_tizen 3.0
   * @param[in] widgetId The widget id.
   * @param[in] contentInfo Contents that will be given to the widget instance.
   * @param[in] width The widget width.
   * @param[in] height The widget height.
   * @param[in] period The period of updating contents of the widget.
   * @return A handle to WidgetView.
   */
  WidgetView AddWidget( const std::string& widgetId, const std::string& contentInfo, int width, int height, double period );

public: // Not intended for application developers

  /**
   * @brief Creates a handle using the WidgetView::Internal implementation.
   *
   * @since_tizen 3.0
   * @param[in] implementation The WidgetViewManager implementation.
   */
  explicit DALI_INTERNAL WidgetViewManager( Internal::WidgetViewManager* implementation );
};

/**
 * @}
 */

} // namespace WidgetView

} // namespace Dali

#endif // __DALI_WIDGET_VIEW_WIDGET_VIEW_MANAGER_H__
