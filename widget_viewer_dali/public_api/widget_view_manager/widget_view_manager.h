#ifndef __DALI_WIDGET_VIEW_WIDGET_VIEW_MANAGER_H__
#define __DALI_WIDGET_VIEW_WIDGET_VIEW_MANAGER_H__

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

// INTERNAL INCLUDES
#include <dali/public-api/object/base-handle.h>

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

class DALI_IMPORT_API WidgetViewManager : public BaseHandle
{
public:

  /**
   * @brief Creates an WidgetViewManager handle.
   *
   * Calling member functions with an uninitialised handle is not allowed.
   * @since_tizen 3.0
   */
  WidgetViewManager();

  /**
   * @brief Destructor
   *
   * This is non-virtual since derived Handle types must not contain data or virtual methods.
   * @since_tizen 3.0
   */
  ~WidgetViewManager();

  /**
   * @brief Get the singleton of WidgetViewManager object.
   *
   * @return A handle to the WidgetViewManager control.
   */
  static WidgetViewManager Get();

public: // Not intended for application developers

  /**
   * @brief Creates a handle using the WidgetView::Internal implementation.
   *
   * @since_tizen 3.0
   * @param[in] implementation The WidgetViewManager implementation.
   */
  explicit DALI_INTERNAL WidgetViewManager( Internal::WidgetViewManager *implementation );
};

/**
 * @}
 */

} // namespace WidgetView

} // namespace Dali

#endif // __DALI_WIDGET_VIEW_WIDGET_VIEW_MANAGER_H__
