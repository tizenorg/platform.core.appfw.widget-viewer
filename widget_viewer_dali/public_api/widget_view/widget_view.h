#ifndef __DALI_WIDGET_VIEW_WIDGET_VIEW_H__
#define __DALI_WIDGET_VIEW_WIDGET_VIEW_H__

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

// EXTERNAL INCLUDES
#include <dali-toolkit/public-api/controls/control.h>

namespace Dali
{

namespace WidgetView
{

namespace Internal DALI_INTERNAL
{
class WidgetView;
}

/**
 * @addtogroup dali_widget_view
 * @{
 */

/**
 * @brief WidgetView is a class for displaying the widget image and controlling the widget.
 * Input events that WidgetView gets are delivered to the widget.
 *
 * @since_tizen 3.0
 */
class DALI_IMPORT_API WidgetView : public Toolkit::Control
{
public:

  /**
   * @brief Create widget view.
   *
   * @since_tizen 3.0
   * @param[in] widgetId The widget id.
   * @param[in] contentInfo Contents that will be given to the widget instance.
   * @param[in] width The widget width.
   * @param[in] height The widget height.
   * @param[in] period The period of updating contents of the widget.
   * @return A handle to WidgetView.
   */
  static WidgetView New( const std::string& widgetId, const std::string& contentInfo, int width, int height, double period );

  /**
   * @brief Downcast a handle to WidgetView handle.
   *
   * If the BaseHandle points is a WidgetView the downcast returns a valid handle.
   * If not the returned handle is left empty.
   *
   * @since_tizen 3.0
   * @param[in] handle Handle to an object
   * @return handle to a WidgetView or an empty handle
   */
  static WidgetView DownCast( BaseHandle handle );

  /**
   * @brief Creates an empty handle.
   * @since_tizen 3.0
   */
  WidgetView();

  /**
   * @brief Copy constructor.
   *
   * @since_tizen 3.0
   * @param[in] handle The handle to copy from.
   */
  WidgetView( const WidgetView& handle );

  /**
   * @brief Assignment operator.
   *
   * @since_tizen 3.0
   * @param[in] handle The handle to copy from.
   * @return A reference to this.
   */
  WidgetView& operator=( const WidgetView& handle );

  /**
   * @brief Destructor
   *
   * This is non-virtual since derived Handle types must not contain data or virtual methods.
   * @since_tizen 3.0
   */
  ~WidgetView();

  /**
   * @brief Get the id of the widget.
   *
   * @since_tizen 3.0
   * @return The widget id on success, otherwise an empty string.
   */
  const std::string& GetWidgetId() const;

  /**
   * @brief Get the instance id of the widget.
   *
   * @since_tizen 3.0
   * @return The instance id on success, otherwise an empty string.
   */
  const std::string& GetInstanceId() const;

  /**
   * @brief Get the content string of the widget.
   * This string can be used for creating contents of widget again after reboot a device or recovered from crash(abnormal status).
   *
   * @since_tizen 3.0
   * @return The content string to be recognize content of the widget or an empty string if there is no specific content string.
   */
  const std::string& GetContentInfo() const;

  /**
   * @brief Get the summarized string of the widget content for accessibility.
   * If the accessibility feature is turned on, a viewer can use this text to describe the widget.
   *
   * @since_tizen 3.0
   * @return The title string to be used for summarizing the widget or an empty string if there is no summarized text for content of given widget.
   */
  const std::string& GetTitle() const;

  /**
   * @brief Get the update period of the widget.
   *
   * @since_tizen 3.0
   * @return The update period of the widget.
   */
  double GetPeriod() const;

  /**
   * @brief Activate a widget in faulted state.
   * A widget in faulted state MUST be activated before adding the widget.
   *
   * @since_tizen 3.0
   * @privlevel public
   * @privilege %http://tizen.org/privilege/widget.viewer
   */
  void ActivateFaultedWidget();

  /**
   * @brief Check whether the widget is faulted.
   *
   * @since_tizen 3.0
   * @return true for faulted state, otherwise false.
   */
  bool IsWidgetFaulted();

  /**
   * @brief Set the deletion mode.
   *
   * @since_tizen 3.0
   * @param[in] permanentDelete Pass true if you want to delete this widget instance permanently, or pass false if you want to keep it and it will be re-created soon.
   */
  void SetPermanentDelete( bool permanentDelete );

public: //Signals

  typedef Signal< void ( WidgetView ) > WidgetViewSignalType;   ///< WidgetView signal type @since_tizen 3.0

  /**
   * @brief This signal is emitted when the widget is added.
   *
   * @since_tizen 3.0
   * @return The signal to connect to.
   */
  WidgetViewSignalType& WidgetAddedSignal();

  /**
   * @brief This signal is emitted when the widget is deleted.
   *
   * @since_tizen 3.0
   * @return The signal to connect to.
   */
  WidgetViewSignalType& WidgetDeletedSignal();

public: // Not intended for application developers

  /**
   * @brief Creates a handle using the WidgetView::Internal implementation.
   *
   * @since_tizen 3.0
   * @param[in] implementation The WidgetView implementation.
   */
  DALI_INTERNAL WidgetView( Internal::WidgetView& implementation );

  /**
   * @brief Allows the creation of this control from an Internal::CustomActor pointer.
   *
   * @since_tizen 3.0
   * @param[in] internal A pointer to the internal CustomActor.
   */
  DALI_INTERNAL WidgetView( Dali::Internal::CustomActor* internal );
};

/**
 * @}
 */
} // namespace WidgetView

} // namespace Dali

#endif // __DALI_WIDGET_VIEW_WIDGET_VIEW_H__
