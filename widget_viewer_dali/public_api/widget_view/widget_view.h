#ifndef __DALI_WIDGET_VIEW_WIDGET_VIEW_H__
#define __DALI_WIDGET_VIEW_WIDGET_VIEW_H__

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
 * Signals
 * | %Signal Name               | Method                                  |
 * |----------------------------|-----------------------------------------|
 * | widgetAdded                | @ref WidgetAddedSignal()                |
 * | widgetDeleted              | @ref WidgetDeletedSignal()              |
 * | widgetCreationAborted      | @ref WidgetCreationAbortedSignal()      |
 * | widgetResized              | @ref WidgetResizedSignal()              |
 * | widgetContentUpdated       | @ref WidgetContentUpdatedSignal()       |
 * | widgetExtraInfoUpdated     | @ref WidgetExtraInfoUpdatedSignal()     |
 * | widgetUpdatePeriodChanged  | @ref WidgetUpdatePeriodChangedSignal()  |
 * | widgetFaulted              | @ref WidgetFaultedSignal()              |
 *
 * @since_tizen 3.0
 */
class DALI_IMPORT_API WidgetView : public Toolkit::Control
{
public:

  /**
   * @brief The start and end property ranges for this control.
   * @since_tizen 3.0
   */
  enum PropertyRange
  {
    PROPERTY_START_INDEX = Control::CONTROL_PROPERTY_END_INDEX + 1, ///< @since_tizen 3.0
    PROPERTY_END_INDEX =   PROPERTY_START_INDEX + 1000              ///< Reserve property indices @since_tizen 3.0
  };

  /**
   * @brief An enumeration of properties belonging to the WidgetView class.
   * @since_tizen 3.0
   */
  struct Property
  {
    enum
    {
      // Event side properties

      /**
       * @brief name "previewEnabled", type BOOLEAN
       * @details Whether the preview of the widget is enabled.
       * @since_tizen 3.0
       */
      PREVIEW_ENABLED = PROPERTY_START_INDEX,

      /**
       * @brief name "stateTextEnabled", type BOOLEAN
       * @details Whether the state message of the widget is enabled.
       * @since_tizen 3.0
       */
      STATE_TEXT_ENABLED,

      /**
       * @brief name "permanentDelete", type BOOLEAN
       * @details Set true if you want to delete this widget instance permanently, or false if you want to keep it and it will be re-created soon.
       * @since_tizen 3.0
       */
      PERMANENT_DELETE,
    };
  };

public:

  /**
   * @brief Create widget view.
   *
   * @since_tizen 3.0
   * @privlevel public
   * @privilege %http://tizen.org/privilege/widget.viewer
   * @param[in] widgetId The widget id.
   * @param[in] contentInfo Contents that will be given to the widget instance.
   * @param[in] width The widget width.
   * @param[in] height The widget height.
   * @param[in] updatePeriod The period of updating contents of the widget.
   * @return A handle to WidgetView.
   */
  static WidgetView New( const std::string& widgetId, const std::string& contentInfo, int width, int height, double updatePeriod );

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
   * @brief Pauses a given widget.
   *
   * @since_tizen 3.0
   * @privlevel public
   * @privilege %http://tizen.org/privilege/widget.viewer
   * @return true on success, false otherwise.
   */
  bool PauseWidget();

  /**
   * @brief Resume a given widget.
   *
   * @since_tizen 3.0
   * @privlevel public
   * @privilege %http://tizen.org/privilege/widget.viewer
   * @return true on success, false otherwise.
   */
  bool ResumeWidget();

  /**
   * @brief Get the id of the widget.
   *
   * @since_tizen 3.0
   * @privlevel public
   * @privilege %http://tizen.org/privilege/widget.viewer
   * @return The widget id on success, otherwise an empty string.
   */
  const std::string& GetWidgetId() const;

  /**
   * @brief Get the instance id of the widget.
   *
   * @since_tizen 3.0
   * @privlevel public
   * @privilege %http://tizen.org/privilege/widget.viewer
   * @return The instance id on success, otherwise an empty string.
   */
  const std::string& GetInstanceId() const;

  /**
   * @brief Get the content string of the widget.
   * This string can be used for creating contents of widget again after reboot a device or recovered from crash(abnormal status).
   *
   * @since_tizen 3.0
   * @privlevel public
   * @privilege %http://tizen.org/privilege/widget.viewer
   * @return The content string to be recognize content of the widget or an empty string if there is no specific content string.
   */
  const std::string& GetContentInfo();

  /**
   * @brief Get the summarized string of the widget content for accessibility.
   * If the accessibility feature is turned on, a viewer can use this text to describe the widget.
   *
   * @since_tizen 3.0
   * @privlevel public
   * @privilege %http://tizen.org/privilege/widget.viewer
   * @return The title string to be used for summarizing the widget or an empty string if there is no summarized text for content of given widget.
   */
  const std::string& GetTitle();

  /**
   * @brief Get the update period of the widget content.
   *
   * @since_tizen 3.0
   * @privlevel public
   * @privilege %http://tizen.org/privilege/widget.viewer
   * @return The update period of the widget content.
   */
  double GetUpdatePeriod() const;

  /**
   * @brief Shows the widget.
   *
   * @since_tizen 3.0
   * @privlevel public
   * @privilege %http://tizen.org/privilege/widget.viewer
   * @note Use this function instead of Dali::Actor::SetVisible() to restart updating widget content.
   */
  void Show();

  /**
   * @brief Hides the widget.
   *
   * @since_tizen 3.0
   * @privlevel public
   * @privilege %http://tizen.org/privilege/widget.viewer
   * @note Use this function instead of Dali::Actor::SetVisible() to stop updating widget content.
   */
  void Hide();

  /**
   * @brief Cancels touch event procedure.
   * If you call this function after feed the touch down event, the widget will get ON_HOLD events.
   * If a widget gets ON_HOLD event, it will not do anything even if you feed touch up event.
   *
   * @since_tizen 3.0
   * @privlevel public
   * @privilege %http://tizen.org/privilege/widget.viewer
   * @return true on success, false otherwise.
   */
  bool CancelTouchEvent();

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
   * @privlevel public
   * @privilege %http://tizen.org/privilege/widget.viewer
   * @return true for faulted state, otherwise false.
   */
  bool IsWidgetFaulted();

public: //Signals

  typedef Signal< void ( WidgetView ) > WidgetViewSignalType;   ///< WidgetView signal type @since_tizen 3.0

  /**
   * @brief This signal is emitted when the widget is added.
   *
   * @since_tizen 3.0
   * @privlevel public
   * @privilege %http://tizen.org/privilege/widget.viewer
   * @return The signal to connect to.
   */
  WidgetViewSignalType& WidgetAddedSignal();

  /**
   * @brief This signal is emitted when the widget is deleted.
   *
   * @since_tizen 3.0
   * @privlevel public
   * @privilege %http://tizen.org/privilege/widget.viewer
   * @return The signal to connect to.
   */
  WidgetViewSignalType& WidgetDeletedSignal();

  /**
   * @brief This signal is emitted when the widget creation is aborted.
   *
   * @since_tizen 3.0
   * @privlevel public
   * @privilege %http://tizen.org/privilege/widget.viewer
   * @return The signal to connect to.
   */
  WidgetViewSignalType& WidgetCreationAbortedSignal();

  /**
   * @brief This signal is emitted when the widget is resized.
   *
   * @since_tizen 3.0
   * @privlevel public
   * @privilege %http://tizen.org/privilege/widget.viewer
   * @return The signal to connect to.
   */
  WidgetViewSignalType& WidgetResizedSignal();

  /**
   * @brief This signal is emitted when the widget content is updated.
   *
   * @since_tizen 3.0
   * @privlevel public
   * @privilege %http://tizen.org/privilege/widget.viewer
   * @return The signal to connect to.
   */
  WidgetViewSignalType& WidgetContentUpdatedSignal();

  /**
   * @brief This signal is emitted when the widget extra info is updated.
   *
   * @since_tizen 3.0
   * @privlevel public
   * @privilege %http://tizen.org/privilege/widget.viewer
   * @return The signal to connect to.
   */
  WidgetViewSignalType& WidgetExtraInfoUpdatedSignal();

  /**
   * @brief This signal is emitted when the widget update period is changed.
   *
   * @since_tizen 3.0
   * @privlevel public
   * @privilege %http://tizen.org/privilege/widget.viewer
   * @return The signal to connect to.
   */
  WidgetViewSignalType& WidgetUpdatePeriodChangedSignal();

  /**
   * @brief This signal is emitted when the widget process is not running.
   *
   * @since_tizen 3.0
   * @privlevel public
   * @privilege %http://tizen.org/privilege/widget.viewer
   * @return The signal to connect to.
   */
  WidgetViewSignalType& WidgetFaultedSignal();

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
