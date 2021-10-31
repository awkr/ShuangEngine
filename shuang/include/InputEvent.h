#pragma once

enum class InputEventSource { NONE, KEYBOARD, MOUSE };

class InputEvent {
public:
  explicit InputEvent(const InputEventSource source) : mSource{source} {}

  [[nodiscard]] InputEventSource getSource() const { return mSource; }

private:
  InputEventSource mSource;
};

enum class KeyCode {
  UNKNOWN,
  ESCAPE,
  UP,
  DOWN,
  LEFT,
  RIGHT,
  A,
  D,
  E,
  F,
  H,
  I,
  J,
  K,
  L,
  O,
  Q,
  R,
  S,
  U,
  W,
  F5,
};
enum class KeyAction { UNKNOWN, DOWN, UP, REPEAT };

class KeyInputEvent : public InputEvent {
public:
  KeyInputEvent(KeyCode code, KeyAction action)
      : InputEvent{InputEventSource::KEYBOARD}, mCode{code}, mAction{action} {}

  [[nodiscard]] KeyCode   getCode() const { return mCode; }
  [[nodiscard]] KeyAction getAction() const { return mAction; }

private:
  KeyCode   mCode;
  KeyAction mAction;
};

enum class MouseButton { UNKNOWN, LEFT, RIGHT, MIDDLE, BACK, FORWARD };
enum class MouseAction { UNKNOWN, DOWN, UP, MOVE };

class MouseButtonInputEvent : public InputEvent {
public:
  MouseButtonInputEvent(MouseButton button, MouseAction action, float x, float y)
      : InputEvent{InputEventSource::MOUSE}, mButton{button}, mAction{action}, mX{x}, mY{y} {}

  [[nodiscard]] MouseButton getButton() const { return mButton; }
  [[nodiscard]] MouseAction getAction() const { return mAction; }
  [[nodiscard]] float       getX() const { return mX; }
  [[nodiscard]] float       getY() const { return mY; }

private:
  MouseButton mButton;
  MouseAction mAction;
  float       mX;
  float       mY;
};

const char *toString(KeyAction action);
